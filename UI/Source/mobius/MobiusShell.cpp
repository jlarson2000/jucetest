/**
 * The Mobius engine shell which interacts with the MobiusContainer
 * and manages the MobiusKernel.
 *
 * This is gradually being fleshed out, parts are still simulated.
 *
 * Thread Notes
 *
 * It is important to keep in mind that the code in this class can
 * be access from two differnt threads: UI Message thread and
 * the Maintenance thread.
 *
 * The UI Message thread is Juce's main message loop where component
 * listener callbacks, paint(), resized() and and a few other things
 * are called.  If you end up in any Component code after initial
 * construction you are in the message thread.  I've tried to keep
 * paint() constrained to just doing UI work, but the component listeners
 * such as ActionButton can end up down here.
 *
 * The other thread is a a maintenance thread that is supposed to
 * regularly call the performMaintenance method.  It is implemented
 * by MainThread.  This is where shell does most of the work, and
 * where the UI state for the next paint() gets refreshed.
 *
 * MainThread uses juce::MessageManagerLock to ensure that the UI
 * thread is blocked for the duration of the MainThread's run cycle.
 * This means we are free to do complex modifications to structures
 * that are shared by both threads, mostly MobiusConfig and DynamicConfig.
 *
 * Still it's a good idea to keep what is done in the UI thread
 * relavely simple, during normal use that is almost always just
 * doAction which handles a few shell=level actions and the queues
 * the action for the Kernel (the audio thread).
 *
 * There's actually probably a third thread involved here, what
 * you're in during the initial construction of the components,
 * Supervisor and this class.  This happens before MainThread is
 * started, and I think before the UI message loop is started so
 * we can be free to do the needful.
 *
 */

#include "../util/Trace.h"
#include "../util/Util.h"

#include "../model/MobiusConfig.h"
#include "../model/MobiusState.h"
#include "../model/FunctionDefinition.h"
#include "../model/UIParameter.h"
#include "../model/Setup.h"
#include "../model/Preset.h"
#include "../model/UIAction.h"
#include "../model/XmlRenderer.h"
#include "../model/UIEventType.h"
#include "../model/DynamicConfig.h"
#include "../model/ScriptConfig.h"
#include "../model/SampleConfig.h"

#include "core/Mobius.h"
#include "core/Scriptarian.h"
#include "core/Script.h"

#include "MobiusContainer.h"
#include "MobiusKernel.h"
#include "SampleManager.h"
#include "SampleReader.h"
#include "Simulator.h"
#include "AudioPool.h"
#include "Intrinsics.h"

#include "MobiusShell.h"

//////////////////////////////////////////////////////////////////////
//
// Initialization
//
//////////////////////////////////////////////////////////////////////

MobiusShell::MobiusShell(MobiusContainer* cont)
{
    container = cont;
    
    // this is given to us later
    configuration = nullptr;

    // see notes below on destructor subtleties
    // keep this on the stack rather than the heap
    // audioPool = new AudioPool();

    doSimulation = false;

    Intrinsic::init();
}

/**
 * Destruction subtelty.
 * 
 * Because MobiusKernel is a member object (or whatever those are called)
 * rather than a dynamically allocated pointer, it will be destructed AFTER
 * MobiusShell is destructed.  The problem is that the AudioPool is shared between
 * them.  Originally AudioPool was a stack object with a member pointer,
 * and we deleted it here in the destructor.  But when we did that it will be invalid
 * when Kernel wants to delete Mobius which will return things to the pool.
 * This didn't seem to crash in my testing, maybe because it still looks enough
 * like it did before it was returned to the heap, still surpised we didn't get
 * an access violation.
 *
 * So MobiusKernel needs to be destructed, or at least have all it's resources released
 * BEFORE we delete the AudioPool.
 *
 * If it were just a pointer to a heap object we could do that here and control the order
 * but if it's on the stack there are two options:
 * 
 *    - call a reset() method on the kernel to force it to delete everything early
 *      then when it's destructor eventually gets called there won't be anything left to do
 *    - put AudioPool on the stack too, paying attention to member order so it gets deleted
 *      last
 *
 * Seems to be one of the downsides to RAII, it makes destruction control less obvious
 * if there is a mixture of stack and heap objects and those things point to each other
 *
 * AudioPool is pretty simple so it can live fine on the stack, just pay careful attention
 * to lexical declaration order.
 *
 * From this thread:
 *  https://stackoverflow.com/questions/2254263/order-of-member-constructor-and-destructor-calls
 *  Yes to both. See 12.6.2
 *
 * "non-static data members shall be initialized in the order they were declared in the
 *  class definition"
 *
 * And then they are destroyed in reverse order.
 *
 * Note to self: the official term for that thing I've been calling "member objects"
 * is "data members".
 *
 * So for our purposes, MobiusKernel must be declared AFTER AudioPool in MobiusShell.
 * And AudioPool is now a data member.
 */
MobiusShell::~MobiusShell()
{
    delete configuration;

    audioPool.dump();
}

void MobiusShell::setListener(MobiusListener* l)
{
    listener = l;

    if (doSimulation)
      simulator.setListener(l);
}

//////////////////////////////////////////////////////////////////////
//
// Configuration
//
//////////////////////////////////////////////////////////////////////

/**
 * This always makes an internal copy of the passed object.
 * It is the responsibility of the caller to either free it
 * or keep using it.
 *
 * Two copies are made, one for the shell and one for the kernel.
 * The kernel copy must be phased in through the communicator.
 * Unless this is the first call during startup.
 *
 * !! not liking this guessing about whether this is the first
 * call or not.  startup processing needs to be defined better.
 * Kernel and Mobius now have initialize() vs. reconfigure()
 * consider that here too.
 */
void MobiusShell::configure(MobiusConfig* config)
{
    bool firstTime =  (configuration == nullptr);
    
    // clone it so we can make internal modifications
    // since we can be called after config editing, delete the existing one
    // give this class a proper clone() method so we don't have to use XML
    delete configuration;
    XmlRenderer xr;
    configuration = xr.clone(config);

    // clone it again and give it to the kernel
    MobiusConfig* kernelCopy = xr.clone(config);
    if (firstTime) {

        // start tracking internal runtime changes that the UI
        // may be interested in (e.g. script loading)
        initDynamicConfig();
        
        // initialization mess
        // to do what it does, the Kernel needs to start with
        //   shell - given at construction
        //   communicator - given at construction
        //   container - given here
        //   config - given here the first time, then passed with a message
        //   audioPool - immediately calls back to getAudioPool
        //
        // most if not all of this could done the same way, either
        // push it all down once in initialize() or have it pull
        // it one at a time inside initialize, can do some things
        // in the constructor, but not all like audioPool and config
        kernel.initialize(container, kernelCopy);

    }
    else {
        sendKernelConfigure(kernelCopy);
    }

    // temporarily simulation to track configuration changes
    // do this even if doSimulation is off so we can
    // get some things fleshed out so Supervisor can still call
    // simulateInterrupt
    simulator.initialize(configuration);
}

//////////////////////////////////////////////////////////////////////
//
// UI Thread Actions
//
// These are the main entry points for things in the UI thread
// to make something happen.
//
//////////////////////////////////////////////////////////////////////

/**
 * Actions may be perfomed at several levels: 
 *   shell, kernel, Core
 *
 * There are very few shell actions, most are sent to the
 * kernel, and then on to the core.
 */
void MobiusShell::doAction(UIAction* action)
{
    if (action->type == ActionFunction) {
        FunctionDefinition* f = action->implementation.function;
        if (f == nullptr) {
            Trace(1, "Unresolved function: %s\n", action->actionName);
        }
        else if (f == UnitTestMode) {
            // this one we can handle in the shell
            unitTests.actionSetup(action);
        }
        else {
            // the remainder are all destined for the core
            // send it to the simulator if configured
            if (doSimulation)
              simulator.doAction(action);
            else
              doKernelAction(action);
        }
    }
    else if (action->type == ActionIntrinsic) {
        // these are a new style of action that don't have
        // FunctionDefinition objects and are relatively
        // simple pass throughs to a Shell method.
        // Lambdas would be really nice here...
        // there are only two right now: LoadScripts and LoadSamples
        doIntrinsic(action);
    }
    else {
        // Parameter or Structure action
        if (doSimulation)
          simulator.doAction(action);
        else
          doKernelAction(action);
    }
}

/**
 * Pass the UIAction to the kernel through KernelCommunicator.
 *
 * Since the caller of this method retains ownership of the
 * UIAction we have to make a copy.  This is where "interning"
 * like we did in the old code might be nice, but issues with
 * differing arguments for actions with the same function and lifespan
 * while it is under Kernel control.
 *
 * When Kernel is done processing the action, it will send the object
 * we create here back through the communicator for reclamation.
 */

void MobiusShell::doKernelAction(UIAction* action)
{
    KernelMessage* msg = communicator.shellAlloc();
    if (msg != nullptr) {
        msg->type = MsgAction;

        // REALLY need a copy operator on these
        msg->object.action = new UIAction(action);
        communicator.shellSend(msg);
    }
}

/**
 * Retrieve the value of a UIParameter as an ordinal.
 * Interface is tempoary, replace with UIQuery someday.
 *
 * This I think can only be called from the Maintenance
 * thread right now when the ParametersElement updates
 * it's display state during the UI refresh cycle.
 */
int MobiusShell::getParameter(UIParameter* p, int trackNumber)
{
    if (doSimulation)
      return simulator.getParameter(p, trackNumber);

    // getParameter is expected to be shell safe so we don't
    // have to mess with KernelMessage, and the caller is expecting
    // this to be a synchronous call
    // todo: need to think about this, some things might
    // not be stable but as long as the readers don't have
    // any side effects it should be okay
    return kernel.getParameter(p, trackNumber);
}

/**
 * Install a set of pre-loaded samples.
 * Called from the "Install Samples" menu item.
 * Ownership of the SampleConfig is taken.
 *
 * The interface for this was done early and is weird.
 * The UI layer is expected to do all the sample file reading
 * and pass us a SampleConfig in a "loaded" state where each
 * Sample object will have a float[] containing the sample data.
 * The notion here was to get all file handling, and Juce in general,
 * above the shell.
 *
 * But when I got to scripts it was impossible not to have file
 * handling down here without exposing great gobs of internal code
 * to the UI.  Same for UnitTestMode.  That may still be a good goal,
 * but it makes the way sample files are read different than script files.
 * 
 * Later when the LoadSamples intrinsic function was added we had
 * to do sample file loading down here anyway so now there are two
 * ways to load samples.  Need to get rid of this interface method
 * and just use UActions to trigger sample loading.
 *
 * Also note that the SampleConfig here comes from Supervisor and
 * with the addition of UnitTestMode may not match the samples
 * that are currently loaded.  So the notion of who owns the
 * list of files in the SampleConfig is messy.
 *
 * The given SampleConfig is converted into a SampleManager
 * which will restructure the sample data using AudioPool.
 * SampleManager is then passed down to the kernel for use.
 */
void MobiusShell::installSamples(SampleConfig* samples)
{
    // consume the loaded samples and build the runtime
    // object used by the Kernel
    SampleManager* manager = new SampleManager(&audioPool, samples);

    // SampleManager took what it needed and left this behind
    // it didn't actually steal the float buffers
    delete samples;

    // update DynamicConfig and send them to the kernel
    sendSamples(manager, false);
}

//////////////////////////////////////////////////////////////////////
//
// Intrinsic Functions
//
// This is an emerging way to define new UIActions that are handled
// above the old core without having to blow out a FunctionDefinition
// static object every time we add one.
//
// Many functions definitions can reduce to just a name and an ordinal,
// but in retrospect we still need a place to hang those so right now
// it's a pair of constants defined in Intrinsics.h .  If it gets more
// complex than that we might as well just go back to some form of
// definition object.  I wanted to do that in XML, but the need for
// a compiled method to call can't be done there.  Lambdas would be
// convenient, but still need something concrete to dispatch on.
//
// Convert the action name to a function ordinal, and since there
// aren't many of these just use a switch to select the implementation
// rather than a jump table.
//
//////////////////////////////////////////////////////////////////////

/**
 * Locate the method that implements an intrinsic action
 * and call it.
 */
void MobiusShell::doIntrinsic(UIAction* action)
{
    // ignore up transitions, no intrinsics support them
    if (action->down) {
        
        IntrinsicId id = (IntrinsicId)action->implementation.ordinal;

        if (id < IntrinsicBase) {
            // if an ordinal is not specified, the action must have a name
            const char* actionName = action->actionName;
            if (actionName != nullptr) {
                id = Intrinsic::getId(actionName);
                // save it in the UIAction to avoid the name lookup next time
                // if the caller reuses this object
                if (id > IntrinsicBase)
                  action->implementation.ordinal = id;
            }
        }

        switch (id) {
            case IntrinsicLoadScripts:
                loadScripts(action);
                break;

            case IntrinsicLoadSamples:
                loadSamples(action);
                break;
            
            case IntrinsicAnalyzeDiff:
                unitTests.analyzeDiff(action);
                break;
        }
    }
}

/**
 * Load the sample data and send it to the Kernel for playback.
 * 
 * This is how I want sample loading to work, but we don't need it yet
 * since we started doing things with installSamples() above.
 * Migrate...
 */
void MobiusShell::loadSamples(UIAction* action)
{
    Trace(2, "Intrinsic loadSamples\n");

    // todo: should remember what we had last time
    // and check to see if there were any file name differences
    // before loading them all again

    SampleConfig* sconfig = configuration->getSampleConfig();
    SampleManager* manager = loadSamples(sconfig);
    sendSamples(manager, false);
}

/**
 * Load and analyze the scripts and send them to the Kernel for running.
 * 
 * Scripts were initially loaded by Mobius during the initialization
 * phasse when it was safe for it to allocate memory.
 * Now it isn't.
 *
 * Once incremental load is working, revisit the initialize() and
 * see if we can eliminate the duplication.
 *
 * This is a relatively heavy thing to be doing in the UI thread shell and requires
 * reaching deep into the core model to build a Scriptarian.  Because
 * compilation and linking to internal components like Function and Parameter
 * is tightly wound together, we can't just compile it to a ScriptLibrary
 * and pass it down, we have to make an entire scriptarian with a Mobius to
 * resolve references.
 *
 * This works but you have to be extremely careful when modifying Scriptarian
 * code, nothing in the construction process can have any side effects
 * on the runtime state of the Mobius object we give it for reference resolving.
 *
 * Similarly, while Mobius is happily running, it can't do anything to the
 * Scriptarian model we just built.
 *
 */
void MobiusShell::loadScripts(UIAction* action)
{
    Trace(2, "Intrinsic loadScripts\n");

    // compile what is currently in MobiusConfig
    ScriptConfig* sconfig = configuration->getScriptConfig();
    Scriptarian* scriptarian =  loadScripts(sconfig);
    sendScripts(scriptarian, false);
}

//////////////////////////////////////////////////////////////////////
//
// Dynamic Configuration
//
//////////////////////////////////////////////////////////////////////

/**
 * Initialize the DynamicConfig to contain the intrinsic function definitions.
 * This may be augmented later as scripts and samples are loaded.
 * Expected to be called only once during initialiazation.
 */
void MobiusShell::initDynamicConfig()
{
    // intrinsic function experiment
    Intrinsic::addIntrinsics(&dynamicConfig);
}

/**
 * Called by the UI in the Maintenance Thread to get information about
 * scripts, samples, and eventually other things that may be
 * of interest that were defined through complex runtime configuration
 * rather than the static configuration in MobiusConfig.
 *
 * This was added mostly so that the UI could know the names of
 * the callable scripts and show them in the action binding UI.
 *
 * Ownership of the object passes to the caller who must delete it.
 * Not entirely happy with this interface.  The caller needs to be
 * able to retain this information indefinitately so we either require
 * that it copy it and not take ownership, or we do the copy and make
 * it deal with it.  Also not sure whether this is the best way
 * to get this or if we should be sending it in the DynamicConfigChange
 * listener callback.
 * 
 * Now that we have this, I can see this being useful to return
 * the names of loaded samples, user variables defined in scripts
 * and maybe other things.
 * Evolving...
 *
 * We can assume that the object has been maintained incrementally as
 * things happen and all we have to do here is copy it and return it.
 */
DynamicConfig* MobiusShell::getDynamicConfig()
{
    // really need to brush up on copy constructors
    return new DynamicConfig(&dynamicConfig);
}

/**
 * Remove any current DynamicActions of the given type.
 * Making the simplifying assumption that when you load scripts
 * or samples, you're going to replace all of them, it isn't a merge.
 */
void MobiusShell::removeDynamicActions(ActionType* type)
{
    juce::OwnedArray<DynamicAction>* actions = dynamicConfig.getActions();
    
    int index = 0;
    while (index < actions->size()) {
        DynamicAction* a = (*actions)[index];
        if (a->type == type) {
            actions->remove(index);
        }
        else {
            index++;
        }
    }
}

/**
 * Update the DynamicConfig to contain things just loaded
 * from script files.
 */
void MobiusShell::installDynamicConfig(Scriptarian* scriptarian)
{
    // remove all the old ones and rebuild the list
    removeDynamicActions(ActionScript);
    
    if (scriptarian != nullptr) {
        ScriptLibrary* lib = scriptarian->getLibrary();
        if (lib != nullptr) {
            int ordinal = 0;
            Script* script = lib->getScripts();
            while (script != nullptr) {
                // Script names are obscure
                // When the compiler creates one it looks for a !name directive
                // and uses that, if not found it will try to derive one from the file name
                // Script::getName is only set if it has a !name, use getDisplayName
                // for the name that must be used to reference it
                const char* bindingName = script->getDisplayName();
                if (bindingName == nullptr) {
                    Trace(1, "MobiusShell: Unable to determine script name for dynamic action!\n");
                }
                else {
                    DynamicAction* action = new DynamicAction();
                    // todo: need the name vs displayname difference here?
                    // as long as the name that is displayed can also be
                    // in a UIAction it doesn't matter
                    action->name = bindingName;
                    action->type = ActionScript;

                    // todo: I want scripts to have ordinals for fast lookup
                    // this could be just the order they are encountered in the ScriptConfig
                    // but I'm not sure how embedded Procs work, should we have several
                    // callable script procs per file?
                    // keep it simple and have just the name for now
                    action->ordinal = ordinal;

                    // todo: one day have more flexible binding suggestions
                    action->button = script->isButton();
                    
                    dynamicConfig.addAction(action);
                }
                script = script->getNext();
                ordinal++;
            }
        }
    }
}

/**
 * Update the DynamicConfig to contain things just loaded
 * from sample files.
 */
void MobiusShell::installDynamicConfig(SampleManager* manager)
{
    // this isn't really an action type, but that's all we have to
    // distinguish these from ActionScripts if we want to keep them
    // on the same list, think more...
    removeDynamicActions(ActionSample);
    
    if (manager != nullptr) {
        int ordinal = 0;
        SamplePlayer* player = manager->getPlayers();
        while (player != nullptr) {
            const char* filename = player->getFilename();
            if (filename == nullptr) {
                Trace(1, "MobiusShell: Unable to determine sample name for dynamic action!\n");
            }
            else {
                DynamicAction* action = new DynamicAction();

                // extract just the leaf file name
                juce::File file(filename);
                juce::String leaf = file.getFileNameWithoutExtension();
                
                action->name = juce::String("Sample:") + leaf;
                action->type = ActionSample;

                // ordinal is just the position in the list
                action->ordinal = ordinal;

                // these aren't buttonable like scripts, but that
                // might be interesting, add a flag to Sample
                action->button = player->isButton();
                    
                dynamicConfig.addAction(action);
            }
            player = player->getNext();
            ordinal++;
        }
    }
}

//////////////////////////////////////////////////////////////////////
//
// Maintenance
//
//////////////////////////////////////////////////////////////////////

/**
 * Return the complex state object that serves as the primary
 * mechanism for communicationg the internal state of the Mobius engine
 * to the UI.  It is intended to be called periodically from the
 * Maintenance Thread, though it is safe to call from the UI thread.
 *
 * The object is owned by the shell and must not be deleted or modified.
 * It will live as long as the MobiusInterface/MobiusShell does so the
 * UI is allowed to retain a pointer to it.  Not sure I like this,
 * we could require that this be called every time and may return
 * different objects, but a handful of componenents now expect to retain
 * pointers into the middle of this.
 *
 * So while this will return the same object every time, this does serve
 * as the trigger to refresh the state.
 *
 * Needs redesign, but this is old and it's all over the core so it will
 * be sensitive.
 */
MobiusState* MobiusShell::getState()
{
    MobiusState* state = nullptr;

    if (doSimulation) {
        // we maintain an object that the simulator will put things into
        state = &simulatorState;
    }
    else {
        // ask the kernel for it, which asks the core
        // one of the rare cases where we bypass KernelCommunicator
        // safe since this is created during the initialization phase
        // and never changed
        // not happy with this...
        state = kernel.getState();
    }

    if (state == nullptr) {
        // kernel isn't happy, caller is expecting something
        // so return the simulation state, just so we don't crash during testing
        state = &simulatorState;
    }
    
    return state;
}

/**
 * Expected to be called at regular small intervals by a thread
 * managed in the UI, usually 1/10 second.
 * 
 * Since Juce is already leaking down here could consider using the
 * Timer directly and registring ourselves rather than having
 * MainThread do it.
 *
 * All the action happens as we consume KernelEvents which are
 * implemented over in KernelEventHandler.
 * 
 */
void MobiusShell::performMaintenance()
{
    // process KernelEvent and other things sent up
    consumeCommunications();
    
    // extend the message pool if necessary
    communicator.checkCapacity();

    // todo: all object pool fluffing should be done here now too
    // need to redesign the old pools to be consistent and allow
    // management from another thread
}

//////////////////////////////////////////////////////////////////////
//
// Kernel Communication
//
// Code in this section is related to the communication between the
// shell and the kernel.  It will not be accessible to the UI level code.
//
//////////////////////////////////////////////////////////////////////

/**
 * We share an AudioPool with the kernel, once this is called
 * the pool can not be deleted.  Kernel calls back to this,
 * would be cleaner if we just passed that to kernel.initialize()
 */
AudioPool* MobiusShell::getAudioPool()
{
    return &audioPool;
}

/**
 * Send the kernel its copy of the MobiusConfig
 * The object is already a copy
 */
void MobiusShell::sendKernelConfigure(MobiusConfig* config)
{
    KernelMessage* msg = communicator.shellAlloc();
    if (msg != nullptr) {
        msg->type = MsgConfigure;
        msg->object.configuration = config;
        communicator.shellSend(msg);
    }
    // else, pool exhaustion, already traced
}

/**
 * Consume any messages sent back from the kernel.
 * Most of these are objects we allocated and passed down,
 * and now they are being returned to us for reclamation.
 *
 * More complex requests are handled through a KernelEvent
 * which is passed over to KernelEventHandler.
 */
void MobiusShell::consumeCommunications()
{
    KernelMessage* msg = communicator.shellReceive();
    while (msg != nullptr) {
        bool abandon = true;
        switch (msg->type) {

            case MsgConfigure: {
                // kernel is done with the previous configuration
                delete msg->object.configuration;
            }
                break;
                
            case MsgSamples: {
                // kernel is giving us back the old SampleManager
                delete msg->object.samples;
            }
                break;
                
            case MsgScripts: {
                // kerel is giving back an old Scriptarian
                delete msg->object.scripts;
            }
                break;
                
            case MsgAction: {
                // kernel returns a processed action
                delete msg->object.action;
            }
                break;

            case MsgEvent: {
                kernelEventHandler.doEvent(msg->object.event);
                // this one is unusual in that we send it back so
                // the KernelEvent can be returned to the pool
                // also resume scripts that were waiting for the event to complete
                communicator.shellSend(msg);
                abandon = false;
                
            }
                break;
        }

        if (abandon) communicator.shellAbandon(msg);
        
        // get the next one
        msg = communicator.shellReceive();
    }
}

//////////////////////////////////////////////////////////////////////
//
// Sample Loading
//
// This is more complicated than scripts because we don't automaticaly
// load them, and there are several ways this can happen.
// 
// This can be used from two contexts: a UIAction sent down
// from the UI or an event sent up from the kernel.
// 
// There is actually a third way, the old isntallSamples method
// but that's going away.
// 
// When initiated by the UI, this is typically from a menu item
// or action button or after the sample config is edited.  Currently
// it can come only from a menu or button and it is assumed that
// the SampleConfig we currently contain is accurate.  We don't load
// samples immediately when configure() is called, though that may change.
// Because loading is deferred, the UI must send down an action to get
// them loaded.
// 
// Loading samples automatically after the SampleConfig is edited
// will be more complicated.  In that case the UI needs to pass down
// a new SampleConfig which is normally contained within the MobiusConfig
// passed to configure().  We're still deferring that and require
// a follow-on action.
// 
// The other way we can get here is from the UnitTestSetup script statement.
// That will schedule a KernelEvent that ends up in the UnitTests class
// which will build a special SampleConfig containing the samples needed
// by the unit tests.  It then calls over here to install them.  In that
// case we are in the Maintenance thread.  Here the SampleConfig is
// transient and will not actually be the one in MobiusConfig.
// 
// In both cases we convert the SampleConfig into a SampleManager,
// update the DynamicAction to contain information about the samples
// that can be used by the UI, and send it down to the kernel.
// 
// The implementation is odd with the SampleReader which was factored out
// for the UI.  That creates a "loaded" SampleConfig containing
// the float buffers of sample data.  This is then converted into
// a SampleManager which restructures the float buffers into
// a segmented Audio object.
//
//////////////////////////////////////////////////////////////////////

/**
 * Take a SampleConfig containing file paths, load the sample data
 * and build the SampManager ready to send down to the kernel.
 */
SampleManager* MobiusShell::loadSamples(SampleConfig* src)
{
    SampleManager* manager = nullptr;
    
    if (src != nullptr) {
        // create a new "loaded" SampleConfig from the source
        SampleReader reader;
        SampleConfig* loaded = reader.loadSamples(src);

        // turn the loaded samples into a SampleManager
        manager = new SampleManager(&audioPool, loaded);

        // SampleManager copied the loaded float buffers into
        // Audio objects, it didn't actually steal the float buffers
        delete loaded;
    }
    
    return manager;
}

/**
 * Send a SampleManager containing loaded samples down to the kernel.
 * Update the DynamicConfig to have information about the samples
 * to return to the UI.
 *
 * The kludgey "safeMode" flag is for UnitTests where this is
 * being initiated from a script and we want to skip KernelMessage
 * passing and slam the samples directly into the kernel.  This is so
 * that the samples are available immediately when the test script
 * continues.  Life means nothing if you can't live dangerously.
 */
void MobiusShell::sendSamples(SampleManager* manager, bool safeMode)
{
    if (manager != nullptr) {
        installDynamicConfig(manager);

        // this is where sendScripts would call the DynamicConfigChanged
        // notification, do that here too?  When called by the unit tests
        // we're going to be doing both samples and scripts, so would
        // be better to merge the two notifications
    
        if (safeMode) {
            // at this point we would normally send a MsgSamples
            // down through KernelCommunicator, but we're going to play
            // fast and loose and assume kernel was left in GlobalReset
            kernel.slamSampleManager(manager);
        }
        else {
            KernelMessage* msg = communicator.shellAlloc();
            if (msg != nullptr) {
                msg->type = MsgSamples;
                msg->object.samples = manager;
                communicator.shellSend(msg);
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////
//
// Script Loading
//
// Like samples, we convert the ScriptConfig containing path names
// into the runtime object, Scriptarian.  Update DynamicConfig
// to show what scripts can be loaded, and send them to the kernel.
//
// There are two contexts for this, the UI may send down a UIAction
// for the LoadSamples function, or the kernel may be
// sending an event up to cause new samples to be loaded and installed.
//
// The former happens in the UI thread, and the later in the Maintenance
// thread.
//
// UnitTests also uses this to force loading a special set of scripts
// just for the unit tests.
// 
//////////////////////////////////////////////////////////////////////

/**
 * Take a ScriptConfig containing script file paths, and build the
 * runtime Scriptarian object ready to send to the kernel.
 * 
 * We have to violate encapsulation and get a pointer to a Mobius
 * because the compilation process needs that to resolve references
 * to Function and Parameter objects.  Unfortunately necessary because
 * the compilation and link phases are not well seperated, and the code
 * is old and cranky.
 *
 * It is safe as long as these rules are followed:
 *
 *    - the ScriptConfig we're dealing with cannot be assumed to
 *      be the same as the one living down in the core
 *
 *    - the Scriptarian compile/link process must have NO
 *      side effects on the Mobius object it is given, it is only
 *      allowed to use it to look up static Function and Parameter
 *      definitions
 */
Scriptarian* MobiusShell::loadScripts(ScriptConfig* src)
{
    // dig deep and get the bad boy
    Mobius* mobius = kernel.getCore();
    Scriptarian *scriptarian = new Scriptarian(mobius);
    scriptarian->compile(src);

    return scriptarian;
}

/**
 * Send a previously constructed Scriptarian down to the core.
 * Like sendSamples, the safeMode flag is only true when were
 * called by UnitTests and we're in a known state where it is safe
 * to skip the KernelCommunicator.
 */
void MobiusShell::sendScripts(Scriptarian* scriptarian, bool safeMode)
{
    // before sending it down, capture the DynamicConfig
    installDynamicConfig(scriptarian);

    // technically we could wait until kernel gives us back
    // the old Scriptarian before notifying, but let's be optimistic
    // if we're in the UI thread right now, for consistency it could
    // be better to defer this to the Maintenance thread but we'd need
    // a flag to tell it that something needs to happen
    if (listener != nullptr)
      listener->MobiusDynamicConfigChanged();

    if (safeMode) {
        Mobius* core = kernel.getCore();
        core->slamScriptarian(scriptarian);
    }
    else {
        // send it down
        KernelMessage* msg = communicator.shellAlloc();
        if (msg != nullptr) {
            msg->type = MsgScripts;
            msg->object.scripts = scriptarian;
            communicator.shellSend(msg);
        }
    }
}

//////////////////////////////////////////////////////////////////////
//
// Simulator
//
// Temporary code to simulate something that resembles the Mobius
// engine for UI testing in the early days before Mobius was there.
//
//////////////////////////////////////////////////////////////////////

void MobiusShell::test()
{
    simulator.test();
}

void MobiusShell::simulateInterrupt(float* input, float* output, int frames)
{
    simulator.simulateInterrupt(input, output, frames);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


/**
 * The Mobius engine shell which interacts with the MobiusContainer
 * and manages the MobiusKernel.
 *
 * This is gradually being fleshed out, parts are still simulated.
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

#include "MobiusContainer.h"
#include "MobiusKernel.h"
#include "SampleTrack.h"
#include "Simulator.h"
#include "AudioPool.h"

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
}

/**
 * Destruction subtelty.
 * Because MobiusKernel is a member object (or whatever those are called)
 * rather than a dynamically allocated pointer, it will be destructed AFTER
 * MobiusShell is destructed.  The problem is that the AudioPool is shared between
 * them.  Originally AudioPool was a stack object with a member pointer,
 * and we deleted it here in the destructor.  But when we did that it will be invalid
 * when Kernel wants to delete the Recorder which returns things to the pool.
 * This didn't seem to crash in my testing, maybe because it still looks enough
 * like it did before it was returned to the heap, still surpised we didn't get
 * an access violation.
 *
 * So MobiusKernel needs to be destructed, or at least have all it's resources released
 * BEFORE we delete the AudioPool.
 *
 * If it were just a pointer to a heap object we could do that here and control the order
 * but if it's on the stack there are two options
 *    - call a reset() method on the kernel to force it to delete everything early
 *      then when it's destructor eventually gets called there won't be anything left to do
 *    - put AudioPool on the stack to, paying attention to member order so it gets deleted
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
    // we don't actually use this, give it to the simulator
    listener = l;
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
 * Kernel instantiation vs. initialization is messy, work on this
 */
void MobiusShell::configure(MobiusConfig* config)
{
    bool firstTime =  (configuration == nullptr);
    
    // clone it so we can make internal modifications
    // since we can be called after config editing delete the existing one
    // give this class a proper clone() method so we don't have to use XML
    delete configuration;
    XmlRenderer xr;
    configuration = xr.clone(config);

    // clone it again and give it to the kernel
    MobiusConfig* kernelCopy = xr.clone(config);
    if (firstTime) {
        // initialization mess
        // to do what it does, the Kernel needs to start with
        //   shell - given at construction
        //   communicator - given at construction
        //   container - given here
        //   config - given here the first time, then passed with message
        //   audioPool - immediately calls back to getAudioPool
        //
        // most if not all of this could done the same way, either
        // push it all down once in initialize() or have it pull
        // it one at a time inside initialize, can do some things
        // in the constructor, but not all like audioPool and config
        // 
        // 
        kernel.initialize(container, kernelCopy);
    }
    else {
        sendKernelConfigure(kernelCopy);
    }

    // temporarily simulation to track configuration changes
    simulator.initialize(configuration);
}

/**
 * Install loaded samples.
 * This builds a new SampleTrack and sends it to the kernel.
 * We could do difference detection here, but that should have been
 * doen earlier before we bothered loading it.
 *
 * We take ownership of the object.
 */
void MobiusShell::installSamples(SampleConfig* samples)
{
    SampleTrack* track = new SampleTrack(&audioPool, samples);

    // SampleTrack took what it needed and left this behind
    // it didn't actually steal the float buffers
    delete samples;

    KernelMessage* msg = communicator.alloc();
    msg->type = MsgSampleTrack;
    msg->object.sampleTrack = track;
    communicator.pushKernel(msg);
}

//////////////////////////////////////////////////////////////////////
//
// Maintenance
//
//////////////////////////////////////////////////////////////////////

/**
 * Expected to be called at regular small intervals by a thread
 * managed in the UI, usually 1/10 second.
 * 
 * Since Juce is already leaking down here could consider using the
 * Timer directly and registring ourselves rather than having
 * MainThread do it.
 */
void MobiusShell::performMaintenance()
{
    consumeCommunications();
}

//////////////////////////////////////////////////////////////////////
//
// State
//
//////////////////////////////////////////////////////////////////////

MobiusState* MobiusShell::getState()
{
    return &state;
}

//////////////////////////////////////////////////////////////////////
//
// Kernel communication
//
//////////////////////////////////////////////////////////////////////

/**
 * We share an AudioPool with the kernel, once this is called
 * the pool can not be deleted.  Kernel calls back to this,
 * would be cleaner if we just passed that to kernel.initialize()
 *
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
    KernelMessage* msg = communicator.alloc();
    if (msg != nullptr) {
        msg->type = MsgConfigure;
        msg->object.configuration = config;
        communicator.pushKernel(msg);
    }
    // else, pool exhaustion, already traced
}

/**
 * Consume any messages sent back from the kernel.
 */
void MobiusShell::consumeCommunications()
{
    KernelMessage* msg = communicator.popShell();
    while (msg != nullptr) {
        switch (msg->type) {

            case MsgConfigure: {
                // kernel is done with the previous configuration
                delete msg->object.configuration;
            }
                break;
            case MsgSampleTrack: {
                // kernel is giving us back the old SampleTrack
                delete msg->object.sampleTrack;
            }
            case MsgAction: {
                // kernel returns a processed action
                delete msg->object.action;
            }
                break;
        }
        
        communicator.free(msg);
        msg = communicator.popShell();
    }
}

//////////////////////////////////////////////////////////////////////
//
// Actions
//
//////////////////////////////////////////////////////////////////////

/**
 * Actions may be perfomed at several levels: 
 *   shell, kernel, RecorderTrack, Core
 *
 * If an action makes it to Core it will be converted
 * from a UIAction to the old Action.
 *
 * There are currently no shell actions so we pass
 * it immediately to the kernel.
 *
 * A tempoary large redirect is made into the Simulator
 * but this will be phased out soon.
 */
void MobiusShell::doAction(UIAction* action)
{
    
    if (action->type == ActionFunction) {
        FunctionDefinition* f = action->implementation.function;
        if (f == nullptr) {
            trace("Unresolved function: %s\n", action->actionName);
        }
        else if (f == SamplePlay) {
            // this one we can handle
            doKernelAction(action);
        }
        else {
            // send it to the simulator if configured
            //simulator.doAction(action);
            doKernelAction(action);
        }
    }
    else {
        //simulator.doAction(action);
        doKernelAction(action);
    }
}

int MobiusShell::getParameter(UIParameter* p, int trackNumber)
{
    return simulator.getParameter(p, trackNumber);
}

/**
 * Ah, the first UIAction problem.
 * UI retains control of the passed object,
 * once we pass into the kernel boundary access is lost so we
 * have to make a copy, this is where interning
 * could be nice like the old engine does.
 * The problem is that each action can have different arguments and
 * flags so we can't intern just based on the function ordinal.
 * We've also got storage allocation problems, if we pass it to the kernel
 * it needs to be passed back for reclamation which isn't so bad.   Actually
 * that works out okay, we can do interning or not.  But start clarifying
 * the status of objects passed to the kernel
 *     does not take ownership but assumes indefinite lifespan
 *        can't modify those
 *     takes ownership and will either return it or destroy it when it is destroyed
 */

void MobiusShell::doKernelAction(UIAction* action)
{
    KernelMessage* msg = communicator.alloc();
    msg->type = MsgAction;

    // REALLY need a copy operator on these
    msg->object.action = new UIAction(action);
    communicator.pushKernel(msg);
}

//////////////////////////////////////////////////////////////////////
//
// Simulator
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


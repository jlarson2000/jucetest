/**
 * An evolving object that wraps "kernel" state and functions.
 */

#include "../util/Trace.h"
#include "../model/MobiusConfig.h"
#include "../model/FunctionDefinition.h"
#include "../model/UIParameter.h"
#include "../model/UIAction.h"

#include "MobiusContainer.h"
#include "MobiusShell.h"

#include "Audio.h"
#include "SampleManager.h"

// drag this bitch in
#include "core/Mobius.h"
#include "core/Function.h"
#include "core/Action.h"
#include "core/Parameter.h"

#include "MobiusKernel.h"

//////////////////////////////////////////////////////////////////////
//
// Configuration
//
//////////////////////////////////////////////////////////////////////

/**
 * We're constructed with the shell and the communicator which
 * are esseential.
 *
 * Well it's not that simple, we must also have a MobiusContainer
 * and an AudioPool which given the current ordering of static initialization
 * should exist, but let's wait till initialize().
 *
 * Note that nothing we may statically initialize may depend on any of this
 * this is especially true of Mobius which reaches back up for conatiner
 * and pool.  It works now because we dynamically allocate that later.
 */
MobiusKernel::MobiusKernel(MobiusShell* argShell, KernelCommunicator* comm)
{
    shell = argShell;
    communicator = comm;
    // something we did for leak debugging
    Mobius::initStaticObjects();
    coreActions = nullptr;
}

/**
 * This can only be destructed by the shell after
 * ensuring it will no longer be responding to
 * events from the audio thread.
 */
MobiusKernel::~MobiusKernel()
{
    delete samples;
    
    // old interface wanted a shutdown method not in the destructor
    // revisit this
    if (mCore != nullptr) {
        mCore->shutdown();
        delete mCore;
    }
    Mobius::freeStaticObjects();

    // we do not own shell, communicator, or container
    delete configuration;

    // stop listening
    if (container != nullptr)
      container->setAudioListener(this);

    // in theory we could have a lingering action queue from the
    // audio thread, but how would that happen, you can't delete
    // the Kernel out from under an active audio stream with good results
    if (coreActions != nullptr) {
        Trace(1, "MobiusKernel: Destruction with a lingering coreAction list!\n");
    }

    // KernelEventPool will auto-delete
}

/**
 * Called by the shell ONLY during the initial startup sequence
 * when the audio stream won't be active and we will be in the UI thread
 * so we can avoid kernel message passing.
 *
 * Configuration is a copy we get to keep until it is replaced
 * by a later MsgConfigure
 *
 */
void MobiusKernel::initialize(MobiusContainer* cont, MobiusConfig* config)
{
    // stuff we need before building Mobius
    container = cont;
    audioPool = shell->getAudioPool();
    configuration = config;

    // register ourselves as the audio listener
    // unclear when things start pumping in, but do this last
    // after we've had a chance to make ourselves look pretty
    cont->setAudioListener(this);

    // build the Mobius core
    // still have the "probe" vs "real" instantiation problem
    // if core initialization is too expensive to do all the time
    // then need to defer this until the first audio interrupt
    mCore = new Mobius(this);
    mCore->initialize(configuration);
}

/**
 * Retuen a pointer to the live MobiusState managed by the core
 * up to the shell.
 */
MobiusState* MobiusKernel::getState()
{
    return (mCore != nullptr) ? mCore->getState(0) : nullptr;
}

/**
 * Consume any messages from the shell at the beginning of each
 * audio listener interrupt.
 * Each message handler is responsible for calling communicator->free
 * when it is done, but often it will reuse the same message to
 * send a reply.
 */
void MobiusKernel::consumeCommunications()
{
    // specific handler methods decide whether to abandon or return this message
    KernelMessage* msg = communicator->kernelReceive();
    
    while (msg != nullptr) {
        switch (msg->type) {

            case MsgConfigure: reconfigure(msg); break;
            case MsgSamples: installSamples(msg); break;
            case MsgAction: doAction(msg); break;
            case MsgEvent: doEvent(msg); break;
        }
        
        msg = communicator->kernelReceive();
    }
}

/**
 * Process a MsgConfigure message containing
 * a change to the MobiusConfig.  This is a copy
 * we get to retain.  Return the old one back to the
 * shell so it can be deleted.
 *
 * NOTE: It would be tempting here to reuse the incomming
 * message for the return, but the consume loop wants to free that
 * not really important and I don't think worth messing with different
 * styles of consumption.
 */
void MobiusKernel::reconfigure(KernelMessage* msg)
{
    MobiusConfig* old = configuration;

    // take the new one
    configuration = msg->object.configuration;
    
    // reuse the request message to respond with the
    // old one to be deleted
    msg->object.configuration = old;

    // send the old one back
    communicator->kernelSend(msg);

    // this would be the place where make changes for
    // the new configuration, nothing right now
    // this is NOT where track configuration comes in
    if (mCore != nullptr)
      mCore->reconfigure(configuration);
}

//////////////////////////////////////////////////////////////////////
//
// MobiusContainer::AudioListener aka "the interrupt"
//
//////////////////////////////////////////////////////////////////////

/**
 * Kernel installs itself as the one AudioListener in the MobiusContainer
 * to receive notifications of audio blocks.
 * What we used to call the "interrupt".
 *
 * Consume any pending shell messages, which may schedule UIActions on the core.
 * Then advance the sample player which may inject audio into the stream.
 * Finally let the core advance.
 *
 * I'm having paranoia about the order of the queued UIAction processing.
 * Before this was done in recorderMonitorEnter after some very sensntive
 * initialization in Synchronizer and Track.  With KernelCommunicator we
 * process those message first, before Mobius gets involved.  This may not mattern
 * but it is worrysome and I don't want to change it until we've reached a stable
 * point.  Even then, it's probably a good idea to let Mobius have some time to wake
 * up before we start slamming actions at it.  UIActions destined for the core
 * will therefore be put in another list and passed to Mobius at the same time
 * as it is notified about audio buffers so it can decide whan to do them.
 */
void MobiusKernel::containerAudioAvailable(MobiusContainer* cont)
{
    // make sure this is clear
    coreActions = nullptr;
    
    // this may receive an updated MobiusConfig and will
    // call Mobius::reconfigure, UIActions that aren't handled at
    // this level are placed in coreActions
    consumeCommunications();

    // todo: it was around this point that we used to ask the Recorder
    // to echo the input to the output for monitoring
    //    rec->setEcho(mConfig->isMonitorAudio());
    // Recorder is gone now, and the option was mostly useless due to
    // latency, but if you need to resurrect it will have to do the
    // equivalent here

    // let SampleManager do it's thing
    if (samples != nullptr)
      samples->containerAudioAvailable(cont);

    // TODO: We now have UIActions to send to core in poorly defined order
    // this usually does not matter but for for sweep controls like OutputLevel
    // it can.  From the UI perspective the knob went from 100 to 101 to 102 but
    // when we pull process the actions we could do them in reverse order leaving it at 100.
    // They aren't timestamped so we don't know for sure what order Shell received them.
    // If we're careful we could make assumptions about how the lists are managed,
    // but that's too subtle, timestamps would be better.  As it stands at the moment,
    // KernelCommunicator message queues are a LIFO.  With the introduction of the coreActions
    // list, the order will be reversed again which is what we want, but if the implementation
    // of either collection changes this could break.
    
    // tell core it has audio and some actions to do
    mCore->containerAudioAvailable(cont, coreActions);

    // we now need to return the queued core actions back to the
    // shell for deletion
    UIAction* next = nullptr;
    while (coreActions != nullptr) {
        next = coreActions->next;
        KernelMessage* msg = communicator->kernelAlloc();
        msg->type = MsgAction;
        msg->object.action = coreActions;
        communicator->kernelSend(msg);
        coreActions = next;
    }
}

//////////////////////////////////////////////////////////////////////
//
// Samples
//
//////////////////////////////////////////////////////////////////////

/**
 * We've just consumed the pending SampleManager from the shell.
 *
 * TODO: If samples are currently playing need to stop them gracefully
 * or we'll get clicks.  Not important right now.
 * 
 */
void MobiusKernel::installSamples(KernelMessage* msg)
{
    SampleManager* old = samples;
    samples = msg->object.samples;

    if (old == nullptr) {
        // nothing to return
        communicator->kernelAbandon(msg);
    }
    else {
        // return the old one
        msg->object.samples = old;
        communicator->kernelSend(msg);
    }
}

//////////////////////////////////////////////////////////////////////
//
// Actions
//
//////////////////////////////////////////////////////////////////////

/**
 * Perform a UIAction sent by the shell.
 * First we check for actions that can be perormed by the
 * new kernel-level code.  If not, then we pass it down to the old core.
 *
 * There isn't much we do up here, currently just the SampleManager
 * See comments in containerAudioAvailable for subtlety around queueing core actions.
 */
void MobiusKernel::doAction(KernelMessage* msg)
{
    UIAction* action = msg->object.action;

    // todo: more flexitility in targeting tracks
    // upper tracks vs. core tracks etc.

    bool processed = false;
    if (action->type == ActionFunction) {
        FunctionDefinition* f = action->implementation.function;
        if (f == SamplePlay) {
            if (samples != nullptr) {
                // start one of the samples playing
                // when we had replicated Sample1, Sample2 functions the
                // sample index was embedded in the Function object, now
                // we require it in the action argument
                // UIAction::init was supposed to parse bindingArgs into
                // a number left in the ExValue arg
                int number = action->arg.getInt();
                // don't remember what I used to do but it is more obvious
                // for users to enter 1 based sample numbers
                // SampleTrack wants zero based
                // if they didn't set an arg, then just play the first one
                int index = (number > 0) ? number - 1 : 0;
                // mSampleTrack->trigger(mInterruptStream, index, action->down);
                samples->trigger(index, action->down);
                processed = true;
            }
        }
        else {
            // not handled above core, pass it down
            action->next = coreActions;
            coreActions = action;
        }
    }
    else {
        // Parameter, Activation, Script action
        action->next = coreActions;
        coreActions = action;
    }
    
    // if we handled it immediately, return it to the shell for deletion
    if (processed)
      communicator->kernelSend(msg);
    else
      communicator->kernelAbandon(msg);
}

//////////////////////////////////////////////////////////////////////
//
// Parameters
//
//////////////////////////////////////////////////////////////////////

/**
 * The interface for this is evolving to use UIQuery but for now
 * the getParameter method will be called by the UI on MobiusShell
 * and shell will pass it directly to us without going through KernelMessages.
 *
 * It is expected to be UI thread safe and synchronous.
 *
 * This isn't used very often, only for the "control" parameters like output
 * level and feedback.  And for the "instant parameter" UI component that allows
 * ad-hoc parameter changes without activating an entire Preset.
 *
 * trackNumber follows the convention of UIAction with a value of zero
 * meaning the active track, and specific track numbers starting from 1.
 *
 * The values returned are expected to be "ordinals" in the new model.
 * 
 */
int MobiusKernel::getParameter(UIParameter* p, int trackNumber)
{
    int value = 0;

    if (mCore != nullptr)
      value = mCore->getParameter(p, trackNumber);

    return value;
}

//////////////////////////////////////////////////////////////////////
//
// Events
//
//////////////////////////////////////////////////////////////////////

/**
 * Pass a kernel event to the shell.
 *
 * We're using KernelMessage as the mechanism to pass this up.
 * Need to think about whether frequent non-response events,
 * in particular TimeBoundary should just BE KernelMessages rather than
 * adding the extra layer of KernelEvent.
 */
void MobiusKernel::sendEvent(KernelEvent* e)
{
    KernelMessage* msg = communicator->kernelAlloc();
    msg->type = MsgEvent;
    msg->object.event = e;
    communicator->kernelSend(msg);
}

/**
 * Handle a MsgEvent sent back down from the shell.
 * For most of these, the ScriptInterpreters need to be informed
 * so they can cancel their wait states.
 */
void MobiusKernel::doEvent(KernelMessage* msg)
{
    KernelEvent* e = msg->object.event;

    if (e != nullptr) {
        
        if (mCore != nullptr) 
          mCore->kernelEventCompleted(e);

        // return to our pool
        eventPool.returnEvent(e);
    }
    
    // nothing to send back
    communicator->kernelAbandon(msg);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


    



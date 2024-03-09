/**
 * An evolving object that wraps "kernel" state and functions.
 */

#include "../util/Trace.h"
#include "../model/MobiusConfig.h"
#include "../model/FunctionDefinition.h"
#include "../model/UIAction.h"

#include "MobiusContainer.h"
#include "MobiusShell.h"

#include "Recorder.h"
#include "Audio.h"
#include "SampleTrack.h"

// drag this bitch in
#include "core/Mobius.h"

#include "MobiusKernel.h"

//////////////////////////////////////////////////////////////////////
//
// Configuration
//
//////////////////////////////////////////////////////////////////////

/**
 * We're constructed with the shell and the communicator which
 * are esseential
 */
MobiusKernel::MobiusKernel(MobiusShell* argShell, KernelCommunicator* comm)
{
    shell = argShell;
    communicator = comm;
    Mobius::initStaticObjects();
}

/**
 * This can only be destructed by the shell after
 * ensuring it will no longer be responding to
 * events from the audio thread.
 */
MobiusKernel::~MobiusKernel()
{
    // old interface wanted a shutdown method not in the destructor
    // revisit this
    if (mCore != nullptr) {
        mCore->shutdown();
        delete mCore;
    }
    Mobius::freeStaticObjects();

    // we do not own shell, communicator, or container
    delete configuration;
    
    delete mRecorder;

    // sampleTrack will be deleted by mRecorder destructor

    // stop listening
    if (container != nullptr)
      container->setAudioListener(this);
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
    container = cont;
    configuration = config;

    // set up the initial Recorder configuration
    initRecorder();

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
 * Consume any messages from the shell at the beginning of each
 * audio listener interrupt.
 * Each message handler is responsible for calling communicator->free
 * when it is done, but often it will reuse the same message to
 * send a reply.
 */
void MobiusKernel::consumeCommunications()
{
    KernelMessage* msg = communicator->popKernel();
    while (msg != nullptr) {
        switch (msg->type) {

            case MsgConfigure: reconfigure(msg); break;
            case MsgSampleTrack: installSampleTrack(msg); break;
            case MsgAction: doAction(msg); break;
        }
        
        msg = communicator->popKernel();
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
    communicator->pushShell(msg);

    // this would be the place where make changes for
    // the new configuration, nothing right now
    // this is NOT where track configuration comes in
    if (mCore != nullptr)
      mCore->reconfigure(configuration);
}

//////////////////////////////////////////////////////////////////////
//
// Recorder
// Start slowly dragging stuff over
//
//////////////////////////////////////////////////////////////////////

/**
 * Build out the inital Recorder structure based on the MobiusConfig.
 * We're in the initialization sequence in the UI thread so this
 * is one of the few places we're allowed to allocate memory.
 *
 * hmm, it would be clearer to enforce use of the communicator
 * even though we don't need to.  
 * 
 * This is approxomatley what the old Mobius::start did
 * We could be statically allocating Recorder now.
 */
void MobiusKernel::initRecorder()
{
    if (mRecorder == nullptr) {

        // formerly setup a listener for Midi events and started a timer

		mRecorder = new Recorder();

        // broke construction and initialization out in case we want
        // to static construct
        // note that we get the shared AudioPool from the shell
        // Recorder needs information about the audio stream from the container
        // but it is NOT the AudioListener, we do that and call Recorder
        // think more about that, Recorder could just use the container
        // passed in the interrupt rather than saving one
        mRecorder->initialize(container, shell->getAudioPool());

        // setup Synchronizer
        // setup MobiusThread

        // put the sample track first so it may put things into the
		// input buffer for the loop tracks
        
		//mSampleTrack = new SampleTrack(this);
		//mRecorder->add(mSampleTrack);

		// this will trigger track initialization, open devices,
		// load scripts, etc.
		//installConfiguration(mConfig, true);
        //installSamples();

        // "shfit" a copy of the MobiusConfig into the interrupt handler
        // install scripts

        // update focus lock/mute cancel limits
        // updateGlobalFunctionPreferences

        // set TraceDebugLevel and TracePrintLevel from MobiusConfig

        // Audio::setWriteFormatPCM(config->isIntegerWaveFile());

        // open MIDI devices depending on whether we're in plugin mode
        // ask Recorder to open audio devices
        
		// start the recorder (opens streams) and begins interrupt
        // does nothing now
        mRecorder->start();
    }
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
 * There are three phases:
 *
 *     start of interrupt housekeeping like phasing in communications from the shell,
 *      and scheduling events
 *
 *     tell the Recorder to process the new buffers and advance the state
 *       of the tracks
 *
 *     end of interrupt housekeeping like passing information back to the shell
 *     and updating MobiusState
 *
 */
void MobiusKernel::containerAudioAvailable(MobiusContainer* cont)
{
    interruptStart();

    // this isn't a listener, but we make the interface use the same signature
    //mRecorder->containerAudioAvailable(cont);

    interruptEnd();
}

//////////////////////////////////////////////////////////////////////
//
// Interrupt Start
//
//////////////////////////////////////////////////////////////////////

/**
 * Do various tasks at the start of each audio interface.
 * This is what old mobius called recorderMonitorEnter
 */
void MobiusKernel::interruptStart()
{
    consumeCommunications();
    if (mCore != nullptr) mCore->beginAudioInterrupt();
}

//////////////////////////////////////////////////////////////////////
//
// Interrupt End
//
//////////////////////////////////////////////////////////////////////

void MobiusKernel::interruptEnd()
{
    if (mCore != nullptr) mCore->endAudioInterrupt();
}

//////////////////////////////////////////////////////////////////////
//
// Samples
//
//////////////////////////////////////////////////////////////////////

/**
 * We've just consumed the pending SampleTrack from the shell.
 * Spray it into the Recorder
 */
void MobiusKernel::installSampleTrack(KernelMessage* msg)
{
    SampleTrack* old = sampleTrack;
    sampleTrack = msg->object.sampleTrack;

    // have to replace it within the Recorder too
    if (old == nullptr) {
        // first time, just add it
        mRecorder->add(sampleTrack);
        // nothing to return
        communicator->free(msg);
    }
    else {
        bool replaced = mRecorder->replace(old, sampleTrack);
        if (replaced) {
            // return the old one
            msg->object.sampleTrack = old;
            communicator->pushShell(msg);
        }
        else {
            // wasn't isntalled, should not happen
            // we can add it, but since sampleTrack was set we've
            // been using something that the recorder didn't have
            // which is a problem
            Trace(1, "MobiusKernel: SampleTrack replacement failed!\n");
            mRecorder->add(sampleTrack);
            // return the old as usual, but could also not add it
            // and return the new one too since we're in a weird state
            msg->object.sampleTrack = old;
            communicator->pushShell(msg);
        }
    }
}

//////////////////////////////////////////////////////////////////////
//
// Actions
//
//////////////////////////////////////////////////////////////////////

void MobiusKernel::doAction(KernelMessage* msg)
{
    UIAction* action = msg->object.action;

    // todo: more flexitility in targeting tracks
    // upper tracks vs. core tracks etc.

    if (action->type == ActionFunction) {
        FunctionDefinition* f = action->implementation.function;
        if (f == SamplePlay) {
            if (sampleTrack != nullptr) {
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
                sampleTrack->trigger(index, action->down);
            }
        }
        else {
            // not handled above core, pass it down
            if (mCore != nullptr)
              mCore->doAction(action);

            // todo: fish any results out and do something with them
        }
    }
    
    // if not handled above this is where we send it
    // to the CoreConverter

    // return it to the shell for deletion
    communicator->pushShell(msg);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


    



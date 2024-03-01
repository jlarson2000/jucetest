/**
 * An evolving object that wraps "kernel" state and functions.
 */

#include "../model/MobiusConfig.h"

#include "MobiusContainer.h"
#include "MobiusShell.h"

#include "Recorder.h"
#include "Audio.h"

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
}

/**
 * This can only be destructed by the shell after
 * ensuring it will no longer be responding to
 * events from the audio thread.
 */
MobiusKernel::~MobiusKernel()
{
    // we do not own shell, communicator, or container
    delete configuration;
    
    delete mRecorder;
    //delete mSampleTrack;
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
}

/**
 * Consume any messages from the shell at the beginning of each
 * audio listener interrupt.
 */
void MobiusKernel::consumeCommunications()
{
    KernelMessage* msg = communicator.popKernel();
    while (msg != nullptr) {
        switch (msg->type) {

            case MsgConfigure: reconfigure(msg); break;
            case MsgSampleTrack: isntallSampleTrack(msg); break;
        }
        
        communicator.free(msg);
        msg = communicator.popKernel();
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
void MobiusKernel::reconfigure(KernelMessage* request)
{
  
    
    // build a message to return the old one
    KernelMessage* response = communicator.alloc();
    response->type = MsgConfigure;
    response->object.configuration = configuration;

    // take the new one
    configuration = request->object.configuration;

    // send the old one back
    communicator.pushShell(response);

    // this would be the place where make changes for
    // the new configuration, nothing right now
    // this is NOT where track configuration comes in
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
    recorder->containerAudioAvailable(cont);

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

}

//////////////////////////////////////////////////////////////////////
//
// Interrupt End
//
//////////////////////////////////////////////////////////////////////

void MobiusKernel::interruptEnd()
{
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
void MobiusKernel::installSamples(KernelMesage* msg)
{
    SampleTrack* neu = msg.object.sampleTrack;
    SampleTrack* old = mRecorder->replaceSampleTrack(neu);

    if (old != nullptr) {
        KernelMessage* reply = communicator.alloc();
        reply.type = MsgSampleTrack;
        reply.object.sampleTrack = old;
        communicator.pushShell(reply);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


    



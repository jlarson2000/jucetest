/**
 * An evolving object that wraps "kernel" state and functions.
 */

#include "../model/MobiusConfig.h"

// what is now the "shell"
#include "MobiusShell.h"

#include "Recorder.h"
#include "Audio.h"

#include "MobiusContainer.h"
#include "MobiusKernel.h"

//////////////////////////////////////////////////////////////////////
//
// Configuration
//
//////////////////////////////////////////////////////////////////////

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
 * Link the objects together and do any pre-active configuration
 * necessary.  We keep ownership of the configuration.
 */
void MobiusKernel::initialize(MobiusContainer* cont, MobiusConfig* config)
{
    container = cont;
    configuration = config;

    // set up the initial Recorder configuration
    // this is what will listen for audio events
    initRecorder();
}

/**
 * Intended to be called on the audio thread at the start of each buffer.
 * Look for things left in the commicator and consume them.
 */
void MobiusKernel::consumeCommunications()
{
    // look for major config changes
    MobiusConfig* newconfig = communicator->receiveConfiguration();
    if (newconfig != nullptr) {
        communicator->returnConfiguration(configuration);
        configuration = newconfig;
        reconfigure();
    }

    // this doesn't use the comm buffer, it's easier
    SamplePack* pack = shell->consumePendingSamplePack();
    if (pack != nullptr)
      installSamples(pack);
}

/**
 * Reconfigure outselves with the initial pre-audio thread configuration
 * or later with one sent through the Communicator.
 * We should behave as if we were are always in the audio thread.
 */
void MobiusKernel::reconfigure()
{
}

//////////////////////////////////////////////////////////////////////
//
// Recorder
// Start slowly dragging stuff over
//
//////////////////////////////////////////////////////////////////////

/**
 * Setup the Recorder and initialize various things.
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
        mRecorder->initialize(container, shell->getAudioPool());

        // need to implement this or punt
		//mRecorder->setMonitor(this);

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

//////////////////////////////////////////////////////////////////////
//
// Interrupt End
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
// Samples
//
//////////////////////////////////////////////////////////////////////

/**
 * We've just consumed the pending SamplePack from the shell.
 * Spray it into the SampleTrack
 */
void MobiusKernel::installSamples(SamplePack* pack)
{
    if (pack != nullptr) {
        // !!!!!!!!! as expected this deletes the current SamplePlayers
        // and isntalls new ones from the pack, then deletes the pack
        // need a defecation method where when the kernel consumes new objects
        // it carefully replaces them, the returns the old ones back to the shell
        // where they can be deleted
        // mSampleTrack->setSamples(pack);
    }

}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


    



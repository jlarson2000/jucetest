/**
 * An evolving object that wraps "kernel" state and functions.
 */

#include "../model/MobiusConfig.h"

// what is now the "shell"
#include "MobiusSimulator.h"

#include "Recorder.h"
#include "Audio.h"

#include "MobiusContainer.h"
#include "MobiusKernel.h"

//////////////////////////////////////////////////////////////////////
//
// Configuration
//
//////////////////////////////////////////////////////////////////////

MobiusKernel::MobiusKernel(MobiusSimulator* argShell, KernelCommunicator* comm)
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
    delete mAudioPool;
    //delete mSampleTrack;
}

/**
 * Link the objects together and do any pre-active configuration
 * necessary.  We keep ownership of the configuration.
 */
void MobiusKernel::initialize(MobiusContainer* cong, MobiusConfig* config)
{
    container = cont;
    configuration = config;

    // todo: register ourselves as the AudioListener
    // or maybe we don't care and register the Recorder instead which
    // is how it used to work
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
// MobiusContainer::AudioListener
//
//////////////////////////////////////////////////////////////////////

void MobiusKernel::containerAudioAvailable(MobiusContainer* cont)
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
 * This is approxomatley what Mobius::start did
 */
void MobiusKernel::initRecorder()
{
    if (mRecorder == nullptr) {

        initObjectPools();

        // formerly setup a listener for Midi events and started a timer

        // formerly passed MidiInterface to get a timer
		mRecorder = new Recorder(audioInterface, mAudioPool);

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

void MobiusKernel::initObjectPools()
{
}

/**
 * This logic was in Mobius::installConfiguration
 * Meant to be called after any major MobiusConfig change
 */
#if 0
void MobiusKernel::installSamples()
{
	// load the samples
    // note that installation has to be deferred to the interrupt handler
    SamplePack* newSamples = nullptr;
	Samples* samples = configuration->getSamples();
    if (samples != NULL) {
        // only reload if there was a difference in order or files 
        // we could be smarter and only reread things that are new
        // but this isn't a commonly used features
        if (mSampleTrack->isDifference(samples))
          newSamples = new SamplePack(mAudioPool, getHomeDirectory(), samples);
    }
    else {
        // in order to remove current samples we need a non-null
        // SamplePack object to pass to the interrupt
        if (mSampleTrack->getSampleCount() > 0)
          newSamples = new SamplePack();
    }

    if (newSamples != NULL) {
        // this is bad, it would be safer just to ignore the shift
        // but then we couldn't edit samples before we add audio devices
        // !! ignore if we're receiving interrupts but allow otherwise
        // this can happen if you're messing with configs and don't have
        // an audio device selected
        Trace(2, "Mobius: phasing in sample changes\n");
        if (mPendingSamples != NULL) {
            if (mInterrupts > 0)
              Trace(1, "Mobius: Overflow installing samples\n");
            else
              delete mPendingSamples;
        }
        mPendingSamples = newSamples;
    }


}
#endif

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


    



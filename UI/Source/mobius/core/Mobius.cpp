/**
 * Heavily reduced version of the original primary class.
 *
 * There is a mixture of UI and audio thread code in here so be careful.
 * 
 * Anything from initialize() on down is called by the UI thread during
 * the initial build of the runtime model before we are receiving audio blocks.
 * This code is allowed to allocate memory and is not especially time constrained.
 *
 * All other code should be assumed to be in the audio thread and is constrained
 * by time and system resources.  Some code is shared between initialize() and
 * reconfigure(), notably propagateConfiguration() that takes a new or modified
 * MobiusConfig and gives it to the internal components that want to cache
 * things from it or do limited adjustments to their runtime structures.
 *
 * So while it may see like "propagate" code is part of initialization, it is both
 * and it must not do anything beyond simple config parameter copying.  This is
 * different than it was before where we allowed recompiling the script environment
 * and reallocation of the Tracks array in the audio thread.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "../../util/Util.h"
#include "../../util/List.h"

#include "../../model/Setup.h"
#include "../../model/UserVariable.h"
// ActionOperator moved up here
// factor this out into the ActionConstants file?
#include "../../model/UIAction.h"

// implemented by MobiusContainer now but still need the old MidiEvent model
#include "MidiByte.h"
#include "MidiEvent.h"
#include "MidiInterface.h"

#include "Mapper.h"
#include "Action.h"
#include "Actionator.h"
#include "Event.h"
#include "Export.h"
#include "Function.h"
#include "Layer.h"
#include "Loop.h"
#include "Mode.h"
#include "Parameter.h"
//#include "Project.h"
#include "Scriptarian.h"
#include "ScriptCompiler.h"
#include "Script.h"
#include "ScriptRuntime.h"
#include "Synchronizer.h"
#include "Track.h"

// for ScriptInternalVariable, encapsulation sucks
#include "Variable.h"

#include "Mobius.h"

//////////////////////////////////////////////////////////////////////
//
// New Kernel Interface
//
//////////////////////////////////////////////////////////////////////

/**
 * Build out only the state that can be done reliably in a static initializer.
 * No devices are ready yet.
 */
Mobius::Mobius(MobiusKernel* kernel)
{
    Trace(2, "Mobius::Mobius");

    mKernel = kernel;

    // this should be set at this point, may want to defer this
    // or force it to be passed in rather than doing a reach-around
    mContainer = kernel->getContainer();
    mAudioPool = kernel->getAudioPool();

    // Kernel may not have a MobiusConfig yet so have to wait
    // to do anything until initialize() is called
    mConfig = nullptr;
    mSetup = nullptr;

    // temporary adapters for old interfaces
    mMidi = new StubMidiInterface();
    
    mActionator = new Actionator(this);
    mScriptarian = new Scriptarian(this);
    mPendingScriptarian = nullptr;
    
    //
    // Legacy Initialization
    //

    mLayerPool = new LayerPool(mAudioPool);
    mEventPool = new EventPool();
        
	mTracks = NULL;
	mTrack = NULL;
	mTrackCount = 0;
	mVariables = new UserVariables();

	mCustomMode[0] = 0;
	//mPendingProject = NULL;
	//mSaveProject = NULL;
	mAudio = NULL;
	mCapturing = false;
	mCaptureOffset = 0;
	mSynchronizer = NULL;
	mHalting = false;
	mNoExternalInput = false;

    // initialize the object tables
    // some of these use "new" and must be deleted on shutdown
    // move this to initialize() !!
    MobiusMode::initModes();
    Function::initStaticFunctions();
    Parameter::initParameters();
}

/**
 * Release any lingering resources.
 *
 * Formerly required shutdown() to be called first to unwind
 * an awkward interconnection between Recorrder and Track.
 * Don't have that now, so we may not need a separate shutdown()
 */
Mobius::~Mobius()
{
	if (!mHalting) {
        shutdown();
    }
	else {
		Trace(1, "Mobius::~Mobius mHalting was set!\n");
	}

    // things owned by Kernel that can't be deleted
    // mContainer, mAudioPool, mConfig, mSetup

    delete mScriptarian;
    delete mPendingScriptarian;
    
	for (int i = 0 ; i < mTrackCount ; i++) {
		Track* t = mTracks[i];
        delete t;
	}
    delete mTracks;
    
    // subtle delete dependency!
    // Actionator maintains an ActionPool
    // Events can point to the Action that scheduled them
    // EventManager contains Events, and Tracks each have an EventManager
    // When you delete a Track it deletes EventManager which "flushes" any Events
    // that are still active back to the event pool.  If the event is attached
    // to an Action it calls Mobius::completeAction/Actionator::completeAction
    // which normally returns the Action to the pool.  We don't need to be doing
    // pooling when we're destructing everything, but that's old sensitive code I
    // don't want to mess with.  What this means is that Actionator/ActionPool has
    // to be alive at the time Tracks are deleted, so here we have to do this
    // after the track loop above.  This only happens if you have an unprocessed Event
    // and then kill the app/plugin.
    delete mActionator;

    delete mSynchronizer;
    delete mVariables;

    mEventPool->dump();
    delete mEventPool;

    mLayerPool->dump();
    delete mLayerPool;

    // these are now stubs, but we own them
    // do them last since the things above may have listened on them
    delete mMidi;

    // delete dynamically allocated Parameter objects to avoid
    // warning message in Visual Studio
    Parameter::deleteParameters();
}

/**
 * Temporary hack for memory leak debugging.
 * Called by Kernel when it is constructed and before Mobius is constructed.
 * Note that we call these again in the Mobius constructor so they need to
 * be prepared for redundant calls.
 *
 * Can remove this eventually.
 */
void Mobius::initStaticObjects()
{
    MobiusMode::initModes();
    Function::initStaticFunctions();
    Parameter::initParameters();
}

/**
 * Partner to the initStaticObjects memory leak test.
 * Called by Kernel when it is destructed AFTER Mobius is destructed.
 * deleteParameters is also called by ~Mobius so it needs to deal with
 * redundant calls.
 *
 * This was used to test leaks in the static object arrays without
 * instantiating Mobius.  Can be removed eventually.
 */
void Mobius::freeStaticObjects()
{
    Parameter::deleteParameters();
}

/**
 * Called by Kernel during application shutdown to release any resources.
 *
 * Some semantic ambiguity here.
 * In old code this left most of the internal structure intact, it just
 * disconnected from the devices.  The notion being that after
 * you constructed a Mobius, you could call start() and stop() several times
 * then delete it when finally done.
 *
 * I don't think we need to retain that, the current assumption is that you
 * won't call shutdown until you're ready to delete it, and if that holds
 * this could all just be done in the destructore.
 */
void Mobius::shutdown()
{
	mHalting = true;

	// no more events, especially important if clocks are being received
	if (mMidi != NULL) {
		mMidi->setListener(NULL);
		// Transport should have done this but make sure
		mMidi->setClockListener(NULL);
	}

	// sleep to make sure we're not in a timer or midi interrupt
    // MobiusContainer can do this now
	SleepMillis(100);

	// paranioa to help catch shutdown errors
	for (int i = 0 ; i < mTrackCount ; i++) {
		Track* t = mTracks[i];
		t->setHalting(true);
	}

    // old comments:
    // !! clear the Layer pool?  Not if we're in a VST and will
	// resume again later...
	// this could cause large leaks
}

int Mobius::getParameter(UIParameter* p, int trackNumber)
{
    return mActionator->getParameter(p, trackNumber);
}

//////////////////////////////////////////////////////////////////////
//
// Initialization
//
// Code in this area is called by Kernel during the initialization phase
// before the audio stream is active.  It will only be called once
// during the Mobius lifetime.
//
//////////////////////////////////////////////////////////////////////

/**
 * New more streamlined initialization interface.
 * Performed in the UI thread before the audio thread and maintenance
 * threads are active.
 *
 * May not actually be in the UI event loop at this point, still during
 * supervisor initialization which Juce controls.  Unclear the thread
 * but we're allowed to do anything at this point. 
 *
 * We are allowed to allocate memory.
 * 
 */
void Mobius::initialize(MobiusConfig* config)
{
    // can save this until the next call to reconfigure()
    mConfig = config;

    // Sanity check on some important parameters
    // TODO: Need more of these...
    if (mConfig->getTracks() <= 0) {
        // don't see a need to be more flexible here
        int newCount = 1;
        Trace(1, "Mobius::initialize Missing track count, adjusting to %d\n", newCount);
        mConfig->setTracks(newCount);
    }

    // determine the Setup to use, bootstrap if necessary
    // sets mSetup
    locateRuntimeSetup();
    
    // will need a way for this to get MIDI
    mSynchronizer = new Synchronizer(this, mMidi);
    
	// Build the track list
    initializeTracks();

    // compile and install scripts, builds the ScriptLibrary
    // !! be consistent about where subcomponents are allocated
    // in the Mobius constructor like Scriptarian, or here
    // like Synchronizer
    mScriptarian->initialize(config);
    
    // common, thread safe configuration propagation
    propagateConfiguration();
}

/**
 * Utility to determine the Setup to use, with warnings about misconfiguration
 * and Bootstrapping if necessary.
 *
 * Using the unusual "locate" name here to distinguish this from getSetup() which
 * just returns the cached pointer.  This is intended for use only by
 * initialize() and reconfigure().
 *
 * This just sets mSetup, it does not propagate any parameters it contains.
 */
void Mobius::locateRuntimeSetup()
{
    const char* name = mConfig->getActiveSetup();
    mSetup = mConfig->getSetup(name);
    if (mSetup == nullptr) {
        // not normal, might be in test environments using a minimal config
        // or the name may be stale
        if (name != nullptr) {
            Trace(1, "Mobius::initialize Invalid Setup name %s\n", name);
        }
        // fall back to the first Setup
        mSetup = mConfig->getSetups();
        if (mSetup == nullptr) {
            // REALLY minimal config, fake one up and put it on the config so
            // it doesn't leak, should not happen normally
            Trace(1, "Mobius::initialize Bootstrapping initial Setup %s\n", name);
            mSetup = new Setup();
            mSetup->setName("Bootstrap");
            mConfig->setSetups(mSetup);
        }
    }
}

/**
 * Called by initialize() to set up the tracks for the first time.
 * We do not yet support incremental track restructuring so MobiusConfig
 * changes that alter the track count will have no effect until after restart.
 */
void Mobius::initializeTracks()
{
    int count = mConfig->getTracks();

    // should have caught misconfigured count earlier
    if (count > 0) {

        // limit this while testing leaks
        //count = 1;

        Track** tracks = new Track*[count];

        for (int i = 0 ; i < count ; i++) {
            Track* t = new Track(this, mSynchronizer, i);
            tracks[i] = t;
        }
        mTracks = tracks;
        mTrack = tracks[0];
        mTrackCount = count;

        // todo: we don't have to wait for propagateConfiguration
        // to set the active track from the Setup but since we need
        // to share that with reconfigure() do it there
    }
}

//////////////////////////////////////////////////////////////////////
//
// Reconfiguration
//
// This is called by Kernel after we have been running to assimilate
// limited changes to a modified MobiusConfig.
//
//////////////////////////////////////////////////////////////////////

/**
 * Assimilate selective changes to a MobiusConfig after we've been running.
 * Called by Kernel in the audio thread before sending buffers so we can
 * set up a stable state before beginAudioInterrupt and containerAudioAvailable
 * are called.
 * 
 * We formerly allowed a lot in here, like recompiling scrdipts and rebuilding
 * the Track array for changes in the Setup's track count. Now this is only allowed
 * to propagate parameter changes without doing anything expensive or dangerous.
 *
 * mConfig and mSetup will be changed.  Internal components are not allowed
 * to maintain pointers into those two objects.
 *
 * There is some ambiguity between what should be done here and what should
 * be done soon after in beginAudioInterrupt.  Old code deferred a lot of
 * configuration propagation to the equivalent of beginAudioInterrupt. Anything
 * related to MobiusConfig changes should be done here, and beginAudioInterrupt
 * only needs to concern itself with audio consumption.
 *
 */
void Mobius::reconfigure(class MobiusConfig* config)
{
    mConfig = config;
    
    locateRuntimeSetup();

    propagateConfiguration();
}

/**
 * Propagate non-structural configuration to internal components that
 * cache things from MobiusConfig.
 * 
 * mConfig and mSetup will be set.
 */
void Mobius::propagateConfiguration()
{
    // cache various parameters directly on the Function objects
    propagateFunctionPreferences();

    // Synchronizer needs maxSyncDrift, driftCheckPoint
    if (mSynchronizer != NULL)
      mSynchronizer->updateConfiguration(mConfig);

    // Modes track altFeedbackDisables
    MobiusMode::updateConfiguration(mConfig);

    // used to allow configuration of fade length
    // should be hidden now and can't be changed randomly
    // this is defined by Audio and should be done in Kernel since
    // it owns Audio now
	AudioFade::setRange(mConfig->getFadeFrames());

    // Tracks are messy AF
    // old code did a combination of these in unclear order
    //
    // Track::updateConfiguration
    // setTrack(setup->getActiveTrack)
    // Track::setSetup

	// If we were editing the Setups, then it is expected that we
	// change the selected track if nothing else is going on
    // !! seems like there should be more here, for every track in reset
    // the setup changes could be immediately propagated?
    bool allReset = true;
    for (int i = 0 ; i < mTrackCount ; i++) {
        Track* t = mTracks[i];
        Loop* l = t->getLoop();
        if (l != nullptr) {
            if (!l->isReset()) {
                allReset = false;
                break;
            }
        }
    }
    
    if (allReset) {
        int initialTrack = mSetup->getActiveTrack();
        setTrack(initialTrack);
    }

    // tracks are sensitive to lots of things in the Setup
    // they will look at Setup::loopCount and adjust the number of loops
    // in each track, but this is done within a fixed array and won't
    // allocate memory.  It also won't adjust tracks that are still doing
    // something with audio
    // !! not sure if this is correctly allowing tracks to process fade tails
    // An track can be in Reset, but still "playing" a fade out, if we
    // take that loop away it would click
	for (int i = 0 ; i < mTrackCount ; i++) {
		Track* t = mTracks[i];
		t->updateConfiguration(mConfig);
	}

    // this is old and still used so I'm keeping what it
    // does encapsulated there until we figure out how the
    // hell Setup propagation should work
    // this will call setTrack() again to set the active track
    setSetupInternal(mSetup);
}

/**
 * Cache some function sensitivity flags from the MobiusConfig
 * directly on the Function objects for faster testing.
 *
 * Note that for the static Functions, this can have conflicts
 * with multiple instances of the Mobius plugin, but the conflicts
 * aren't significant and that never happens in practice.
 *
 * Would like to move focusLock/group behavor up to the UI.
 */
void Mobius::propagateFunctionPreferences()
{
    // Function sensitivity to focus lock
    StringList* names = mConfig->getFocusLockFunctions();
    Function** functions = mScriptarian->getFunctions();
    
    for (int i = 0 ; functions[i] != NULL ; i++) {
        Function* f = functions[i];
        // always clear this if not in the config
        f->focusLockDisabled = false;
        
        // ugh, so many awkward double negatives
        // 
        // noFocusLock means the fuction will never respond to focus lock
        // so we don't have to consider it, but then it shouldn't have been
        // on the name list in the first place right?
        //
        // eventType != RunScriptEvent means to always allow script
        // functions to have focus lock and ignore the config
        // this seems wrong, why wouldn't you want to selectively
        // allow scripts to disable focus lock?
        if (names != nullptr &&
            !f->noFocusLock &&
            f->eventType != RunScriptEvent) {
            
            // disable focus lock if the function is not in the name list
            f->focusLockDisabled = !(names->containsNoCase(f->getName()));
        }
    }

    // Functions that can cancel Mute mode
	names = mConfig->getMuteCancelFunctions();
	for (int i = 0 ; functions[i] != NULL ; i++) {
		Function* f = functions[i];
		if (f->mayCancelMute) {
			if (names == NULL)
			  f->cancelMute = false;
			else
			  f->cancelMute = names->containsNoCase(f->getName());
		}
	}

    // Functions that can be used for Switch confirmation
	names = mConfig->getConfirmationFunctions();
	for (int i = 0 ; functions[i] != NULL ; i++) {
		Function* f = functions[i];
		if (f->mayConfirm) {
			if (names == NULL)
			  f->confirms = false;
			else
			  f->confirms = names->containsNoCase(f->getName());
		}
	}
}

//
// Setup Propagation
// This is a mess but we need the following set of functions
// to be called by internal components that want to change setups
// This is almost always done by edited the MobiusConfig but
// there is still support for this by setting a Parameter
// or in Scripts, need to clean this up!
//

/**
 * Unconditionally changes the active track.  
 *
 * This is not part of the public interface.  If you want to change
 * tracks with EmptyTrackAction behavior create an Action.
 *
 * Used by propagateConfiguration and also by Loop.
 *
 * !!! rename this
 */
void Mobius::setTrack(int index)
{
    if (index >= 0 && index < mTrackCount) {
        mTrack = mTracks[index];
    }
}

/**
 * Formerly called from recorderMonitorEnter to change
 * to a "pending" setup.
 */
void Mobius::setSetupInternal(int index)
{
    Setup* setup = GetSetup(mConfig, index);
    if (setup == NULL)
      Trace(1, "ERROR: Invalid setup number %ld\n", (long)index);
    else
      setSetupInternal(setup);
}

/**
 * Old code called from severl places.
 * Pulled this up to be near reconfigure() which needed it but can also
 * be called randomly to change the selected setup.
 *
 * Activate a new setup.
 *
 * Old comments about phasing mInterruptConfig:
 * This MUST be called within the interrupt and the passed Setup
 * object must be within mInterruptConfig.
 * This can be called from these places:
 *
 *     - loadProjectInternal to select the setup stored in the project
 *     - ScriptSetupStatement to select a setup in a script
 *     - recorderMonitorEnter to process mPendingSetup
 *     - unitTestSetup to select the unit test setup
 *
 * Newer comments:
 // need to track the selection here so Reset processing
 // can go back to the last setup
 // !! this looks dangerous, what does it do?
 // and how is it different from Track::updateConfiguration above?
 // what this does...
 //   gets the SetupTrack for this track
 //   if loop is reset
 //      resetParameters
 //   else
 //      reset ports, name, group
 //   setPreset(getStartingPreset)
 //      copies the Preset into the private track Preset
 //      setupLoops
 //        resets active loops and sets the loop count within
 //        the fixed maximum range
 //
 // yes, Track::updateConfiguration will also set the Preset
 * 
 */
void Mobius::setSetupInternal(Setup* setup)
{
	if (setup != NULL) {
        // need to track the selection here so Reset processing
        // can go back to the last setup

        // !! not necessary, we're all sharing the same MobiusConfig object
        // mInterruptConfig->setCurrentSetup(setup);

        for (int i = 0 ; i < mTrackCount ; i++) {
            Track* t = mTracks[i];
            t->setSetup(setup);
        }

        // formerly in a function called propagateSetupGlobals
        // things that aren't track specific
        // propagateSetupGlobals(setup);   
        setTrack(setup->getActiveTrack());
    }
}

/**
 * Install a new set of scripts after we've been running.
 * The shell built an entirely new Scriptarian and we need to
 * splice it in.  The process is relatively simple as long as
 * nothing is allowed to remember things inside the Scriptarian.
 *
 * The tricky part is that scripts may currently be running which means
 * the existing ScriptRuntime inside the existing Scriptarian may be busy.
 *
 * Usually you only reload scripts when the core is in a quiet state
 * but we can't depend on that safely.
 *
 * If the current Scriptarian is busy, wait until it isn't.
 */
void Mobius::installScripts(Scriptarian* neu)
{
    if (mPendingScriptarian != nullptr) {
        // we've apparnetly been busy and someone just keeps
        // sending them down, ignore the last one
        mKernel->returnScriptarian(mPendingScriptarian);
        mPendingScriptarian = nullptr;
        Trace(1, "Pending Scriptarian was not consumed before we received another!\n");
        Trace(1, "This may indiciate a hung script\n");
    }

    if (mScriptarian->isBusy()) {
        // wait, beginAudioInterrupt will install it when it can
        mPendingScriptarian = neu;
    }
    else {
        mKernel->returnScriptarian(mScriptarian);
        mScriptarian = neu;
    }
}

//////////////////////////////////////////////////////////////////////
//
// Audio Interrupt
//
//////////////////////////////////////////////////////////////////////

/**
 * Called by Kernel at the start of each audio block processing notification.
 *
 * This is vastly simplified now that we don't have Recorder sitting in the
 * middle of everything.
 * 
 * Things related to "phasing" configuration from the UI thread to the audio
 * thread are gone and now done in reconfigure()
 *
 * We can assume internal components are in a stable state regarding configuration
 * options and only need to concern ourselves with preparing for audio
 * block housekeeping.
 *
 * Old comments:
 *
 * !! Script Recording Inconsistency
 *
 * This is implemented assuming that we only record functions for the
 * active track.  In theory, if a burst of functions came in within
 * the same interrupt, something like this could happen:
 *
 *      NextTrack
 *      Record
 *      NextTrack
 *      Record
 *
 * The effect would be that there are now pending functions
 * on two tracks, but the script recorder doesn't know how
 * to interleave them with the NextTrack function handled
 * by Mobius.  The script would end up with:
 *
 *      NextTrack
 *      NextTrack
 *      Record
 *      Record
 *
 * We could address this by always posting functions to a list
 * in the order they come in and wait for the interrupt
 * handler to consume them.  But it's complicated because we have
 * to synchronize access to the list.    In practice, it is very
 * hard to get functions to come in this rapidly so there
 * are more important things to do right now.  Also, Track
 * only allows one function at a time.
 */
void Mobius::containerAudioAvailable(MobiusContainer* cont, UIAction* actions)
{
    // pre-processing
    beginAudioInterrupt(actions);

    // advance the tracks
    //
    // if we have a TrackSync master, process it first
    // in the old Recorder model this used RecorderTrack::isPriority
    // to process those tracks first, it supported more than one for
    // SampleTrack, but I don't think more than one Track can have priority
    
    Track* master = nullptr;
	for (int i = 0 ; i < mTrackCount ; i++) {
		Track* t = mTracks[i];
        if (t->isPriority()) {
            if (master != nullptr) {
                Trace(1, "Mobius: More than one priority track!\n");
            }
            else {
                master = t;
            }
        }
    }

    if (master != nullptr)
      master->containerAudioAvailable(cont);

	for (int i = 0 ; i < mTrackCount ; i++) {
		Track* t = mTracks[i];
        if (t != master) {
            t->containerAudioAvailable(cont);
        }
    }

    // post-processing
    endAudioInterrupt();
}

/**
 * Get things ready for the tracks to process the audio stream.
 * Approximately what the old code called recorderMonitorEnter.
 *
 * We have a lot less to do now.  Configuration phasing, and action
 * scheduling has already been done by Kernel.
 *
 * The UIActions received through the KernelCommunicator were queued
 * and passed down so we can process them in the same location as the
 * old code.
 */
void Mobius::beginAudioInterrupt(UIAction* actions)
{
    // old flag to disable audio processing when a halt was requested
    // don't know if we still need this but it seems like a good idea
	if (mHalting) return;

    // phase in a new scriptarian if we're not busy
    if (mPendingScriptarian != nullptr) {
        if (!mScriptarian->isBusy()) {
            mKernel->returnScriptarian(mScriptarian);
            mScriptarian = mPendingScriptarian;
            mPendingScriptarian = nullptr;
        }
        else {
            // wait for a future interrupt when it's quiet
            // todo: if a script is waiting on something, and the
            // wait was misconfigured, or the UI dropped the ball on
            // an event, this could cause the script to hang forever
            // after about 10 seconds we should just give up and
            // do a global reset, or at least cancel the active scripts
            // so we can move on
        }
    }
    
	// Hack for testing, when this flag is set remove all external input
	// and only pass through sample content.  Necessary for repeatable
	// tests so we don't get random noise in the input.
    // yes, this is necessary for the unit tests
	if (mNoExternalInput) {
		long frames = mContainer->getInterruptFrames();
        // !! assuming 2 channel ports
		long samples = frames * 2;
		float* input;
		mContainer->getInterruptBuffers(0, &input, 0, NULL);
		memset(input, 0, sizeof(float) * samples);
	}

	mSynchronizer->interruptStart(mContainer);

	// prepare the tracks before running scripts
    // this is a holdover from the old days, do we still need
    // this or can we just do it in Track::containerAudioAvailable?
    // how would this be order sensitive for actions?
	for (int i = 0 ; i < mTrackCount ; i++) {
		Track* t = mTracks[i];
		t->prepareForInterrupt();
	}

    // do the queued actions
    mActionator->doInterruptActions(actions, mContainer->getInterruptFrames());

	// process scripts
    mScriptarian->doScriptMaintenance();
}

/**
 * Called by Kernel at the end of the audio interrupt for each buffer.
 * All tracks have been processed.
 *
 * Formerly known as recorderMonitorExit
 */
void Mobius::endAudioInterrupt()
{
    // don't need this any more?
	if (mHalting) return;

    long frames = mContainer->getInterruptFrames();
	mSynchronizer->interruptEnd();
	
	// if we're recording, capture whatever was left in the output buffer
	// !! need to support merging of all of the output buffers for
	// each port selected in each track
	if (mCapturing && mAudio != NULL) {
		float* output = NULL;
		mContainer->getInterruptBuffers(0, NULL, 0, &output);
		if (output != NULL) {

			// debugging capture
            // this was convenient at the time, and I'd like to support it now
            // but need to give MobiusContainer an interface for file handling
#if 0            
			static int bufcount = 1;
			if (false && bufcount < 5) {
				char file[128];
				sprintf(file, "record%d-%ld.wav", bufcount++, mAudio->getFrames());
                Audio* temp = mAudioPool->newAudio();
                temp->append(output, frames);
                temp->write(file);
                temp->free();
			}
#endif
            
			// the first block in the recording may be a partial block
			if (mCaptureOffset > 0) {
                // !! assuming 2 channel ports
                int channels = 2;
				output += (mCaptureOffset * channels);
				frames -= mCaptureOffset;
				if (frames < 0) {
					Trace(this, 1, "Mobius: Recording offset calculation error!\n");
					frames = 0;
				}
				mCaptureOffset = 0;
			}

			mAudio->append(output, frames);
		}
	}

	// if any of the tracks have requested a UI update, post a message
	// since we're only displaying the beat counter for one track, we don't
	// need to do this for all of them?
	bool uiSignal = false;
	for (int i = 0 ; i < mTrackCount ; i++) {
		if (mTracks[i]->isUISignal())
		  uiSignal = true;
	}
/*    
	if (uiSignal) {
        KernelEvent* e = newKernelEvent();
        e->type = EventTimeBoundary;
        sendKernelEvent(e);
    }
*/
}

//////////////////////////////////////////////////////////////////////
//
// Acctionator support and forwarding
// Provide accessors to the track state Actionator needs
// and forward some older methods used by Scripts to the Actionator.
//
//////////////////////////////////////////////////////////////////////

/**
 * Called by Actionator
 */

Action* Mobius::newAction()
{
    return mActionator->newAction();
}

Action* Mobius::cloneAction(Action* src)
{
    return mActionator->cloneAction(src);
}

void Mobius::completeAction(Action* a)
{
    mActionator->completeAction(a);
}

/**
 * Merging the old doAction and doActionNow styles
 * to just be doActionNow.  Keep both till we transition everything.
 */
void Mobius::doAction(Action* a)
{
    mActionator->doActionNow(a);
}

void Mobius::doActionNow(Action* a)
{
    mActionator->doActionNow(a);
}

Track* Mobius::resolveTrack(Action* a)
{
    return mActionator->resolveTrack(a);
}

//////////////////////////////////////////////////////////////////////
//
// Internal Component Accessors
//
//////////////////////////////////////////////////////////////////////

/**
 * Used by internal components that need something from the container.
 */
MobiusContainer* Mobius::getContainer()
{
    return mContainer;
}

// used by Midi, MidiExporter
MidiInterface* Mobius::getMidiInterface()
{
    return mMidi;
}

Synchronizer* Mobius::getSynchronizer()
{
	return mSynchronizer;
}

/**
 * Return the read-only configuration for internal components to use.
 */
MobiusConfig* Mobius::getConfiguration()
{
	return mConfig;
}

/**
 * Return the read-only Setup currently in use.
 */
Setup* Mobius::getSetup()
{
	return mSetup;
}

/**
 * This is now passed down from Kernel.
 */
AudioPool* Mobius::getAudioPool()
{
    return mAudioPool;
}

LayerPool* Mobius::getLayerPool()
{
    return mLayerPool;
}

EventPool* Mobius::getEventPool()
{
    return mEventPool;
}

int Mobius::getTrackCount()
{
	return mTrackCount;
}

Track* Mobius::getTrack()
{
    return mTrack;
}

Track* Mobius::getTrack(int index)
{
	return ((index >= 0 && index < mTrackCount) ? mTracks[index] : NULL);
}

/**
 * Return the effective input latency.
 * The configuration may override what the audio device reports
 * in order to fine tune actual latency.
 *
 * Called by Track, probably for scheduling.
 * We don't have the "reported" latency concept any more, so always
 * get it from MobiusConfig then fall back to what MobiusContainer has?
 * 
 */
int Mobius::getEffectiveInputLatency()
{
	return  mConfig->getInputLatency();
}

int Mobius::getEffectiveOutputLatency()
{
	return mConfig->getOutputLatency();
}

/**
 * Called by Scripts to ask for a few things from the outside
 * and a handful of Function actions.
 *
 * Allocate a KernelEvent from the pool
 * There aren't many uses of this, could make it use Kernel directly.
 */
KernelEvent* Mobius::newKernelEvent()
{
    return mKernel->newEvent();
}

/**
 * Called by Scripts to send an event returned by newKernelEvent
 * back up to the shell.
 */
void Mobius::sendKernelEvent(KernelEvent* e)
{
    mKernel->sendEvent(e);
}

/**
 * Called by Kernel when the Shell has finished processing a KernelEvent
 * For most events we need to inform the ScriptInterpreters so they can
 * cancel their wait states.
 *
 * This takes the place of what the old code did with special Actions.
 *
 * We do not take ownership of the event or return it to the pool.
 * It is not expected to be modified and no complex side effects should be
 * taking place.
 *
 * Timing should be assumed to be early in the audio interrupt before
 * containerAudioAvailable is called.  Might want to stage these and pass
 * them to constainerAudioAvailable like we do for UIActions so it has more
 * control over when they get done, but we're only using these for script
 * waits right now and it doesn't matter when they happen as long as it is
 * before doScriptMaintenance.
 */
void Mobius::kernelEventCompleted(KernelEvent* e)
{
    // TimeBoundary can't be waited on
    // !! this should be moved down to ScriptRuntime when that
    // gets finished
    if (e->type != EventTimeBoundary) {

        mScriptarian->finishEvent(e);
    }
}

//////////////////////////////////////////////////////////////////////
//
// Legacy Interface for internal components
//
//////////////////////////////////////////////////////////////////////

/**
 * Called by a few function handlers (originally Mute and Insert, now
 * just Insert to change the preset.  This is an old EDPism that I
 * don't really like.
 */
void Mobius::setPresetInternal(int number)
{
    mTrack->setPreset(number);
}

/**
 * Get the active track number.
 * Used by TrackParameter "activeTrack" to get the ordinal of the active track.
 * Also used by Synchronizer for some reason, it could just use getTrack() ?
 */
int Mobius::getActiveTrack()
{
    return (mTrack != NULL) ? mTrack->getRawNumber() : 0;
}

/**
 * Return the sample rate.
 * Whoever needs should just access MobiusContainer directly
 */
int Mobius::getSampleRate()
{
    return mContainer->getSampleRate();
}

/**
 * Return the set of user defined global variables.
 */
UserVariables* Mobius::getVariables()
{
    return mVariables;
}

/**
 * Implementation of the util/TraceContext interface.
 * Supply track/loop time.
 */
void Mobius::getTraceContext(int* context, long* time)
{
	*context = 0;
	*time = 0;
}

/**
 * Return true if the given track has input focus.
 * Prior to 1.43 track groups had automatic focus
 * beheavior, now you have to ask for that with the
 * groupFocusLock global parameter.
 *
 * UPDATE: Really want to move the concept of focus up to the UI
 * and have it just replicate UIActions to focused tracks
 * rather than doing it down here.
 */
bool Mobius::isFocused(Track* t) 
{
    int group = t->getGroup();

    return (t == mTrack || 
            t->isFocusLock() ||
            (mConfig->isGroupFocusLock() && 
             group > 0 && 
             group == mTrack->getGroup()));
}

/**
 * Used only during Script linkage to find a Parameter
 * referenced by name.
 */
Parameter* Mobius::getParameter(const char* name)
{
    return Parameter::getParameter(name);
}

/**
 * Search the dynamic function list.
 * This is only used by Script for an obscure wait state.
 * Comments indiciate that "Wait function" was never used and may not
 * work, but there is other code around waiting for a Switch to happen.
 * Need to sort this out.
 *
 * Now that this is encapsulated in Scriptarian find a way for
 * Scripts to reference that instead so we don't need this
 * pass through.
 */
Function* Mobius::getFunction(const char * name)
{
    return mScriptarian->getFunction(name);
}

/**
 * The loop frame we're currently "on"
 */
  
long Mobius::getFrame()
{
	return mTrack->getFrame();
}

MobiusMode* Mobius::getMode()
{
	return mTrack->getMode();
}

/****************************************************************************
 *                                                                          *
 *                                   STATE                                  *
 *                                                                          *
 ****************************************************************************/

/**
 * Refresh and return the full MobiusState object.
 * Called at regular intervals by the UI refresh thread.
 * We could just let the internal MobiusState object be retained by the
 * caller but this still serves as the mechanism to refresh it.
 */

MobiusState* Mobius::getState()
{
	MobiusState* s = &mState;

	// why not just keep it here?
    // this got lost, if you want it back just let this be the main location for it
	//strcpy(mState.customMode, mCustomMode);

	mState.globalRecording = mCapturing;

    // OG Mobius only refreshed the active track, now we do all of them
    // since the TrackStrips will want most things
    
    for (int i = 0 ; i < mTrackCount ; i++) {
        Track* t = mTracks[i];
        MobiusTrackState* tstate = &(mState.tracks[i]);
        t->getState(tstate);
    }
    
    mState.activeTrack = getActiveTrack();

	return &mState;
}

/**
 * Formerly called by MobiusThread to do periodic status logging.
 * can do it in performMaintenance now, but the maintenance thread
 * is not supposed to have direct access to Mobius and it's internal
 * components.  Needs thought...
 *
 * This used an old TraceBuffer that was useless since it used printf
 * Need to revisit this since it is a useful thing but needs to reliably
 * use buffered Trace records and the emergening DebugWindow.
 */
void Mobius::logStatus()
{
#if 0    
    // !!!!!!!!!!!!!!!!!!!!!!!!
    // we are leaking audio buffers and all kinds of shit
    // if this is a plugin, figure out how we reference count
    // static caches

    printf("*** Mobius engine status:\n");

    mActionator->dump();
    
    mEventPool->dump();
    mLayerPool->dump();
    mAudioPool->dump();

    TraceBuffer* b = new TraceBuffer();
	for (int i = 0 ; i < mTrackCount ; i++) {
		Track* t = mTracks[i];
		t->dump(b);
	}
    b->print();
    delete b;

    fflush(stdout);
#endif    
}

/**
 * Intended for use in scripts to override the usual mode display
 * if the script enters some arbitrary user-defined mode.
 * !! should this be persisted?
 */
void Mobius::setCustomMode(const char* s)
{
	strcpy(mCustomMode, "");
	if (s != NULL) {
		int len = strlen(s);
		if (len < MAX_CUSTOM_MODE - 1) 
		  strcpy(mCustomMode, s);
	}
}

const char* Mobius::getCustomMode()
{
	const char* mode = NULL;
	if (mCustomMode[0] != 0)
	  mode = mCustomMode;
	return mode;
}

/****************************************************************************
 *                                                                          *
 *   							   SCRIPTS                                  *
 *                                                                          *
 ****************************************************************************/

/**
 * RunScriptFunction global function handler.
 * RunScriptFunction::invoke calls back to to this.
 */
void Mobius::runScript(Action* action)
{
    // everything is now encapsulated in here
    mScriptarian->runScript(action);
}

void Mobius::resumeScript(Track* t, Function* f)
{
    mScriptarian->resumeScript(t, f);
}

void Mobius::cancelScripts(Action* action, Track* t)
{
    mScriptarian->cancelScripts(action, t);
}

/**
 * Convey a message to the UI from a Script.
 * This isn't necessarily just for scripts, think about other uses
 * for this now that we have it
 *
 * !! How did this ever work, we can't just call listeners from within
 * the audio thread.
 */
void Mobius::addMessage(const char* msg)
{
    mScriptarian->addMessage(msg);
}

/****************************************************************************
 *                                                                          *
 *   					   SCRIPT CONTROL VARIABLES                         *
 *                                                                          *
 ****************************************************************************/

bool Mobius::isNoExternalInput()
{
	return mNoExternalInput;
}

/**
 * Called indirectly by the NoExternalAudio script variable setter.
 *
 * wtf was this?
 */
void Mobius::setNoExternalInput(bool b)
{
	mNoExternalInput = b;

	// test hack, if we're still in an interrupt, zero out the last 
	// input buffer so we can begin recording immediately
	if (b) {
		long frames = mContainer->getInterruptFrames();
        // !! assuming 2 channel ports
		long samples = frames * 2;
		float* inbuf;
		float* outbuf;

		// always port 0, any need to change?
		mContainer->getInterruptBuffers(0, &inbuf, 0, &outbuf);

		memset(inbuf, 0, sizeof(float) * samples);
	}
}

/****************************************************************************
 *                                                                          *
 *                              GLOBAL FUNCTIONS                            *
 *                                                                          *
 ****************************************************************************/

/**
 * Eventually called by KernelEvent handling to implement SaveLoop.
 *
 * Obvsiously serious race conditions here, but relatively safe
 * as long as you don't do a Reset while it is being saved.  Even then
 * the buffers will be returned to the pool so we should at least
 * not have invalid pointers.
 *
 * !! The Rehearse test scripts can get into a race condition
 * of they SaveLoop at the exact end of the loop when we're
 * about to enter another record phase.
 */
Audio* Mobius::getPlaybackAudio()
{
    Audio* audio = mTrack->getPlaybackAudio();

    // since this might be saved to a file make sure the
    // sample rate is correct
	if (audio != NULL)
	  audio->setSampleRate(getSampleRate());

    return audio;
}

/**
 * GlobalReset function handler.  This isn't a "global" function
 * even though it has global in the name.  This will always be scheduled
 * on a track and be called from within the interrupt.
 */
void Mobius::globalReset(Action* action)
{
	// let action be null so we can call it internally
	if (action == NULL || action->down) {

        // reset global variables
        mVariables->reset();

		// reset all tracks
		for (int i = 0 ; i < mTrackCount ; i++) {
			Track* t = mTracks[i];
			t->reset(action);

            // also reset the variables until we can determine
            // whether TrackReset should do this
            UserVariables* vars = t->getVariables();
            vars->reset();
		}

		// return to the track selected int the setup
		int initialTrack = 0;
		Setup* setup = GetCurrentSetup(mConfig);
		if (setup != NULL)
		  initialTrack = setup->getActiveTrack();
		setTrack(initialTrack);

		// cancel in progress audio recordings	
		// or should we leave the last one behind?
		if (mAudio != NULL)
		  mAudio->reset();
		mCapturing = false;

		// post a thread event to notify the UI
        // UPDATE: can't imagine this is necessary, UI thread will
        // refresh every 1/10th, why was this so important?
        // this caused a special callback notifyGlobalReset
        // and that went nowhere, so this was never used
		//ThreadEvent* te = new ThreadEvent(TE_GLOBAL_RESET);
		//mThread->addEvent(te);

        // Should we reset all sync pulses too?
        mSynchronizer->globalReset();
	}
}

/**
 * Called by some function handlers to cancel global mute mode.
 * This happens whenever we start altering mute status in tracks
 * directly before using GlobalMute to restore the last mute state.
 *
 * Giving this an Action for symetry, though since we're called
 * from an event handler won't have one.
 */
void Mobius::cancelGlobalMute(Action* action)
{
	for (int i = 0 ; i < mTrackCount ; i++) {
		Track* t = mTracks[i];
		t->setGlobalMute(false);
		t->setSolo(false);
	}
}

/**
 * Names that used to live somewhere and need someplace better
 */
#define UNIT_TEST_SETUP_NAME "UnitTestSetup"
#define UNIT_TEST_PRESET_NAME "UnitTestPreset"

/**
 * Bootstrap and select a standard unit test setup.
 * This is called only by evaluation of the UnitTestSetup script statement.
 *
 * This is unusual because we're in the interrupt handler but we'll
 * also perform an edit to the master config.
 * 
 * We first bootstrap a Setup named "Unit Test Setup" and "Unit Test Preset"
 * if they don't already exist.  If the setup or preset already exist they
 * are initialized to a standard state.  This initialization prevents
 * test anomolies that can happen if the unit test setup is manually edited.
 *
 * TODO: Ideally we would have a way to install the
 * samples the tests require, for now assume we've got a 
 * captured mobius.xml file for unit tests.  But if we do that
 * then why bother with this?
 *
 * new: leaving this in place because we're going to want it eventually
 * but need to work out why writeConfiguration was needed and
 * move it higher
 *
 */
void Mobius::unitTestSetup()
{
    bool saveConfig = false;
	bool saveSamples = false;

    // first bootstrap the master config
    // !! ordinarilly we try not to do things like write files 
    // in the interrupt handler but since this is just for testing don't
    // bother bifurcating this into a KernelEvent part and an interrupt part
    if (unitTestSetup(mConfig)) {
        // the part we can't do in the new world
        //writeConfiguration(mConfig);
        Trace(1, "Mobius::unitTestSetup can't write the new configuration!\n");
    }
    

    // then apply the same changes to the interrupt config so we
    // can avoid pushing another thing on the history
    unitTestSetup(mConfig);

    // then set and propagate the setup and preset
    // note that all loops have to be reset for the preset to be refreshed
    Setup* setup = GetSetup(mConfig, UNIT_TEST_SETUP_NAME);
    setSetupInternal(setup);

    // can't do this down here
    //if (mListener)
    //mListener->MobiusConfigChanged();}
}

/**
 * Initialize the unit test setup and preset within a config object.
 * This is called twice, once for the master config and once for
 * the interrupt config to make sure they're both in sync without
 * having to worry about cloning and adding to the history list.
 */
bool Mobius::unitTestSetup(MobiusConfig* config)
{
    bool needsSaving = false;

    // boostrap a preset
    Preset* p = GetPreset(config, UNIT_TEST_PRESET_NAME);
    if (p != NULL) {
        p->reset();
    }
    else {
        p = new Preset();
        p->setName(UNIT_TEST_PRESET_NAME);
        config->addPreset(p);
        needsSaving = true;
    }
    // just an ordinal now
    // this no longer exists, need to refine a permanent MobiusConfig
    // notion of what this means
    Trace(1, "Mobius::unitTestSetup can't set the current preset!\n");
    //config->setCurrentPreset(p);

    // boostrap a setup
    Setup* s = GetSetup(config, UNIT_TEST_SETUP_NAME);
    if (s != NULL) {
        s->reset(p);
    }
    else {
        s = new Setup();
        s->setName(UNIT_TEST_SETUP_NAME);
        s->reset(p);
        config->addSetup(s);
        needsSaving = true;
    }
    SetCurrentSetup(config, s->ordinal);

    return needsSaving;
}

/****************************************************************************
 *                                                                          *
 *                                  CAPTURE                                 *
 *                                                                          *
 ****************************************************************************/

/**
 * StartCapture global function handler.
 *
 * Also called by the BounceEvent handler to begin a bounce recording.
 * May want to have different Audios for StartCapture and Bounce,
 * but it's simpler to reuse the same mechanism for both.
 *
 * Here we just set the mCapturing flag to enable recording, 
 * appending the samples to mAudio actually happens down in
 * recorderMonitorExit after all the tracks have had a chance to 
 * contribute.  Note though that on the first block we may
 * actually be somewhere in the middle due event scheduling, and the first
 * part of the block is technically not part of the recording.  The test
 * scripts currently use "Wait block" to avoid this, but BouceEvent needs
 * to be more precise.  The block offset for the first block is stored
 * in mCaptureOffset, used once then reset back to zero.
 */
void Mobius::startCapture(Action* action)
{
	if (!mCapturing) {
		if (mAudio != NULL)
		  mAudio->reset();
        else {
            mAudio = mAudioPool->newAudio();
            mAudio->setSampleRate(getSampleRate());
        }
		mCapturing = true;

		Track* t = resolveTrack(action);
        if (t == NULL)
          t = mTrack;

		mCaptureOffset = t->getProcessedOutputFrames();
	}
}

/**
 * StopCapture global function handler.
 * 
 * Also now used by the BounceEvent handler when we end a bouce record.
 * 
 * If we're in a script, try to be precise about where we end the
 * recording.  Simply turning the flag off will remove all of the
 * current block from the recording, and a portion of it may
 * actually have been included.
 * 
 * UPDATE: Any reason why we should only do this from a script?
 * Seems like something we should do all the time, especially for bounces.
 */
void Mobius::stopCapture(Action* action)
{

	if (mCapturing && mAudio != NULL
		// && action->trigger == TriggerScript
		) {
		float* output = NULL;
		// TODO: merge the interrupt buffers for all port sets
		// that are being used by any of the tracks
		mContainer->getInterruptBuffers(0, NULL, 0, &output);
		if (output != NULL) {
			Track* t = resolveTrack(action);
            if (t == NULL)
              t = mTrack;
			mAudio->append(output, t->getProcessedOutputFrames());
		}
	}

	mCapturing = false;
}

/**
 * SaveCapture global function handler.
 *
 * This expects the file to be passed, or maybe just allows it
 * when called from a script.  Pass control to the shell to do the
 * file IO.
 */
void Mobius::saveCapture(Action* action)
{
    // action won't be non-null any more, if it ever was
    const char* file = NULL;
    if (action != NULL && action->arg.getType() == EX_STRING)
      file = action->arg.getString();

    KernelEvent* e = newKernelEvent();
    e->type = EventSaveAudio;
    e->setArg(0, file);

    // pass the event back up so the script can wait on it
    if (action != NULL)
      action->setKernelEvent(e);

	sendKernelEvent(e);
}

/**
 * Eventually called by KernelEvent to implement the SaveCapture function.
 * 
 * !! We have a race condition with the interrupt handler.  
 * Tell it to stop recording and pause for at least one interupt.
 *
 * Caller MUST NOT DELETE the returned Audio object.  We keep it around
 * for the next time.  
 */
Audio* Mobius::getCapture()
{
	if (mAudio != NULL) {
		mCapturing = false;
		SleepMillis(100);
	}
	return mAudio;
}

/**
 * Hander for BounceEvent.
 *
 * NOTE: Since this relies on the audio recording stuff above have to
 * reconcile the inside/outside interrupt issues.  Think more about this
 * when you redesign bounce.
 *
 * Since all the logic is up here in Mobius, the event handler doesn't
 * do anything other than provide a mechanism for scheduling the call
 * at a specific time.
 *
 * Note that if we are called by the event handler rather than
 * directly by BounceFunction, we won't have a Action so the
 * things we call need to deal with that.
 *
 * Currently using the same mechanism as audio recording, the only difference
 * is that the start/end times may be quantized and how we process the
 * recording after it has finished.
 * 
 * TODO: I was going to support a BounceMode preset parameter that
 * would let you customize the bounce. The default would be to mute all
 * source tracks, another option would be to reset them.  Should we do
 * this we need to decide which of the possible source tracks provides
 * the Preset.  Assume the current track if not changed by the script.
 *
 * Selecting the target track could also be controlled with parameters.
 * Currently we pick the first non-empty track from the left.
 *
 * Try to preserve the cycle length in the bounce target track.  If the
 * length of the bounce track is an even multiple of the cycle length 
 * of the source track(s) preserve the cycle length.
 * 
 * Determining the cycle length of the source tracks is ambiguous because
 * all tracks could have a different cycle length.  Some methods are:
 *
 *  - Let tracks "vote" and the ones with the most common cycle length win.  
 *    Muted tracks should not be allowed to vote.  
 *
 *  - The first unmuted track from the left wins.
 *
 *  - The current track (or script target track) wins, but it may be empty.
 *
 *  - The current track wins if not empty, otherwise first unmuted
 *    track from the left.
 *
 * It feels right to favor the current track if it is not empty.
 * Voting would be nice but complicated, assume for now we can pick
 * the first one from the left.
 */
void Mobius::toggleBounceRecording(Action* action)
{
	if (!mCapturing) {
		// start one, use the same function that StartCapture uses
		startCapture(action);
	}
	else {
		// stop and capture it
		stopCapture(action);
		Audio* bounce = mAudio;
		mAudio = NULL;
		mCapturing = false;

		if (bounce == NULL)
		  Trace(this, 1, "Mobius: No audio after end of bounce recording!\n");
		else {
			// Determine the track that supplies the preset parameters
			// (not actually used right now)
			Track* source = resolveTrack(action);
            if (source == NULL)
              source = mTrack;
			Preset* p = source->getPreset();

			// TODO: p->getBounceMode() should tell us whether
			// to simply mute the source tracks or reset them,
			// for now assume mute
			
			// locate the target track for the bounce
			Track* target = NULL;
			int targetIndex = 0;
			for (int i = 0 ; i < mTrackCount ; i++) {
				Track* t = mTracks[i];
				// formerly would not select the "source" track
				// but if it is empty we should use it?
				//if (t != source && t->isEmpty()) {
				if (t->isEmpty()) {
					target = t;
					targetIndex = i;
					break;
				}
			}

			// determine the number of cycles in the bounce track
			Track* cycleTrack = source;
			if (cycleTrack == NULL || cycleTrack->isEmpty()) {
				for (int i = 0 ; i < mTrackCount ; i++) {
					Track* t = mTracks[i];
					// ignore muted tracks?
					if (!t->isEmpty()) {
						cycleTrack = t;
						break;
					}
				}
			}

			int cycles = 1;
			if (cycleTrack != NULL) {
				Loop* l = cycleTrack->getLoop();
				long cycleFrames = l->getCycleFrames();
				long recordedFrames = bounce->getFrames();
				if ((recordedFrames % cycleFrames) == 0)
				  cycles = recordedFrames / cycleFrames;
			}

			if (target == NULL) {
				// all dressed up, nowhere to go
				delete bounce;
			}
			else {
				// this is raw, have to fade the edge
				bounce->fadeEdges();
				target->setBounceRecording(bounce, cycles);

				// all other tracks go dark
				// technically we should have prepared for this by scheduling
				// a mute jump in all the tracks at the moment the
				// BounceFunction was called.  But that's hard, and at
				// ASIO latencies, it will be hard to notice the latency
				// adjustment.

				for (int i = 0 ; i < mTrackCount ; i++) {
					Track* t = mTracks[i];
					if (t != target)
					  t->setMuteKludge(NULL, true);
				}

				// and make it the active track
				// sigh, the tooling is all set up to do this by index
				setTrack(targetIndex);
			}
		}
	}
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

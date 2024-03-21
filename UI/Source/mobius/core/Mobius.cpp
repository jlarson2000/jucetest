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
#include "Mem.h"

//////////////////////////////////////////////////////////////////////
//
// New Kernel Interface
//
//////////////////////////////////////////////////////////////////////

#define xxx(cls) new cls

/**
 * Build out only the state that can be done reliably in a static initializer.
 * No devices are ready yet.
 */
Mobius::Mobius(MobiusKernel* kernel)
{
    Trace(2, "Mobius::Mobius");

    Action* a = NEW(Action);
    delete a;
    Preset* p = xxx(Preset);
    delete p;

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
    
    //mActionator = new Actionator(this);
    mActionator = NEW1(Actionator, this);
    mScriptarian = NEW1(Scriptarian, this);
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
	mCaptureAudio = NULL;
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
    // see design/capture-bounce.txt
    
	if (mCapturing && mCaptureAudio != NULL) {
		float* output = NULL;
        // note, only looking at port zero
		mContainer->getInterruptBuffers(0, NULL, 0, &output);
		if (output != NULL) {
			// the first block in the recording may be a partial block
			if (mCaptureOffset > 0) {
                // !! assuming 2 channel ports
                int channels = 2;
				output += (mCaptureOffset * channels);
				frames -= mCaptureOffset;
				if (frames < 0) {
					Trace(1, "Mobius: Recording offset calculation error!\n");
					frames = 0;
				}
				mCaptureOffset = 0;
			}

			mCaptureAudio->append(output, frames);
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
// Capture and Bounce
//
//////////////////////////////////////////////////////////////////////

/**
 * StartCapture global function handler.
 *
 * Also called by the BounceEvent handler to begin a bounce recording.
 * May want to have different Audios for StartCapture and Bounce,
 * but it's simpler to reuse the same mechanism for both.
 *
 * Here we just set the mCapturing flag to enable recording,
 * appending content to mCaptureAudio happens in endAudioInterrupt
 * after all the tracks have had a chance to contribute.
 *
 * Note though that what we include in the capture depends on when
 * the StartCapture function was invoked.  There are two possible times:
 *
 *    1) At the start of the audio interrupt before audio blocks
 *       are being processed.  This happens when a UIAction was received
 *       from above, or when a script runs and initiaites the capture.
 *
 *    2) In the middle of audio block processing if the Function was
 *       scheduled with an Event.  This happens when StartCapture
 *       is quantized, or when it is invoked from a script that has been
 *       waiting for a particular time.
 *
 * If we're in case 2, the first part of the audio block that has already
 * been consumed is technically not part of the recording.  The test scripts
 * currently use "Wait block" to avoid this and have predictable results.
 * 
 * But the Bounce function needs to be more precise.  mCaptureOffset is
 * set to the track's processed output frames and used later.
 *
 * todo: That last comment I don't understand.  Bouce was sort of half
 * done anyway so not focusing on that till we get to Bounce.
 *
 * todo: Capture only works for one track, identified in the action.
 * It can be the active track but it can't be a group.  Tests don't
 * need to capture more than one track, but a more general resampling
 * feature might want to.
 */
void Mobius::startCapture(Action* action)
{
    // if we're already capturing, ignore it
    // this currently requires specific Start and Stop functions, could
    // let this toggle like Record and Bounce, but this is only used in
    // scripts right now
	if (!mCapturing) {
		if (mCaptureAudio != NULL) {
            // left behind from the last capture, clear it
            // if not clear already
            mCaptureAudio->reset();
        }
        else {
            mCaptureAudio = mAudioPool->newAudio();
            // this I've always done, not sure how significant it is
            // it probably ends up in metadata in the .wav file 
            mCaptureAudio->setSampleRate(getSampleRate());
        }
		mCapturing = true;

        // if we're not at the start of the interrupt, save
        // the block offset of where we are now
        // todo: I see this gets it from the Track, are there any
        // conditions where Tracks could have different ideas of what
        // "processed output frames" means?  If that is sensntive to things
        // like TimeStretch then it is probably wrong, here we need to
        // and won't work if we ever do multi-track capture
        Track* t = resolveTrack(action);
        if (t == NULL)
          t = mTrack;

		mCaptureOffset = t->getProcessedOutputFrames();
	}
}

/**
 * StopCapture global function handler.
 * 
 * Old comments:
 * Also now used by the BounceEvent handler when we end a bouce record.
 * 
 * If we're in a script, try to be precise about where we end the
 * recording.  Simply turning the flag off will remove all of the
 * current block from the recording, and a portion of it may
 * actually have been included.
 * 
 * UPDATE: Any reason why we should only do this from a script?
 * Seems like something we should do all the time, especially for bounces.
 * :End old comments
 *
 * new: This looks weird, we're asking the track for ProcessedOutputFrames
 * which is the same thing we did in startCapture to set mCaptureOffset.
 * This captures the audio from the start of the block up to whever
 * the current event is in the track.  Fine, but why is this track specific?
 *
 * Also we're only looking at output port zero which may not be the
 * port the track was actually sending to.  
 *
 */
void Mobius::stopCapture(Action* action)
{
	if (mCapturing && mCaptureAudio != NULL
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
			mCaptureAudio->append(output, t->getProcessedOutputFrames());
		}
	}

	mCapturing = false;
}

/**
 * SaveCapture global function handler.
 *
 * The mAudioCapture object has been accumulating audio during
 * audio block processing, and a little at the end from
 * the stopCapture function handler.
 *
 * This expects the file name to be passed as an Action argument
 * which it will be when called from a script.  I suppose
 * this could have also been a bound action from the UI, but you
 * would need to include the file in the binding.
 *
 * I don't know if it does, but we should allow the file to be
 * optional, and have it fall back to the quickSaveFile parameter.
 *
 * The file save is actually performed by the shell through a KernelEvent.
 * We just pass the file name, the even thandler is expected to call down
 * to Mobius::getCapture when it is ready to save.
 *
 * todo: Could avoid the extra step and just pass mCaptureAudio here
 * but I like keeping the subtle ownersip window of mCaptureAudio
 * smaller.
 *
 * new: this is normally called sometime after StopCapture it called
 * but we could still be within an active capture if the action
 * is being sent from the UI rather than a test script.  Even if it
 * from a script it seems reasonable to start the save process
 * and stop the capture at the same time so you don't have to remember
 * to call StopCapture.  In fact if you don't stop it here, then
 * we can still be in an active capture when Mobius::getCapture
 * is eventually called by the event handler which makes the returned
 * Audio unstable.  So stop it now.
 */
void Mobius::saveCapture(Action* action)
{
    if (mCapturing) {
        // someone forgot to call StopCapture first
        // like stopCapture we have an Action here but
        // there is no guarantee that the target track will be
        // the same  it shouldn't matter as long as
        // Track::getProcessedOutputFrames would be the same
        // for all tracks, which I think it is but I'm not
        // certain about what happens during time stretch modes
        Trace(1, "Warning: saveCapture with active capture, stopping capture\n");
        stopCapture(action);
    }

    // action won't be non-null any more, if it ever was
    const char* file = NULL;
    if (action != NULL && action->arg.getType() == EX_STRING)
      file = action->arg.getString();

    KernelEvent* e = newKernelEvent();
    e->type = EventSaveAudio;
    // this will copy the name to a static buffer on the event
    // so no ownership issues of the string
    e->setArg(0, file);

    if (action != NULL) {
        // here we save the event we're sending up on the Action
        // so the script that is calling us can wait on it
        action->setKernelEvent(e);
    }
    
	sendKernelEvent(e);
}

/**
 * Eventually called by KernelEvent to implement the SaveCapture function.
 * 
 * We are now in the maintenance thread since mCaptureAudio was not copied
 * and passed in the event.  There is a subtle ownership window here
 * that isn't a problem for test scripts but could be if this becomes
 * a more general feature.
 *
 * The maintenance thread expects the Audio object we're returning to
 * remain stable for as long as it takes to save the file.  This
 * means that mCapture must be OFF at this point, which it noramlly will
 * be, but if they're calling SaveCapture from a UI component that isn't
 * necessarily the case.
 *
 * Further, once this method returns, mCaptureAudio should be considered
 * to be in a "checked out" state and any further modifications should
 * be prevented until it is "checked in" later when the KernelEvent
 * sent up by saveCapture is completed.   That happens in
 * Mobius::kernelEventCompleted which right now just informs the
 * script that it can stop waiting.
 *
 * If you want to make this safer, should set a "pending save"
 * flag here and clear it in kernelEventCompleted so more capture
 * can happen.  That does however mean that if a kernel bug fails
 * to complete the event, we could disable future captures forever
 * which isn't so bad.
 *
 * To avoid expensive copying of a large Audio object, the caller
 * MUST NOT DELETE the returned object.  It remains owned by Mobius
 * and should only be used for a short period of time.
 * 
 */
Audio* Mobius::getCapture()
{
    Audio* capture = nullptr;
    
    if (mCapturing) {
        // this isn't supposed to be happening now, this should
        // only be called in response to an EventSaveAudio
        // KernelEvent and that should have stopped it
        Trace(1, "Mobius::getCapture called while still capturing!\n");
    }
    else if (mCaptureAudio == nullptr) {
        // nothing to give, shouldn't be asking if unless you
        // knew it was relevant
        Trace(1, "Mobius: getCapture called without a saved capture\n");
    }
    else {
        capture = mCaptureAudio;

        // todo: here is where the "checkout" concept could be done
        // to prevent further modifications to the capture Audio
        // object while it is out being saved

        // old code had a sleep here, don't remember why that would
        // have been necessary if the capture was stopped properly
		//SleepMillis(100);
	}
	return capture;
}

/**
 * Hander for BounceEvent.
 * See design/capture-bounce.txt
 * 
 * Since all the logic is up here in Mobius, the event handler doesn't
 * do anything other than provide a mechanism for scheduling the call
 * at a specific time.
 *
 * Currently using the same mechanism as audio recording, the only difference
 * is that the start/end times may be quantized and how we process the
 * recording after it has finished.
 * 
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
		Audio* bounce = mCaptureAudio;
		mCaptureAudio = NULL;
		mCapturing = false;

		if (bounce == NULL)
		  Trace(1, "Mobius: No audio after end of bounce recording!\n");
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
                // formerly deleted the entire Audio here which
                // should have returned at least some of it to the AudioPool
                // now, we just put it back so we can continue to use it for
                // future captures
                mCaptureAudio = bounce;
			}
			else {
				// this is raw, have to fade the edge
				bounce->fadeEdges();

                // this is where the ownership transfers
                // it makes it's way to Loop::setBouncRecording
                // which resets itself and builds a single Layer containing
                // the Audio we're passing
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
		if (mCaptureAudio != NULL)
		  mCaptureAudio->reset();
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
 * In old code this saved the modified configuration to the file system.
 * I thought about just requiring that a special mobius.xml for unit tests
 * and always loading that through mobius-redirect.  That's actually
 * happening, but let's continue the old way and fix them if they
 * already exist just to make initial testing less error prone.
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
        //Trace(1, "Mobius::unitTestSetup can't write the new configuration!\n");
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

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

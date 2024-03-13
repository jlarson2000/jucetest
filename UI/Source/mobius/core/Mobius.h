/**
 * Heavily reduced copy of the original code.
 * Mobius now lives entirely in the Kernel and does not have
 * to worry about which thread it is on.
 *
 * MobiusConfig is shared with Kernel
 *
 * Still using some communication classes in MobiusInterface
 * like Prompt and MobiusListener but we are no longer a MobiusInterface implementation.
 */

#pragma once

#include "../../util/Trace.h"
#include "../../model/MobiusState.h"
#include "../Recorder.h"
#include "../Audio.h"
#include "../AudioPool.h"
#include "../MobiusKernel.h"

// for Prompt, OldMobiusListener
#include "OldMobiusInterface.h"

// for ThreadEventType
#include "MobiusThread.h"

// got lost somewhere
#define MAX_CUSTOM_MODE 256

/****************************************************************************
 *                                                                          *
 *                                   MOBIUS                                 *
 *                                                                          *
 ****************************************************************************/

class Mobius : 
    public TraceContext
    //public MidiEventListener, 
    //public RecorderMonitor 
{
	friend class ScriptInterpreter;
	friend class ScriptSetupStatement;
	friend class ScriptPresetStatement;
	friend class ScriptFunctionStatement;
	friend class MobiusThread;
	friend class Loop;
	friend class Track;
	friend class Synchronizer;
	friend class EventManager;
    friend class Function;
    friend class Parameter;

  public:

    //////////////////////////////////////////////////////////////////////
    //
    // Kernel Interface
    //
    // These are the only methods that should be called by the kernel.
    // everything else is legacy and needs to be weeded out.
    //
    //////////////////////////////////////////////////////////////////////

    /**
     * Constructred by Kernel, pull MobiusConfig and other things from there.
     */
	Mobius(class MobiusKernel* kernel);
	~Mobius();

    /**
     * Called by Kernel at a suitable time after construction to flesh out the
     * internal components.
     */
    void initialize(class MobiusConfig* config);

    /**
     * Called by Kernel during application shutdown to release any resources.
     * Should be able to merge this with the destructor but need to think
     * through whether Mobius can be in a suspended state without being destroyed.
     */
    void shutdown();

    /**
     * Called by Kernel after initialization and we've been running and
     * the user has edited the configuration.
     */
    void reconfigure(class MobiusConfig* config);

    /**
     * Called by Kernel at the beginning of each audio block "interrupt".
     * The MobiusKernel and anything we extracted from it in the constructor
     * will still be valid.
     */
    void beginAudioInterrupt();

    /**
     * Called by Kernel at the end of each audio block.
     */
    void endAudioInterrupt();

    /**
     * Called by Kernel to process audio buffers in the current audio block.
     * PENDING: will be used once we get Recorder back down in core.
     *
     * Once we start doing this we no longer need to expose
     * beginAudioInterrupt/endAudioInterrupt
     */
    void containerAudioAvailable(class MobiusContainer* cont);

    /**
     * Process actions using the new UIAction model.
     * This will be internally converted into the old Action model in all
     * it's gory detail.
     *
     * The UIAction has been copied from what was passed by the  UI so
     * it is safe to use, but it is owned by the Kernel and must not
     * be deleted.
     *
     * PENDING: MobiusKernel now does the Action conversion and Function mapping
     * and calls the old doActionNow.
     * 
     * Need to move this down here.
     */
    void doAction(class UIAction* action);

    /**
     * Temporary until we get UIQuery to Export fleshed out.
     * TODO: Like doAction need to move Parameter mapping down here.
     */
    int getParameter(Parameter* p, int trackNumber);


    /**
     * Force initialization of some static object arrays for leak detection.
     * Temporary, and should be done in initialize
     */
    static void initStaticObjects();
    static void freeStaticObjects();

    /**
     * This is still important for the UI but I don't want it to
     * be on a per-track basis.
     */
    class MobiusState* getState(int track);

    // emerging initialization/reconfigure sequence
    
    void Mobius::newInitialize(class MobiusConfig* config);
    // these can be private
    void initializeTracks(int count);
    void initializeScripts(class ScriptConfig* config);
    void initializeFunctions();
    void propagateConfiguration();
    void updateGlobalFunctionPreferences();
    void setTrack(int index);
    
    //////////////////////////////////////////////////////////////////////
    //
    // New environment accessors for internal components
    //
    // These are not part of the Kernel/Mobius interface, but they are
    // new concepts that modified internal components need to access.
    //
    //////////////////////////////////////////////////////////////////////

    /**
     * This now serves the same purposes as the old AudioStream
     */
    class MobiusContainer* getContainer();

    //////////////////////////////////////////////////////////////////////
    // 
    // Legacy Interface
    //
    // Everything from here below are part of the old interface between
    // Mobius and it's internal components.  Need to start weeding this out.
    // In particular MidiInterface needs to be replaced with MobiusContainer
    //
    //////////////////////////////////////////////////////////////////////

    // TODO: I don't think this needs to be public any more, Kernel won't
    // call it 
    void start();
    
    /**
     * Return the shared MobiusConfig for use by internal components.
     *
     * This is shared with Kernel and should have limited modifications.
     * To support changing runtime parameters from
     * scripts, each track will be given a copy of the Preset.  Script changes go
     * into those copies.  On Reset, the original values are restored from this
     * master config.
     *
     * Scripts can also change things in Global config.  This is rare and I don't
     * know if it's worth making an entire copy of the MobiusConfig.  This does
     * however mean that Shell/Kernel can go out of sync.  Need to think about
     * what that would mean.
     */
    class MobiusConfig* getConfiguration();

    /**
     * Return an object that implements the old MidiInterface interface.
     * This is currently stubbed and will become a bridge into MobiusContainer.
     * It probably can't be replaced by MobiusContainer because there is
     * more to it than just a method interface, it also defines the MidiEvent model
     * which is used extensively.
     */
    class MidiInterface* getMidiInterface();

    /**
     * Used in a few places to alter behavior if we're in a plugin
     * vs. running standalone.
     * Try to eliminate this difference down here.
     */
    bool isPlugin() {
        return false;
    }
    
    /**
     * Keep listener support for awhile until we clean up the MobiusThread
     * control handling.  Should be able to get rid of this.
     */
	void setListener(OldMobiusListener* mon);
	OldMobiusListener* getListener();
    
    /**
     * Used to support an interface for assimilating an edited config
     * but only updating internal state for a portion of it.
     * That could still be relevant but we'll assume full reconfigure for now.
     */
	//void setFullConfiguration(class MobiusConfig* config);
	//void setGeneralConfiguration(class MobiusConfig* config);
	//void setPresetConfiguration(class MobiusConfig* config);
	//void setSetupConfiguration(class MobiusConfig* config);
	//void setBindingConfiguration(class MobiusConfig* config);

    /**
     * This I think was an action handler to make it reload script files
     * after the files were modified outside the application.
     * Still want that but it needs to be done in Supervisor where
     * all the other file handling lives.  Make it similar to the way
     * SampleConfig is loaded and passed in.
     */
    void reloadScripts();

    // Triggers and Actions
    // A few of these are used by Kernel but those are being
    // brought down, still used in scripts
    Action* newAction();
    Action* cloneAction(Action* a);
    void freeAction(Action* a);
    void doAction(Action* a);
    // for ScriptInterpreter, some Parameters
    void doActionNow(Action* a);
    void completeAction(Action* a);
    
    // scripts could prompt through the MobiusThread, the UI
    // was supposed to handle that, then call back to this
	void finishPrompt(Prompt* p);

    // forget what these were, similar to MidiExports I think
    // might have been used for plugin parameter export
    // revisit the need for this
    class WatchPoint* addWatcher(class WatchPointListener* l);
    void notifyWatchers(class WatchPoint* wp, int value);

    // Status

    // these should all come from MobiusContainer now
    // don't think we need the difference between "reported" and "effectrive"
	int getEffectiveInputLatency();
	int getEffectiveOutputLatency();
    int getSampleRate();

    // Tracks
    // used internally only
    
    int getTrackCount();
    int getActiveTrack();
    class Track* getTrack(int index);

    // Load/Save
    // all this needs a complete redesign and moved up
    
	void loadLoop(class Audio* a);
	void loadProject(class Project* a);
	class Project* saveProject();
	void saveLoop(const char* name);
    void saveLoop(class Action* action);

    // External bindings

    // scripts may still use this?
    class Export* resolveExport(class Action* a);

    // Object pools
    // needed by internal components
    
    class AudioPool* getAudioPool();
    class LayerPool* getLayerPool();
    class EventPool* getEventPool();

    // Thread
    // threads will be redesigned
    void addEvent(class ThreadEvent* te);
    void addEvent(ThreadEventType type);
    
    //////////////////////////////////////////////////////////////////////
    //
    // Semi-protected methods for function invocation
    //
    //////////////////////////////////////////////////////////////////////
    
    class MobiusConfig* getInterruptConfiguration();
    class Recorder* getRecorder();

	class MobiusMode* getMode();
	long getFrame();

	void setCustomMode(const char* s);
	const char* getCustomMode();

    class Watchers* getWatchers();

	// MidiHandler interface
	void midiEvent(class MidiEvent* e);

    // Object constants

    Parameter** getParameters();
    Parameter* getParameter(const char* name);
    Parameter* getParameterWithDisplayName(const char* name);

    Function** getFunctions();
    Function* getFunction(const char* name);

    MobiusMode** getModes();
    MobiusMode* getMode(const char* name);

	// Function Invocation

    void run(class Script* s);

	// Global functions
	// Only need to be public for the Function handlers

    class Track* resolveTrack(Action* a);

	void globalReset(class Action* action);
	void globalMute(class Action* action);
	void cancelGlobalMute(class Action* action);
	void globalPause(class Action* action);
	void sampleTrigger(class Action* action, int index);
	long getLastSampleFrames();
	void addMessage(const char* msg);
	void runScript(class Action* action);

	void startCapture(class Action* action);
	void stopCapture(class Action* action);
	void saveCapture(class Action* action);

	void toggleBounceRecording(class Action* action);

    void unitTestSetup();

	void resumeScript(class Track* t, class Function* f);
	void cancelScripts(class Action* action, class Track* t);

    // needed by TrackSetupParameter to change setups within the interrupt
    void setSetupInternal(int index);

    // Unit Test Interface

    void setOutputLatency(int l);
	class Track* getSourceTrack();
	void stopRecorder();

	// user defined variables
    class UserVariables* getVariables();

	// script control variables

	// has to be public for NoExternalInputVarialbe
	bool isNoExternalInput();
	void setNoExternalInput(bool b);
	
    // trace

	void getTraceContext(int* context, long* time);
	void logStatus();
    
    // utilities

    class Track* getTrack();
	class Synchronizer* getSynchronizer();
	long getInterrupts();
	void setInterrupts(long i);
	long getClock();

    // for Synchronizer and a few Functions
    Setup* getInterruptSetup();

    // ActionDispatcher, ScriptRuntime
    bool isFocused(class Track* t);
    
  protected:

	// for MobiusThread and others

	Audio* getCapture();
	Audio* getPlaybackAudio();
	void loadProjectInternal(class Project* p);
    class MobiusThread* getThread();
	void notifyGlobalReset();

    // Need these for the Setup and Preset script statements
    void setSetupInternal(class Setup* setup);

    // for some Functions
    void setPresetInternal(int p);

  private:

	void stop();
    bool installScripts(class ScriptConfig* config, bool force);
    void installWatchers();
	void updateBindings();
    void propagateInterruptConfig();
    void propagateSetupGlobals(class Setup* setup);
    bool unitTestSetup(MobiusConfig* config);

	void setConfiguration(class MobiusConfig* config, bool doBindings);
	void installConfiguration(class MobiusConfig* config, bool doBindings);
	class MobiusConfig* loadConfiguration();
    void initFunctions();
    void initScriptParameters();
    void addScriptParameter(class ScriptParamStatement* s);
	void initObjectPools();
	void dumpObjectPools();
	void flushObjectPools();
	void buildTracks(int count);
	void tracePrefix();
	bool isInUse(class Script* s);
	void startScript(class Action* action, Script* s);
	void startScript(class Action* action, Script* s, class Track* t);
	void addScript(class ScriptInterpreter* si);
	class ScriptInterpreter* findScript(class Action* action, class Script* s, class Track* t);
    void doScriptMaintenance();
	void freeScripts();

    void doInterruptActions();
    void doPreset(Action* a);
    void doSetup(Action* a);
    void doBindings(Action* a);
    void doFunction(Action* a);
    void doFunction(Action* action, Function* f, class Track* t);
    void doScriptNotification(Action* a);
    void doParameter(Action* a);
    void doParameter(Action* a, Parameter*p, class Track* t);
    void doControl(Action* a);
    void doUIControl(Action* a);
    void invoke(Action* a, class Track* t);

    //
    // Member Variables
    //

    // Supplied by Kernel
    class MobiusKernel* mKernel;
    class MobiusContainer* mContainer;
    class AudioPool* mAudioPool;
    
    //MobiusContext* mContext;
	//class ObjectPoolManager* mPools;
    class LayerPool* mLayerPool;
    class EventPool* mEventPool;
    class ActionPool* mActionPool;
	OldMobiusListener* mListener;
    Watchers* mWatchers;
    class List* mNewWatchers;
	class MobiusConfig *mConfig;
    class Setup* mInterruptSetup;
	class MobiusConfig *mPendingInterruptConfig;
	class MidiInterface* mMidi;

    class TriggerState* mTriggerState;

	Recorder* mRecorder;
    class MobiusThread* mThread;
    class Track** mTracks;
	class Track* mTrack;
	int mTrackCount;
    int mTrackIndex;
	class UserVariables* mVariables;
	class ScriptEnv* mScriptEnv;
    class Function** mFunctions;
	class ScriptInterpreter* mScripts;
    class Action* mRegisteredActions;
    class Action *mActions;
    class Action *mLastAction;
	bool mHalting;
	bool mNoExternalInput;
	long mInterrupts;
	char mCustomMode[MAX_CUSTOM_MODE];
	class Synchronizer* mSynchronizer;

	// pending project to be loaded
	class Project* mPendingProject;

    // pending samples to install
	//class SamplePack* mPendingSamples;

	// pending project to be saved
	class Project* mSaveProject;
	
    // pending setup to switch to
    int mPendingSetup;

    // number of script threads launched
    int mScriptThreadCounter;

	// state related to realtime audio capture
	Audio* mAudio;
	bool mCapturing;
	long mCaptureOffset;
	
	// state exposed to the outside world
	MobiusState mState;

};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


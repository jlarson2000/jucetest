/**
 * Heavily reduced copy of the original code.
 * Mobius now lives entirely in the Kernel and does not have
 * to worry about which thread it is on.
 *
 * MobiusConfig is shared with Kernel
 *
 * Still using some communication classes in MobiusInterface
 * like MobiusAlerts but we are no longer a MobiusInterface implementation.
 */

#pragma once

#include "../../util/Trace.h"
#include "../../model/MobiusState.h"
#include "../Recorder.h"
#include "../Audio.h"
#include "../AudioPool.h"
#include "../MobiusKernel.h"

// for MobiusAlerts, OldMobiusListener
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
    friend class ActionDispatcher;
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
     * Called by Kernel during application shutdown to release any resources,
     * though at this point since we can't allocate memory there
     * shouldn't be much to do.
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
     *
     * THINK: Might still want to pass an AudioStream wrapper here so kernel
     * has the flexibility to splice in something different for testing.
     */
    void beginAudioInterrupt();

    /**
     * Called by Kernel at the end of each audio block.
     */
    void endAudioInterrupt();

    /**
     * Process actions using the new UIAction model.
     * This will be internally convreted into the old Action model in all
     * it's gory detail.
     *
     * The UIAction has been copied from what was passed by the  UI so
     * it is safe to use, but it is owned by the Kernel and must not
     * be deleted.
     */
    void doAction(class UIAction* action);

    /**
     * Temporary until we get UIQuery to Export fleshed out.
     */
    int getParameter(Parameter* p, int trackNumber);

    static void initStaticObjects();
    static void freeStaticObjects();

    //////////////////////////////////////////////////////////////////////
    // New environment accessors for internal components
    //////////////////////////////////////////////////////////////////////

    /**
     * This now serves the same purposes as AudioStream
     */
    class MobiusContainer* getContainer();

    //////////////////////////////////////////////////////////////////////
    // 
    // Legacy Interface
    //
    // Things that used to be in MobiusInterface. Keep a few of them around
    // for the Kernel/Mobius interface but refactor when ready.
    //
    // Some are used by internal components to get to important objects
    // like AudioStream and MidiInterface
    //
    //////////////////////////////////////////////////////////////////////

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
     * Return an object that implements the old AudioStream interface.
     * This will now be bridge code between core code and MobiusContainer.
     */
    class AudioStream* getAudioStream();

    // formerly a combo of getContxt and getMidiInterface
    class MidiInterface* getMidiInterface();

    // formerly on MobiusContext
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

    Action* newAction();
    Action* cloneAction(Action* a);
    void freeAction(Action* a);
    void doAction(Action* a);
    // for ScriptInterpreter, some Parameters
    void doActionNow(Action* a);
    void completeAction(Action* a);
    
	void setCheckInterrupt(bool b);
	class CalibrationResult* calibrateLatency();
	void finishPrompt(Prompt* p);

    class WatchPoint* addWatcher(class WatchPointListener* l);
    void notifyWatchers(class WatchPoint* wp, int value);

    // Status

	//class MessageCatalog* getMessageCatalog();
    class MobiusState* getState(int track);
    class MobiusAlerts* getAlerts();

	int getReportedInputLatency();
	int getReportedOutputLatency();
	int getEffectiveInputLatency();
	int getEffectiveOutputLatency();

    int getTrackCount();
    int getActiveTrack();
    class Track* getTrack(int index);
	int getTrackPreset();

    int getSampleRate();

    // Load/Save

	void loadLoop(class Audio* a);
	void loadProject(class Project* a);
	class Project* saveProject();
	void saveLoop(const char* name);
    void saveLoop(class Action* action);

    // External bindings

    //class ResolvedTarget* resolveTarget(OldBinding* b);
    //class Action* resolveAction(OldBinding* b);
    //class Export* resolveExport(OldBinding* b);
    //class Export* resolveExport(class ResolvedTarget* t);
    class Export* resolveExport(class Action* a);

    Action* getScriptButtonActions();

    // Object pools

    class AudioPool* getAudioPool();
    class LayerPool* getLayerPool();
    class EventPool* getEventPool();

    // Thread
    void addEvent(class ThreadEvent* te);
    void addEvent(ThreadEventType type);
    
    //////////////////////////////////////////////////////////////////////
    //
    // Semi-protected methods for function invocation
    //
    //////////////////////////////////////////////////////////////////////
    
	//void writeConfiguration();
	class MobiusConfig* getMasterConfiguration();
    class MobiusConfig* getInterruptConfiguration();
    class Recorder* getRecorder();

    // Used by MobiusThread when it needs to access files
	//const char* getHomeDirectory();

	//void setOverlayBindings(class BindingConfig* c);

	class MobiusMode* getMode();
	long getFrame();

	void setCustomMode(const char* s);
	const char* getCustomMode();

    class Watchers* getWatchers();

	// MidiHandler interface

	void midiEvent(class MidiEvent* e);

	// RecorderMonitor interface
	//void recorderMonitorEnter(AudioStream* stream);
	//void recorderMonitorExit(AudioStream* stream);

    // Object constants

    Parameter** getParameters();
    Parameter* getParameter(const char* name);
    Parameter* getParameterWithDisplayName(const char* name);

    Function** getFunctions();
    Function* getFunction(const char* name);
    void updateGlobalFunctionPreferences();

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
    void setTrack(int i);
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
	bool isInInterrupt();
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
	void emergencyExit();
	void notifyGlobalReset();

    // Need these for the Setup and Preset script statements
    void setSetupInternal(class Setup* setup);

    // for some Functions
    void setPresetInternal(int p);

  private:

	void stop();
    bool installScripts(class ScriptConfig* config, bool force);
    void installWatchers();
	//void localize();
	//class MessageCatalog* readCatalog(const char* language);
    //void localizeUIControls();
	void updateBindings();
    void propagateInterruptConfig();
    void propagateSetupGlobals(class Setup* setup);
    bool unitTestSetup(MobiusConfig* config);

    // bool isBindableDifference(class OldBindable* orig, class OldBindable* neu);
	void setConfiguration(class MobiusConfig* config, bool doBindings);
	void installConfiguration(class MobiusConfig* config, bool doBindings);
	//void writeConfiguration(MobiusConfig* config);
	//void parseCommandLine();
	class MobiusConfig* loadConfiguration();
    //class OscConfig* loadOscConfiguration();
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
    class AudioInterface* mAudioInterface;
    class AudioStream* mAudioStream;
    
    //MobiusContext* mContext;
	//class ObjectPoolManager* mPools;
    class LayerPool* mLayerPool;
    class EventPool* mEventPool;
    class ActionPool* mActionPool;
	//class MessageCatalog* mCatalog;
	//bool mLocalized;
	OldMobiusListener* mListener;
    Watchers* mWatchers;
    class List* mNewWatchers;
    //class UIControl** mUIControls;
    //class UIParameter** mUIParameters;
	//char* mConfigFile;
	class MobiusConfig *mConfig;
	class MobiusConfig *mInterruptConfig;
    class Setup* mInterruptSetup;
	class MobiusConfig *mPendingInterruptConfig;
	class MidiInterface* mMidi;

    //class ResolvedTarget* mResolvedTargets;
    class TriggerState* mTriggerState;
    //class OscConfig* mOscConfig;
	//class OscRuntime* mOsc;

	Recorder* mRecorder;
    class MobiusThread* mThread;
    class Track** mTracks;
	class Track* mTrack;
	int mTrackCount;
    int mTrackIndex;
	//class SampleTrack* mSampleTrack;
	class UserVariables* mVariables;
	class ScriptEnv* mScriptEnv;
    class Function** mFunctions;
	class ScriptInterpreter* mScripts;
    class Action* mRegisteredActions;
    class Action *mActions;
    class Action *mLastAction;
	bool mHalting;
	bool mNoExternalInput;
	AudioStream* mInterruptStream;
	long mInterrupts;
	char mCustomMode[MAX_CUSTOM_MODE];
	class Synchronizer* mSynchronizer;
	//class CriticalSection* mCsect;

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
    MobiusAlerts mAlerts;

};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


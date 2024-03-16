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

#include "../Audio.h"
#include "../AudioPool.h"
#include "../MobiusKernel.h"

// for Prompt, OldMobiusListener
#include "OldMobiusInterface.h"

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
{
	friend class ScriptInterpreter;
	friend class ScriptSetupStatement;
	friend class ScriptPresetStatement;
	friend class ScriptFunctionStatement;
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
     * Called by Kernel at the begging of each audio block.
     * What we once called "the interrupt".
     */
    void containerAudioAvailable(class MobiusContainer* cont, UIAction* actions);
    
    /**
     * Temporary until we get UIQuery to Export fleshed out.
     * TODO: Like doAction need to move Parameter mapping down here.
     */
    int getParameter(Parameter* p, int trackNumber);
    int getParameter(class UIParameter* p, int trackNumber);

    /**
     * Process a completed KernelEvent core scheduled earlier.
     */
    void kernelEventCompleted(class KernelEvent* e);

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

    // !! ugly, this is part of the reconfigure() sequence
    // but is also called by Loop, why would it do that?
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
     * Keep listener support for awhile until we finish KernelEvent
     * processing.  Should be able to get rid of most if not all this.
     */
	void setListener(OldMobiusListener* mon);
	OldMobiusListener* getListener();
    
    
    /**
     * Scripts could send a prompt through a KernelEvent, the
     * Shell would send that out to the UI, the UI sends the Prompt
     * back to the Shell, and we finish it here.
     */
	void finishPrompt(Prompt* p);

    // Status

    // these should all come from MobiusContainer now
    // don't think we need the difference between "reported" and "effectrive"
	int getEffectiveInputLatency();
	int getEffectiveOutputLatency();
    int getSampleRate();

    // Tracks
    // used internally only
    // now used by Actionator
    
    int getTrackCount();
    int getActiveTrack();
    class Track* getTrack(int index);

    // Actionator
    class ScriptInterpreter* getScripts();
    long getInterrupts();

    // scripts may still use this?
    class Export* resolveExport(class Action* a);

    // Object pools
    // needed by internal components
    
    class AudioPool* getAudioPool();
    class LayerPool* getLayerPool();
    class EventPool* getEventPool();

    // KernelEvents, passes through to MobiusKernel
    class KernelEvent* newKernelEvent();
    void sendKernelEvent(class KernelEvent* e);
    
    //////////////////////////////////////////////////////////////////////
    //
    // Semi-protected methods for function invocation
    //
    //////////////////////////////////////////////////////////////////////
    
    class MobiusConfig* getInterruptConfiguration();

	class MobiusMode* getMode();
	long getFrame();

	void setCustomMode(const char* s);
	const char* getCustomMode();

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
	void setInterrupts(long i);
	long getClock();

    // for Synchronizer and a few Functions
    Setup* getInterruptSetup();

    // ActionDispatcher, ScriptRuntime
    bool isFocused(class Track* t);
    
    // actions moved to Actionator, but Script and others
    // still want to go through Mobius
    class Action* newAction();
    class Action* cloneAction(class Action* src);
    void completeAction(class Action* a);

    // these are now the same but keep both until we
    // can visit all the callers
    void doAction(Action* a);
    void doActionNow(Action* a);
    
    class Track* resolveTrack(Action* a);

  protected:

	// used by KernelEvent handlers

	Audio* getCapture();
	Audio* getPlaybackAudio();
	//void loadProjectInternal(class Project* p);

    // Need these for the Setup and Preset script statements
    void setSetupInternal(class Setup* setup);

    // for some Functions
    void setPresetInternal(int p);

  private:

    // initialization
    void locateRuntimeSetup();
    void initializeTracks();
    void initializeScripts();
    void initializeFunctions();

    // reconfigure
    void propagateConfiguration();
    void propagateFunctionPreferences();
    
    // audio buffers
    void beginAudioInterrupt(class UIAction* actions);
    void endAudioInterrupt();

    // legacy
    
    bool unitTestSetup(MobiusConfig* config);

	void setConfiguration(class MobiusConfig* config, bool doBindings);
	void initObjectPools();
	void dumpObjectPools();
	void flushObjectPools();
	void tracePrefix();
	bool isInUse(class Script* s);
	void startScript(class Action* action, Script* s);
	void startScript(class Action* action, Script* s, class Track* t);
	void addScript(class ScriptInterpreter* si);
	class ScriptInterpreter* findScript(class Action* action, class Script* s, class Track* t);
    void doScriptMaintenance();
	void freeScripts();

    /*
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
    */
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
	OldMobiusListener* mListener;
	class MobiusConfig *mConfig;
    class Setup* mSetup;
	class MidiInterface* mMidi;
    class Actionator* mActionator;

    class Track** mTracks;
	class Track* mTrack;
	int mTrackCount;
    int mTrackIndex;
	class UserVariables* mVariables;
	class ScriptEnv* mScriptEnv;
    class Function** mFunctions;
	class ScriptInterpreter* mScripts;
	bool mHalting;
	bool mNoExternalInput;
	long mInterrupts;
	char mCustomMode[MAX_CUSTOM_MODE];
	class Synchronizer* mSynchronizer;

	// pending project to be loaded
	//class Project* mPendingProject;

    // pending samples to install
	//class SamplePack* mPendingSamples;

	// pending project to be saved
	//class Project* mSaveProject;
	
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


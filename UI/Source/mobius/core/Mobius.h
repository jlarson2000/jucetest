/**
 * The main component of the Mobius core.
 * 
 * The code between Mobius and roughtly Track is an almost complete
 * rewrite of the original code.  From Track on down is still relatively
 * original.  Script is mostly original.
 *
 * Mobius now lives entirely in the Kernel and does need to deal with
 * thread issues except during the initialize() sequence.
 *
 */

#pragma once

#include "../../model/MobiusState.h"

/**
 * Size of a static char buffer to keep the custom mode name.
 */
#define MAX_CUSTOM_MODE 256

/****************************************************************************
 *                                                                          *
 *                                   MOBIUS                                 *
 *                                                                          *
 ****************************************************************************/

class Mobius
{
  public:

    //////////////////////////////////////////////////////////////////////
    //
    // Kernel Interface
    //
    // These are the only methods that should be called by the kernel.
    // everything else is for internal component access.
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
    void containerAudioAvailable(class MobiusContainer* cont, class UIAction* actions);


    /**
     * Called by Kernel in the middle of an auto block to tell any
     * tracks that an input buffer was modified due to
     * sample injection.
     */
    void Mobius::notifyBufferModified(float* buffer);
    
    /**
     * Temporary until we get UIQuery to Export fleshed out.
     * TODO: Like doAction need to move Parameter mapping down here.
     */
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
     * Refresh and return state for the engine and the active track.
     */
    class MobiusState* getState();

    /**
     * Install a freshly minted Scriptarian when scripts are reloaded
     * after we've been initialized and running.
     */
    void installScripts(class Scriptarian* s);
    
    /**
     * Retrieve the capture audio for the KernelEvent handler
     * to save capture.
     */
    class Audio* getCapture();

    /**
     * Retrieve the contents of the current loop for the KernelEvent
     * handler for SaveLoop.
     */
	class Audio* getPlaybackAudio();

    /**
     * Special interface only for UnitTests
     */
    void slamScriptarian(class Scriptarian* scriptarian);
    
    //////////////////////////////////////////////////////////////////////
    //
    // Environment accessors for internal components
    //
    // These are not part of the Kernel/Mobius interface, but they are
    // things internal components need.
    //
    //////////////////////////////////////////////////////////////////////

    /**
     * This now serves the same purposes as the old AudioStream
     */
    class MobiusContainer* getContainer();

    /**
     * Used by a small number of internal function handlers that forward
     * things back up to the kernel.
     */
    class MobiusKernel* getKernel();

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
     * Return the Setup currently in use.
     */
    class Setup* getActiveSetup();

    /**
     * Ugh, a ton of code uses this old name, redirect
     * until we can change everything.
     */
    class Setup* getSetup() {
        return getActiveSetup();
    }
    
    /**
     * Set the active setup by name or ordinal.
     */
    void setActiveSetup(int ordinal);
    void setActiveSetup(const char* name);

	class Synchronizer* getSynchronizer();
    class AudioPool* getAudioPool();
    class LayerPool* getLayerPool();
    class EventPool* getEventPool();
    class UserVariables* getVariables();

    /**
     * Return an object that implements the old MidiInterface interface.
     * This is currently stubbed and will become a bridge into MobiusContainer.
     * It probably can't be replaced by MobiusContainer because there is
     * more to it than just a method interface, it also defines the MidiEvent model
     * which is used extensively.
     */
    class MidiInterface* getMidiInterface();

    /**
     * Used in a few places that need to calculate tempo relative to frames.
     */
    int getSampleRate();
    // may come from MobiusContainer or overridden in MobiusConfig
	int getEffectiveInputLatency();
	int getEffectiveOutputLatency();

    // Tracks
    // used internally only
    // now used by Actionator
    
    int getTrackCount();
    int getActiveTrack();
    class Track* getTrack();
    class Track* getTrack(int index);

	class MobiusMode* getMode();
	long getFrame();

    // Control over the active track and preset from functions and parameters
    void setActiveTrack(int index);
    void setActivePreset(int ordinal);
    
    // Actions
    class Action* newAction();
    class Action* cloneAction(class Action* src);
    void completeAction(class Action* a);
    // these are now the same but keep both until we
    // can visit all the callers
    void doAction(Action* a);
    void doActionNow(Action* a);
    class Track* resolveTrack(Action* a);
    // ActionDispatcher, ScriptRuntime
    bool isFocused(class Track* t);

    // KernelEvents, passes through to MobiusKernel
    class KernelEvent* newKernelEvent();
    void sendKernelEvent(class KernelEvent* e);
    
    // Scripts, pass through to Scriptarian/ScriptRuntime
	void addMessage(const char* msg);
	void runScript(class Action* action);
    void resumeScript(class Track* t, class Function* f);
    void cancelScripts(class Action* action, class Track* t);

    // needed for Script compilation
    class Parameter* getParameter(const char* name);
    class Function* getFunction(const char* name);

    //////////////////////////////////////////////////////////////////////
    // Global Function Handlers
    //////////////////////////////////////////////////////////////////////
    
	void globalReset(class Action* action);
	void cancelGlobalMute(class Action* action);

    // used to be here, where did they go?
	//void globalMute(class Action* action);
	//void globalPause(class Action* action);
    
	void startCapture(class Action* action);
	void stopCapture(class Action* action);
	void saveCapture(class Action* action);
	void toggleBounceRecording(class Action* action);
    void saveLoop(class Action* action);

    //////////////////////////////////////////////////////////////////////
    // 
    // Legacy Interface
    //
    // Everything from here below are part of the old interface between
    // Mobius and it's internal components.  Need to start weeding this out.
    //
    //////////////////////////////////////////////////////////////////////

	void setCustomMode(const char* s);
	const char* getCustomMode();

	// MidiHandler interface
	void midiEvent(class MidiEvent* e);

    // trace
	void logStatus();

    // used only by InputPortParameter and OutputPortParameter
    bool isPlugin();

  protected:

  private:

    // initialization
    void initializeTracks();

    // reconfigure
    void propagateConfiguration();
    void propagateFunctionPreferences();
    void propagateSetup();
    
    // audio buffers
    void beginAudioInterrupt(class UIAction* actions);
    void endAudioInterrupt();

    //
    // Member Variables
    //

    // Supplied by Kernel
    class MobiusKernel* mKernel;
    class MobiusContainer* mContainer;
    class AudioPool* mAudioPool;

    // stub
    class MidiInterface* mMidi;

    // object pools
    class LayerPool* mLayerPool;
    class EventPool* mEventPool;
    
    class Actionator* mActionator;
    class Scriptarian* mScriptarian;
    class Scriptarian* mPendingScriptarian;
	class Synchronizer* mSynchronizer;
	class UserVariables* mVariables;

    class Track** mTracks;
	class Track* mTrack;
	int mTrackCount;
    
	class MobiusConfig *mConfig;
    class Setup* mSetup;
	char mCustomMode[MAX_CUSTOM_MODE];
    
	// state related to realtime audio capture
	Audio* mCaptureAudio;
	bool mCapturing;
	long mCaptureOffset;
	
	// state exposed to the outside world
	MobiusState mState;
    
	bool mHalting;
    
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


/**
 * An interface that wraps an implementation of the Mobius engine.
 * This is the only way for a user interface to control the engine.
 *
 * It is a significantly reduced and modified veraion of the original
 * interface.  The names of the classes are the same so you won't be
 * able to continue using the old ones as old code is ported over.
 */

#pragma once

/**
 * A listener callback that may be given to the engine to receive
 * notifications when certain interesting things happen.
 */
class MobiusListener {
  public:

	/**
	 * A significant time boundary has passed (beat, cycle, loop)
	 * so refresh time sensitive components now rather than waiting
	 * for the next timer event to make it look more accurate.
	 */
	virtual void MobiusTimeBoundary() = 0;
};

class MobiusInterface {

  public:

    // must have a virtual destructor to avoid a warning
    virtual ~MobiusInterface();
    
    /**
     * Called while the application runs to obtain a handle to the
     * Mobius engine.  It must not be deleted.
     */
    static class MobiusInterface* getMobius();

    /**
     * This must be called once during main application initialization.
     * It is imporant to keep the full startup side effects out of the
     * constructor because that can be a complex process that you
     * can't necessarily do during static initialization.  Further VST hosts
     * typically instantiate a plugin just to probe it for information without
     * actually using it, and we don't want to initialize everything
     * if it won't be used.
     */
    static void MobiusInterface::startup();

    /**
     * This must be called once during main application shutdown.
     */
    static void MobiusInterface::shutdown();

    /**
     * Register the listener.
     */
    virtual void setListener(MobiusListener* l) = 0;

    /**
     * Supply an object encapsulating the audio interface.
     * 
     * Very messy and stubbed right now, but gets us started.
     * The AudioInterface provides us with an AudioStream and we
     * register oursleives as a Listener to receive callback when
     * audio events come in from the hardware.
     * Need to completely rethink how things are wired together now
     * and weed out any references to the the old AudioInterface.
     */
    virtual void setAudioInterface(class AudioInterface* ai) = 0;
    
    /**
     * Reconfigure the Mobius engine.
     * Called during application startup after loading the initial MobiusConfig
     * and again after editing the configuration.
     *
     * Ownership is retained by the caller.
     *
     * Not everytthing in MobiusConfig is used by the engine, some is now under "UI"
     * control such as the management of the audio and MIDI devices, OSC interface,
     * Sample loading etc.
     *
     * We're evolving a layer of code that isn't really part of the UI and isn't
     * really part of the engine either since it can be implemented on top
     * of MobiusInterface.  Need to refactor MobiusConfig and remove things
     * that are no longer relevant.
     *
     * todo: the engine may want to return error messages if it doesn't like
     * something about the configuration
     *
     * I'd like to start handling Script loading and parsing above the interface
     * since it is error prone and may require involvement from the user to fix things.
     * It also impacts how Bindigns are managed which is now above the interface.
     * Once scripts are compiled, call loadScripts
     */
    virtual void configure(class MobiusConfig* config) =  0;

    /**
     * Return a state object that can be watched by the UI display engine changes.
     * The object is owned by the MobiusInterface and will
     * be deleted during shutdown()
     *
     * It is considered read-only and possibly damaging to the engine if you
     * modify it.
     */
    virtual class MobiusState* getState() = 0;

    /**
     * Do periodic housekeeping tasks within the client thread.
     * This may include checking the status of pending actions,
     * processing automatic exports, and managing communication
     * with the kernel.  It must be called at regular intervals.
     * todo: Unclear whether this should be done before or after
     * refreshing the UI, before may make the UI feel more responsive,
     * after may make the engine more responsive to the UI.  Maybe
     * we want both?
     */
    virtual void performMaintenance() = 0;
    
    /**
     * Tell the engine to do something.
     * The old engine took ownership of the Action, now ownership is retained
     * and the object is converted to an internal representation.
     *
     * Will want a more helpful error reporting mechanism here.
     */
    virtual void doAction(class UIAction* action) = 0;

    // todo: need something to check UIAction status?

    /**
     * Return the value of a Parameter as a normalized ordinal.
     */
    virtual int getParameter(class Parameter* p, int trackNumber = 0) = 0;
    
    /**
     * Install a set of Samples
     */
    virtual void installSamples(class SampleConfig* samples) = 0;

    /**
     * Run a random test
     */
    virtual void test() = 0;

    /**
     * This is only for the simulator.
     * Simulates the processing of the audio interface without being
     * actually connected to anything.
     */
    virtual void simulateInterrupt(float* input, float* output, int frames) = 0;

  private:

    // maintain a singleton for now so we can get to it easilly without
    // passing it everywhere
    static MobiusInterface* Singleton;
    
};
   

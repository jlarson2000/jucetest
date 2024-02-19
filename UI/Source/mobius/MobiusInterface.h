/*
 * Interface that wraps an implementation of the Mobius engine.
 * Keeps the UI clean and makes it easier to swap in a stub implementation
 * The old implementation has one of these too, but since it was almost entirely
 * related to things we're writing I'm not bothering with backward compatibility.
 * To the extent that it exists, rewrite engine code to either use this or not
 * depend on it.
 */

#pragma once

/**
 * Old engine has both of these so we'll probably have to rename or use namespaces.
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
     * This must be called once during main application initialization.
     */
    static void MobiusInterface::startup();

    /**
     * This must be called once during main application shutdown.
     */
    static void MobiusInterface::shutdown();

    /**
     * Called while the application runs to obtain a handle to the
     * Mobius engine.  It must not be deleted.
     */
    static class MobiusInterface* getMobius();

    /**
     * Register the listener.
     */
    virtual void setListener(MobiusListener* l) = 0;

    /**
     * Reconfigure the Mobius engine.
     * Called during application startup after loading the initial MobiusConfig
     * and again after editing the configuration.
     * Ownership is retained by the caller.
     * Not everytthing in MobiusConfig is used by the engine, some is now under "UI"
     * control such as the management of the audio and MIDI devices, OSC interface,
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
     * with the interrupt.  It must be called at regular intervals.
     * todo: Unclear whether this should be done before or after
     * refreshing the UI, before may make the UI feel more responsive,
     * after may make the engine more responsive to the UI.  Maybe
     * we want both?
     */
    virtual void performMaintenance() = 0;
    
    /**
     * Tell the engine to do something.
     * The old engine took ownership of the Action, for now we're letting the caller
     * keep it and will do any necessary copying with internal structure resolution.
     * Revisit the concept of "interning" scripts to skip internal resolution for
     * every action.
     *
     * Will want a more robust error reporting mechanism here.
     */
    virtual void doAction(class UIAction* action) = 0;

    // todo: need something to check UIAction status?

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
   

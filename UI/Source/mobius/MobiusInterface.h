/**
 * An interface that wraps an implementation of the Mobius engine.
 * This is the only way for a user interface to control the engine.
 *
 * It is a significantly reduced and modified veraion of the original
 * interface.  The names of the classes are the same so you won't be
 * able to continue using the old ones as old code is ported over.
 *
 * Too many meandering thoughts in the method comments, clean those up.
 *
 * TODO: Really need to nail down the construction, startup, initialize,
 * reconfigure timeline to accomodate VST plugin probing.
 * 
 */

#pragma once

/**
 * Normally used by the UI to receive notifications when something
 * happens within the engine that is interesting for the display.
 * Now that we have MobiusContainer, could make that bi-directional
 * and allow information to be pushed back through that rather than needing
 * another listener?
 */
class MobiusListener {
  public:

	/**
	 * A significant time boundary has passed (beat, cycle, loop)
	 * so refresh time sensitive components now rather than waiting
	 * for the next timer event to make it look more accurate.
	 */
	virtual void MobiusTimeBoundary() = 0;

    /**
     * A change was made internally that effects the dynamic configuration,
     * typically after loading scripts.  UI is expected to call
     * getDynamicConfig and respond accordingly.
     *
     * Hmm, we could pass it here but have ownership issues.
     * I want the UI to be able to own the object indefinately.
     * So either need to be clear that the object passed here
     * or returned by getDynamicConfig must be deleted, or that
     * it will become invalid when the engine is shut down.
     * More thought...
     */
    virtual void MobiusDynamicConfigChanged() = 0;
    
};

/**
 * Interfafce to make the Mobius looping engine do the things.
 */
class MobiusInterface {

  public:

    /**
     * Factory method called during application initialization to obtain
     * a handle to the Mobius engine.  This will be a singleton that
     * must not be deleted.  Call shutdown() when no longer needed.
     */
    static class MobiusInterface* getMobius(class MobiusContainer* container);

    // must have a virtual destructor to avoid a warning
    virtual ~MobiusInterface();
    
    /**
     * This must be called once during main application initialization.
     * It is imporant to keep the full startup side effects out of the
     * constructor because that can be a complex process that you
     * can't necessarily do during static initialization.  Further VST hosts
     * typically instantiate a plugin just to probe it for information without
     * actually using it, and we don't want to initialize everything
     * if it won't be used.
     *
     * TODO: Messy amguity about the differnce between startup() and configure()
     * Also, change configure to initialize() to better reflect what it does
     * and match internal code.
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
     * TODO: started by just having one configure() method that internally
     * went to both initialize() and reconfigure().  Sort out whether we should
     * have that distinction up here.
     * 
     * Reconfigure the Mobius engine.
     * Called during application startup after loading the initial MobiusConfig
     * and again after editing the configuration.
     *
     * Ownership is retained by the caller.
     *
     * TODO: the engine may want to return error messages if it doesn't like
     * something about the configuration
     */
    virtual void configure(class MobiusConfig* config) =  0;

    /**
     * Return information about dynamic configuration.  Should be called
     * every time after configure() is called or after the DynamicConfigChanged
     * listener is notified.
     *
     * Ownership of the object is passed to the caller.
     * TODO: Should this be like MobiusState and owned by the engine
     * till shutdown?
     */
    virtual class DynamicConfig* getDynamicConfig() = 0;

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
     * 
     * TODO: Unclear whether this should be done before or after
     * refreshing the UI, before may make the UI feel more responsive,
     * after may make the engine more responsive to the UI.  Maybe
     * we want both?
     */
    virtual void performMaintenance() = 0;
    
    /**
     * Tell the engine to do something.
     * Ownership of the UIAction is retained by the caller.
     *
     * TODO: Will want a more helpful error reporting mechanism here.
     * And some actions may have return codes or other return data.
     */
    virtual void doAction(class UIAction* action) = 0;

    // TODO: need something to check queued UIAction status

    /**
     * Return the value of a Parameter as a normalized ordinal.
     */
    virtual int getParameter(class UIParameter* p, int trackNumber = 0) = 0;
    
    //
    // From here down the design is less clear and evolving
    //

    /**
     * Install a set of Samples
     */
    virtual void installSamples(class SampleConfig* samples) = 0;

    //
    // Temporary testing interface
    //

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
   

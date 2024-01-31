/*
 * Interface that wraps an implementation of the Mobius engine.
 * Keeps the UI clean and makes it easier to swap in a stub implementation
 */

#pragma once

class MobiusInterface {

  public:

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

    MobiusInterface();
    virtual ~MobiusInterface();

    /**
     * Return an editable version of the full Mobius Configuration.
     * This is owned by the caller and must be deleted or passed back
     * to saveConfiguration.
     */
    virtual class MobiusConfig* editConfiguration() = 0;

    /**
     * Take a transient configuration object and apply the changes.
     * Ownership passes from the caller into Mobius and the object
     * will be deleted.
     */
    virtual void saveConfiguration(class MobiusConfig* config) = 0;
    
    /**
     * Run a random test
     */
    virtual void test() = 0;

  private:

    // maintain a singleton for now so we can get to it easilly without
    // passing it everywhere
    static MobiusInterface* Singleton;
    
};
   

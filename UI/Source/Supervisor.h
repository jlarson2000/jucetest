/*
 * A singleton object that provides services and coordinates
 * activities between the various sub-components of the Mobius application.
 * Managed by the Juce MainComponent
 *
 */

#pragma once

// for unique_ptr
#include <memory>

#include <JuceHeader.h>

class Supervisor
{
  public:

    /**
     * Constructed by MainComponent
     * Unclear how much we need to know about this.  Will find out
     * if we need the full AudioAppComponent interface here.
     * We require MainComponent to call start() after possibly
     * setting operational parameters after construction.  The
     * shutdown() method must be called when the application closes.
     */
    Supervisor(juce::Component* main);
    ~Supervisor();

    void start();
    void shutdown();

    class MobiusConfig* getMobiusConfig();
    void updateMobiusConfig();
    class UIConfig* getUIConfig();
    void updateUIConfig();

    void doAction(class UIAction*);
    
  private:

    juce::Component* mainComponent;

    // explore using std::unique_ptr here so we don't have to rely on delete
    // and could defer constructing until we need something, I'd like to
    // keep Supervisor relatively free of compile time dependencies for the caller
    // class DisplayManager* displayManager;

    std::unique_ptr<class DisplayManager> displayManager;

    // master copies of the configuration files
    std::unique_ptr<class MobiusConfig> mobiusConfig;
    std::unique_ptr<class UIConfig> uiConfig;

    juce::String findMobiusInstallationPath();

    // config file management
    char* readConfigFile(const char* name);
    void writeConfigFile(const char* name, char* xml);
    class MobiusConfig* readMobiusConfig();
    void writeMobiusConfig(class MobiusConfig* config);
    class UIConfig* readUIConfig();
    void writeUIConfig(class UIConfig* config);
    
    
};

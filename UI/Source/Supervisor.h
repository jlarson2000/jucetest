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

#include "JuceMobiusContainer.h"
#include "MainThread.h"
#include "Binderator.h"
#include "MidiManager.h"

class Supervisor
{
  public:

    static Supervisor* Instance;

    /**
     * Temporary hack for internal display components to register interest
     * in Actions.  Saves having to have all the component levels on the
     * way down pass them through.
     */
    class ActionListener {
      public:
        virtual ~ActionListener() {};
        virtual bool doAction(UIAction* action) = 0;
    };

    /**
     * Constructed by MainComponent
     * Unclear how much we need to know about this.  Will find out
     * if we need the full AudioAppComponent interface here.
     * We require MainComponent to call start() after possibly
     * setting operational parameters after construction.  The
     * shutdown() method must be called when the application closes.
     */
    Supervisor(juce::AudioAppComponent* main);
    ~Supervisor();

    void start();
    void shutdown();

    class MobiusConfig* getMobiusConfig();
    void updateMobiusConfig();
    class UIConfig* getUIConfig();
    void updateUIConfig();

    class MobiusInterface* getMobius() {
        return mobius;
    }

    class MidiManager* getMidiManager() {
        return &midiManager;
    }
    
    // propagate an action to either MobiusInterface or DisplayManager
    void doAction(class UIAction*);

    // register UI component action handlers
    void addActionListener(ActionListener* l);
    void removeActionListener(ActionListener* l);

    // only to be called by MainThread
    void advance();
    
    juce::AudioDeviceManager& getAudioDeviceManager();

    // some menu implementations
    void installSamples();

    // audio thread callbacks
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill);
    void releaseResources();

  private:

    juce::AudioAppComponent* mainComponent;

    Binderator binderator {this};

    // new pattern, assume Instance is accessible
    MidiManager midiManager;
    
    // explore using std::unique_ptr here so we don't have to rely on delete
    // and could defer constructing until we need something, I'd like to
    // keep Supervisor relatively free of compile time dependencies for the caller
    // class DisplayManager* displayManager;

    std::unique_ptr<class DisplayManager> displayManager;

    // the singleton instance is managed by MobiusInterface::shutdown
    // do not make this a unique_ptr
    class MobiusInterface* mobius;
    JuceMobiusContainer mobiusContainer {this};
    
    MainThread uiThread {this};

    juce::Array<ActionListener*> actionListeners;

    // master copies of the configuration files
    std::unique_ptr<class MobiusConfig> mobiusConfig;
    std::unique_ptr<class UIConfig> uiConfig;

    // Audio callback statistics
    int prepareToPlayCalls = 0;
    int getNextAudioBlockCalls = 0;
    int releaseResourcesCalls = 0;
    int preparedSamplesPerBlock = 0;
    double preparedSampleRate = 0.0f;
    bool audioPrepared = false;
    
    bool audioUnpreparedBlocksTraced = false;
    int audioLastSourceStartSample = 0;
    int audioLastSourceNumSamples = 0;
    int audioLastBufferChannels = 0;
    int audioLastBufferSamples = 0;
    
    void traceDeviceStatus();
    void traceFinalStatistics();
    
    juce::String findMobiusInstallationPath();

    // config file management
    char* readConfigFile(const char* name);
    void writeConfigFile(const char* name, char* xml);
    class MobiusConfig* readMobiusConfig();
    void writeMobiusConfig(class MobiusConfig* config);
    class UIConfig* readUIConfig();
    void writeUIConfig(class UIConfig* config);
    
    
};

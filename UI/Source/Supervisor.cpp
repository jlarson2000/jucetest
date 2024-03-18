/*
 * A singleton object that provides services and coordinates
 * activities between the various sub-components of the Mobius application
 *
 * Note that the main component is expected to be an AudioAppComponent which
 * it will be when using Projucer's generated main code when asking for a standalone
 * audio app.  Plugins won't have this.  This is necessary to get to the singleton
 * of AudioDeviceManager which is normally used to deal with MIDI devices.
 * Interesting to see how this plays out when we get to plugins.  You won't ever want
 * audio devices there, but I have liked to open private MIDI devices outside of the host.
 * 
 */

#include <JuceHeader.h>

//#include "util/DataModel.h"
#include "util/Trace.h"
#include "util/FileUtil.h"

#include "model/MobiusConfig.h"
#include "model/UIConfig.h"
#include "model/XmlRenderer.h"
#include "model/UIAction.h"
#include "model/MobiusState.h"
#include "model/DynamicConfig.h"

#include "ui/DisplayManager.h"

#include "mobius/MobiusInterface.h"
#include "mobius/SampleReader.h"

#include "JuceMobiusContainer.h"

#include "RootLocator.h"
#include "Supervisor.h"

// what do static pointers get filled with?
Supervisor* Supervisor::Instance = nullptr;

Supervisor::Supervisor(juce::AudioAppComponent* main)
{
    trace("Supervisor: start construction\n");
    if (Instance != nullptr)
      trace("Attempt to instantiate more than one Supervisor!\n");
    else
      Instance = this;
    
    mainComponent = main;
//    uiThread.setSupervisor(this);
    trace("Supervisor: end construction\n");

    //RootLocator::whereAmI();
    rootPath = rootLocator.getRootPath();
    trace("Root path: %s\n", rootPath.toUTF8());
}

juce::String Supervisor::getRootPath()
{
    return rootPath;
}

Supervisor::~Supervisor()
{
    trace("Supervisor: destructor\n");
    // todo: check for proper termination?
}

/**
 * Initialize the supervisor, this is where the magic begins.
 */
void Supervisor::start()
{
    // we've been using the simple non-buffering Trace tools during
    // early development, the more complicated buffered Trace is now
    // controlled up here rather than buried in Mobius
    // force the level to 2 which includes errors and generally useful
    // progress messages from Mobius
    // MainThread will later register itself as the TraceListener and
    // call FlushTrace
    TraceDebugLevel = 2;
    
    trace("Supervisor::start\n");
    traceDeviceStatus();
    
    MobiusConfig* config = getMobiusConfig();

    // todo: think more about the initialization sequence here
    // see mobius-initialization.txt
    MobiusInterface::startup();
    mobius = MobiusInterface::getMobius(&mobiusContainer);
    // this is where the bulk of the engine initialization happens
    // it will call MobiusContainer to register callbacks for
    // audio and midi streams, I dislike the difference in side
    // effects between the first time this is called and the second time
    mobius->configure(config);

    // listen for timing and config changes we didn't initiate
    mobius->setListener(this);
    
    // this hasn't been static initialized, don't remember why
    // it may have some dependencies 
    displayManager.reset(new DisplayManager(this, mainComponent));
    
    // load the initial configuration and tell everyone about it
    displayManager->configure(getUIConfig());

    // a few things in the UI are sensitive to global parameters
    // this MUST be done after UIConfig, I think only for LoopStack
    // dislike the order dependency, just have one and pass both
    displayManager->configure(config);

    // now that the display components are fleshed out, they
    // may have registered interest in the DynamicConfig
    // tell the ones that care whata we're starting with
    notifyDynamicConfigListeners();

    // let the UI refresh go
    uiThread.start();

    // initial display update
    if (mobius != nullptr) {
        MobiusState* state = mobius->getState();
        displayManager->update(state);
    }
    
    // wait till everything is initialized before pumping events
    binderator.configure(config);
    binderator.start();

    // if this is going to open devices probably need to defer
    // that so it doesn't start piping events back to us until
    // we're done initializing
    midiManager.configure(config);

    // temporary porting check
    //DataModelDump();
    trace("Supervisor::start finished\n");
}

/**
 * Shut down the supervisor, we're tired, but it's a good kind of tired.
 */
void Supervisor::shutdown()
{
    trace("Supervisor::shutdown\n");
    binderator.stop();
    midiManager.shutdown();
    
    // stop the UI thread so we don't get any lingering events
    uiThread.stop();

    // race condition here
    // MobiusInterface::getMobius was passed our JuceMobiusContainer
    // and then registered the MobiusShell as a listener for the container
    // MoibusInterface::shutdown deletes the instance but JuceMobiusContainer
    // still has a pointer to it as a listener
    // the audio thread can still be pumping events to us which we forward to the
    // container which forwards to the listener which is now gone
    // the MobiusShell destructor now removes it's listener but I think
    // we could have still "gotten into it" just before we got the call to shut down
    // so the audio thread could be using it while the UI thread is deleting it
    // I think the bug was that ~MainComponent wasn't calling shutdownAudio before
    // calling Supervisor::shutdown
    // I fixed the listener reference and changed the order so I think it's good
    // but if you see random access violations during shutdown, look here
    // don't like the control flow, MobiusShell needs to pull things from the container
    // but it doesn't really need to be a listener, we can just push things at it
    mobiusContainer.setAudioListener(nullptr);
    MobiusInterface::shutdown();
    // this is now invalid
    mobius = nullptr;
    // any cleanup in mobiusContainer?
    
    // save any UI configuration changes that were made during use
    // in practice this is only for StatusArea but might have others someday
    UIConfig* config = getUIConfig();
    bool changes = displayManager->saveConfiguration(config);
    if (changes) {
        writeUIConfig(config);
    }
        
    // DisplayManager is in a smart pointer, but should we be doing
    // things before the destructor happens, in particular the audio
    // streams will be closed before delete happens

    // mobiusConfig and uiConfig are smart pointers

    traceFinalStatistics();
}

/**
 * Trace information about the state of the AudioDeviceManager
 */
void Supervisor::traceDeviceStatus()
{
    juce::AudioDeviceManager& deviceManager = mainComponent->deviceManager;

    // duplicated in ui/config/AudioDevicesPanel
    juce::AudioDeviceManager::AudioDeviceSetup setup = deviceManager.getAudioDeviceSetup();

    trace("AudioDeviceSetup\n");
    trace("  inputDeviceName: " + setup.inputDeviceName);
    trace("  outputDeviceName: " + setup.outputDeviceName);
    trace("  sampleRate: " + juce::String(setup.sampleRate));
    trace("  bufferSize: " + juce::String(setup.bufferSize));
    const char* bstring = setup.useDefaultInputChannels ? "true" : "false";
    trace("  useDefaultInputChannels: " + juce::String(bstring));
    bstring = setup.useDefaultOutputChannels ? "true" : "false";
    trace("  useDefaultOutputChannels: " + juce::String(bstring));
    // inputChannels and outputChannels are BigIneger bit vectors
}

void Supervisor::traceFinalStatistics()
{
    trace("Supervisor ending statistics:\n");
    trace("  prepareToPlay %d\n", prepareToPlayCalls);
    trace("  getNextAudioBlock %d\n", getNextAudioBlockCalls);
    trace("  releaseResources %d\n", releaseResourcesCalls);
    if (audioPrepared) 
      trace("  Ending with audio still prepared!\n");
}

/**
 * Return the AudioDeviceManager owned by the MainComponent.
 * Used only by MidiManager
 * Need to figure out how this will work if we're a plugin and
 * won't have access to AudioAppComponent.  Most of the code should
 * be okay as long as it goes through MidiManager, may need to refactor
 * this to have different MidiManagers to hide the standalone/plugin differences.
 */
juce::AudioDeviceManager& Supervisor::getAudioDeviceManager()
{
    return mainComponent->deviceManager;
}

/**
 * Called by the MobiusThread to process events.
 * The expected cycle time is 10ms or 1/10 second.
 * To make things look real, we need to advance the simulator by an amount
 * that corresponds to the thread notification cycle.  So if the cycle is 100ms
 * or 1/10 second, then the number of audio frames that would have been cousumed
 * in that time is SamplesPerSecond divided by 10.  It doesn't really matter what
 * sample rate is as long as both sides agree, so we'll assume 41.1 which
 * is what old Mobius always used.  So the number of frames per
 * thread cycle is 4110.
 */
void Supervisor::advance()
{
    if (mobius != nullptr) {
        // tell the engine to do housekeeping before we refresh the UI
        mobius->performMaintenance();

        // tell the simulator to pretend it received some audio
        // this will set beat flags in state
        // geez, this violates the notion that we're using a simulator at all
        // but what the hell, it's okay for now
        mobius->simulateInterrupt(nullptr, nullptr, 4110);
        
        // traverse the display components telling then to reflect changes in the engine
        MobiusState* state = mobius->getState();
        displayManager->update(state);
    }
    
}

//////////////////////////////////////////////////////////////////////
//
// Configuration Files
//
// Still using our old File utiltiies, need to use Juce
//
//////////////////////////////////////////////////////////////////////

const char* MobiusConfigFile = "mobius.xml";
const char* UIConfigFile = "ui.xml";

/**
 * Locate the root of the Mobius installation directory
 * where configuration files are stored.
 */
juce::String Supervisor::findMobiusInstallationPath()
{
    // obviously need to be smarter here
    return getRootPath();
}

/**
 * Read the XML for a configuration file.
 */
char* Supervisor::readConfigFile(const char* name)
{
    juce::String rootPath = findMobiusInstallationPath();
    juce::File root (rootPath);
    juce::File file = root.getChildFile(name);
    juce::String path = file.getFullPathName();
    trace("Reading configuration file %s\n", path.toUTF8());
    char* xml = ReadFile(path.toUTF8());
    return xml;
}

/**
 * Write an XML configuration file.
 */
void Supervisor::writeConfigFile(const char* name, char* xml)
{
    juce::String rootPath = findMobiusInstallationPath();
    juce::File root (rootPath);
    juce::File file = root.getChildFile(name);
    juce::String path = file.getFullPathName();
    WriteFile(path.toUTF8(), xml);
}

/**
 * Read the MobiusConfig from the file system.
 * The returned object is owned by the caller and must be deleted.
 */
MobiusConfig* Supervisor::readMobiusConfig()
{
    MobiusConfig* config = nullptr;
    
    char* xml = readConfigFile(MobiusConfigFile);
    if (xml != nullptr) {
        XmlRenderer xr;
        config = xr.parseMobiusConfig(xml);
        // todo: capture or trace parse errors
        delete xml;
    }
    return config;
}

/**
 * Write a MobiusConfig back to the file system.
 * Ownership of the config object does not transfer.
 */
void Supervisor::writeMobiusConfig(MobiusConfig* config)
{
    if (config != nullptr) {
        XmlRenderer xr;
        char* xml = xr.render(config);
        writeConfigFile(MobiusConfigFile, xml);
        delete xml;
    }
}

/**
 * Similar read/writer for the UIConfig
 */
UIConfig* Supervisor::readUIConfig()
{
    UIConfig* config = nullptr;
    
    char* xml = readConfigFile(UIConfigFile);
    if (xml != nullptr) {
        XmlRenderer xr;
        config = xr.parseUIConfig(xml);
        // todo: capture or trace parse errors
        delete xml;
    }
    return config;
}

/**
 * Write a UIConfig back to the file system.
 * Ownership of the config object does not transfer.
 */
void Supervisor::writeUIConfig(UIConfig* config)
{
    if (config != nullptr) {
        XmlRenderer xr;
        char* xml = xr.render(config);
        writeConfigFile(UIConfigFile, xml);
        delete xml;
    }
}

/**
 * Called by components to obtain the MobiusConfig file.
 * The object remains owned by the Supervisor and must not be deleted.
 * For now we will allow it to be modified by the caller, but to save
 * it and propagate change it must call updateMobiusConfig.
 * Caller is responsible for copying it if it wants to make temporary
 * changes.
 *
 * todo: the Proper C++ Way seems to be to pass around the std::unique_pointer
 * rather than calling .get on it.  I suppose it helps drive the point
 * home about ownership, but I find it ugly.
 */
MobiusConfig* Supervisor::getMobiusConfig()
{
    // bool operator tests for nullness of the pointer
    if (!mobiusConfig) {
        // haven't loaded it yet
        MobiusConfig* neu = readMobiusConfig();
        if (neu == nullptr) {
            // bootstrap one so we don't have to keep checking
            neu = new MobiusConfig();
        }
        mobiusConfig.reset(neu);
    }
    return mobiusConfig.get();
}

/**
 * Same dance for the UIConfig
 */
UIConfig* Supervisor::getUIConfig()
{
    if (!uiConfig) {
        // haven't loaded it yet
        UIConfig* neu = readUIConfig();
        if (neu == nullptr) {
            neu = new UIConfig();
        }
        uiConfig.reset(neu);
    }
    return uiConfig.get();
}

/**
 * Save a modified MobiusConfig, and propagate changes
 * to the interested components.
 * In practice this should only be called by ConfigEditor.
 *
 * Current assumption is that the object returned by getMobiusConfig
 * has been modified.  I don't think it's worth messing with excessive
 * copying of this and ownership transfers.
 */
void Supervisor::updateMobiusConfig()
{
    if (mobiusConfig) {
        MobiusConfig* config = mobiusConfig.get();

        writeMobiusConfig(config);

        // reset this so we get a fresh one on next use
        // to reflect potential changes to Script actions
        dynamicConfig.reset(nullptr);

        displayManager->configure(config);
        if (mobius != nullptr)
          mobius->configure(config);
        binderator.configure(config);
        midiManager.configure(config);

        // do we need to do this or will we receive a listener callback from Mobius?
        // updating the MobiusConfig may have changed the ScriptConfig
        // which may have loaded Scripts, which may have changed the
        // set of ActionButtons.  Whew.
        // Since making any kind of internal changes to scripts should result
        // in a MobiusListener call, let's wait for that to happen since it may not
        // notifyDynamicConfigListeners();
    }
}

void Supervisor::updateUIConfig()
{
    if (uiConfig) {
        UIConfig* config = uiConfig.get();
        
        writeUIConfig(config);

        // UIConfig can't invalidate DynamicConfig so don't need to reset it here
        
        // DisplayManager/MainWindow are the primary consumers of this
        displayManager->configure(config);
    }
}

//////////////////////////////////////////////////////////////////////
//
// DynamicConfig
//
//////////////////////////////////////////////////////////////////////

/**
 * Called by internal components sensitive to the dynamic config.
 */
DynamicConfig* Supervisor::getDynamicConfig()
{
    if (!dynamicConfig) {
        dynamicConfig.reset(mobius->getDynamicConfig());
    }
    return dynamicConfig.get();
}

void Supervisor::addDynamicConfigListener(DynamicConfigListener* l)
{
    if (!dynamicConfigListeners.contains(l))
      dynamicConfigListeners.add(l);
}

void Supervisor::removeDynamicConfigListener(DynamicConfigListener* l)
{
    dynamicConfigListeners.removeFirstMatchingValue(l);
}

void Supervisor::notifyDynamicConfigListeners()
{
    DynamicConfig* dyn = getDynamicConfig();
    
    for (int i = 0 ; i < dynamicConfigListeners.size() ; i++) {
        DynamicConfigListener* l = dynamicConfigListeners[i];
        l->dynamicConfigChanged(dyn);
    }
}

//////////////////////////////////////////////////////////////////////
//
// Mobius Listener
//
//////////////////////////////////////////////////////////////////////

/**
 * Called when the engine advances past a subcycle, cycle, loop boundary
 * and would like the UI to be refreshed early to make it look snappy.
 */
void Supervisor::MobiusTimeBoundary()
{
}

/**
 * Called when the dynamic configuration changes internally as a side
 * effect of something.  Typically invoking the LoadScripts function.
 */
void Supervisor::MobiusDynamicConfigChanged()
{
    dynamicConfig.reset(mobius->getDynamicConfig());
    notifyDynamicConfigListeners();
}

//////////////////////////////////////////////////////////////////////
//
// Random Menu Handlers
//
//////////////////////////////////////////////////////////////////////

void Supervisor::installSamples()
{
    SampleReader sr;
    
    MobiusConfig* mconfig = getMobiusConfig();
    SampleConfig* sconfig = mconfig->getSampleConfig();
    if (sconfig != nullptr) {
        SampleConfig* loaded = sr.loadSamples(sconfig);
        if (loaded != nullptr) {
            if (mobius != nullptr) {
                mobius->installSamples(loaded);
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////
//
// Actions
//
//////////////////////////////////////////////////////////////////////

void Supervisor::addActionListener(ActionListener* l)
{
    if (!actionListeners.contains(l))
      actionListeners.add(l);
}

void Supervisor::removeActionListener(ActionListener* l)
{
    actionListeners.removeFirstMatchingValue(l);
}

/**
 * Propagate an action sent up from one of the display elements that
 * was not handled at a lower level.
 * If we get back here, the action is targeted to the mobius engine.
 * Other options could be to refresh configuration, save projects,
 * or other housekeeping tasks you would normally do from menu items.
 */
void Supervisor::doAction(UIAction* action)
{
    // resolve it if we haven't already
    // should normally be able to do this early but if we're
    // dealing with scripts it will be harder
    action->resolve();

    // Test hack, if we have any action listeners, let them intercept
    // control over the action.  Passing all of them right now,
    // need to be smarter about an ActionType of ActionUI to only
    // redirect the relevant ones
    bool handled = false;
    for (int i = 0 ; i < actionListeners.size() ; i++) {
        ActionListener* l = actionListeners[i];
        handled = l->doAction(action);
        if (handled)
          break;
    }

    if (!handled) {
        if (mobius != nullptr) {
            mobius->doAction(action);
        }
    }
}

//////////////////////////////////////////////////////////////////////
//
// Audio Thread
//
//////////////////////////////////////////////////////////////////////

// Web chatter...
//
//There are no other guarantees. Some streams will call prepareToPlay()/releaseResources()
// repeatedly if they stop and start, others will not. It is possible for
// releaseResources() to be called when prepareToPlay() has not.
// Or another way to look at it that I find a little more intuitive:
// An AudioStream can only have two states, prepared and released,
// An AudioStream must always start and end its life in the released state…
// …but getNextAudioBlock() can only be called if the AudioStream is in the prepared state.
// prepareToPlay() puts the AudioStream into the prepared state;
// releaseResources() puts it in the released state.
// prepareToPlay() and releaseResources() can be called at any time, in any order.
//
// ---
// Prepare lets you know that the audio pipeline is about to start (or you’re about to
// be included in said pipeline if you’re a plugin), this is where you can create buffers or
// threads to use on the audio thread, and, if you’re a plugin this is where you will
// find out what SampleRate and BlockSize you’re going to be working with.

    
/**
 * This function will be called when the audio device is started, or when
 * its settings (i.e. sample rate, block size, etc) are changed.
 *
 * The prepareToPlay() method is guaranteed to be called at least once on an 'unprepared'
 * source to put it into a 'prepared' state before any calls will be made to
 * getNextAudioBlock(). This callback allows the source to initialise any resources it
 * might need when playing.
 * 
 * Once playback has finished, the releaseResources() method is called to put the stream
 * back into an 'unprepared' state.
 *
 * Note that this method could be called more than once in succession without a matching
 * call to releaseResources(), so make sure your code is robust and can handle that kind
 * of situation.
 *
 * samplesPerBlockExpected
 *   the number of samples that the source will be expected to supply each time
 *   its getNextAudioBlock() method is called. This number may vary slightly, because it
 *   will be dependent on audio hardware callbacks, and these aren't guaranteed to always
 *   use a constant block size, so the source should be able to cope with small variations.
 *
 * sampleRate
 *  the sample rate that the output will be used at - this is needed by sources such
 *  as tone generators.
 */
void Supervisor::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    if (prepareToPlayCalls == 0) {
        // first time here, trace to something to understand when things start
        // happening during initialization
        trace("Supervisor::prepareToPlay first call\n");
        // wasn't set up during construction, better be by now
        traceDeviceStatus();
    }
    else if (audioPrepared) {
        // called again in an already prepared state without calling releaseResources
        // can this happen?
        trace("Supervisor::prepareToPlay already prepared\n");
        if (sampleRate != preparedSampleRate)
          trace("  sampleRate changing from %lf\n", preparedSampleRate);
        if (samplesPerBlockExpected != preparedSamplesPerBlock)
          trace("  samplesPerBlock changing from %d\n", preparedSamplesPerBlock);
    }
    else {
        trace("Supervisor::prepareToPlay\n");
    }

    prepareToPlayCalls++;
    preparedSamplesPerBlock = samplesPerBlockExpected;
    preparedSampleRate = sampleRate;
    
    trace("samplesPerBlockExpected %d sampleRate %lf\n", samplesPerBlockExpected, sampleRate);

    // if we were already prepared, and no changes were made, we could
    // suppress passing this along?
    mobiusContainer.prepareToPlay(samplesPerBlockExpected, sampleRate);
        
    audioPrepared = true;
}

/**
 * AudioSourceChannelInfo is a simple struct with attributes
 *    AudioBuffer<float>* buffer
 *    int startSample
 *      the first sample in the buffer from which the callback is expected to write data
 *    int numSamples
 *      the number of samples in the buffer which the callback is expected to fill with data
 *
 * AudioBuffer
 *    packages the buffer arrays and various sizing info
 */
void Supervisor::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    getNextAudioBlockCalls++;

    // this clears everything we are expected to write to
    // until the engine is fully functional, start with this to avoid noise
    // NO: unclear whether this just clears the output buffers or the inputs
    // too, need to read more, till then let the test scaffolding handle it
    //bufferToFill.clearActiveBufferRegion();

    // outer object has a startSample and numSamples
    // the buffer can apparently be larger than the amount we're asked to fill
    // I'm interested in whether the buffer size is variable or if it always
    // stays at preparedSamplesPerBlock, and whether startSample jumps areound or
    // statys at zero
    if (audioLastSourceStartSample != bufferToFill.startSample) {
        trace("Supervisor: audio source start sample %d\n", bufferToFill.startSample);
        audioLastSourceStartSample = bufferToFill.startSample;
    }
    if (audioLastSourceNumSamples != bufferToFill.numSamples) {
        trace("Supervisor: audio source num samples %d\n", bufferToFill.numSamples);
        audioLastSourceNumSamples = bufferToFill.numSamples;
    }

    // number of channels, I think this should match MainComponent asked for in setAudioChannels
    // currently 4 input and 2 output
    // always getting 2 here, which means that this callback can't support
    // different numbers for input and output channels?  makes sense for this interface
    // but I think you can actually configure the hardware to have different numbers internally,
    // works well enough for now, I alwaysd used 8 for both
    int channels = bufferToFill.buffer->getNumChannels();
    if (audioLastBufferChannels != channels) {
        trace("Supervisor: audio buffer channels %d\n", channels);
        audioLastBufferChannels = channels;
    }

    // number of channels of audio data that this buffer contains
    // this may not match what the source wants us to fill?
    // clearActiveBufferRegion seems to imply that
    int samples = bufferToFill.buffer->getNumSamples();
    if (audioLastBufferSamples != samples) {
        trace("Supervisor: audio buffer samples %d\n", samples);
        audioLastBufferSamples = samples;
    }
    
    // can these ever be different
    if (samples > audioLastSourceNumSamples) {
        trace("Supervisor: buffer is larger than requested\n");
        // startSample should be > 0 then because we're only
        // filling a portion of the buffer
        // doesn't really matter, Juce may want to deal with larger
        // buffers for some reason but it raises latency questions
    }
    
    // you can call setSample and various gain utility methods
    // buffer.setSample(destChannel, int destSample, valueToAdd)
    // I don't think this is required, float* getWritePointer
    // just returns the raw array

    // buffer.getReadPointer(channel, sampleIndex)
    // returns a pointer to an array of read-only samples of one channel,
    // KEY POINT: Unlike PortAudio, the samples are not interleaved into "frames"
    // containing samples for all channels.  You will have to build that
    // AudioBuffer<Type> has a variable type, you can't assume it is AudioBuffer<float>
    // and seems to be usually AudioBuffer<double> exaples use "auto" to hide the difference
    if (!audioPrepared) {
        if (!audioUnpreparedBlocksTraced) {
            // this isn't supposed to happen, right?
            trace("Supervisor::getNextAudioBlock called in an unprepared state\n");
            // prevent this from happening more than once if it's common
            // audioUnpreparedBlocksTraced = true;
        }
        // until we know if this is a state that's supposed to happen
        // prevent passing this along to Mobius
    }
    else {
        mobiusContainer.getNextAudioBlock(bufferToFill);
    }
}

void Supervisor::releaseResources()
{
    trace("Supervisor::releaseResources\n");

    mobiusContainer.releaseResources();
    
    audioPrepared = false;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

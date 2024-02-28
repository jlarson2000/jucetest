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

#include "ui/DisplayManager.h"

#include "mobius/JuceMobiusContainer.h"
#include "mobius/MobiusInterface.h"

#include "Supervisor.h"

// what do static pointers get filled with?
Supervisor* Supervisor::Instance = nullptr;

Supervisor::Supervisor(juce::AudioAppComponent* main)
{
    if (Instance != nullptr)
      trace("Attempt to instantiate more than one Supervisor!\n");
    else
      Instance = this;
    
    mainComponent = main;
//    uiThread.setSupervisor(this);
}

Supervisor::~Supervisor()
{
    // todo: check for proper termination?
}

/**
 * Initialize the supervisor, this is where the magic begins.
 */
void Supervisor::start()
{
    MobiusConfig* config = getMobiusConfig();

    MobiusInterface::startup();
    
    mobius = MobiusInterface::getMobius(&mobiusContainer);
    mobius->configure(config);

    // this hasn't been static initialized, don't remember why
    // it may have some dependencies 
    displayManager.reset(new DisplayManager(this, mainComponent));
    
    // load the initial configuration and tell everyone about it
    displayManager->configure(getUIConfig());

    // a few things in the UI are sensitive to global parameters
    displayManager->configure(config);

    // let the UI refresh go
    uiThread.start();

    // initial display update
    MobiusState* state = mobius->getState();
    displayManager->update(state);

    // wait till everything is initialized before pumping events
    binderator.configure(config);
    binderator.start();

    // if this is going to open devices probably need to defer
    // that so it doesn't start piping events back to us until
    // we're done initializing
    midiManager.configure(config);

    // temporary porting check
    //DataModelDump();
}

/**
 * Shut down the supervisor, we're tired, but it's a good kind of tired.
 */
void Supervisor::shutdown()
{
    binderator.stop();
    midiManager.shutdown();
    
    // stop the UI thread so we don't get any lingering events
    uiThread.stop();

    // stop the Mobius engine, mobius object is a smart pointer
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
    return juce::String("c:/dev/jucetest/UI/Source/");
}

/**
 * Read the XML for a configuration file.
 */
char* Supervisor::readConfigFile(const char* name)
{
    juce::String root = findMobiusInstallationPath();
    juce::String path = root + name;
    char* xml = ReadFile(path.toUTF8());
    return xml;
}

/**
 * Write an XML configuration file.
 */
void Supervisor::writeConfigFile(const char* name, char* xml)
{
    juce::String root = findMobiusInstallationPath();
    juce::String path = root + name;
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

        displayManager->configure(config);
        mobius->configure(config);
        binderator.configure(config);
        midiManager.configure(config);
    }
}

void Supervisor::updateUIConfig()
{
    if (uiConfig) {
        UIConfig* config = uiConfig.get();
        
        writeUIConfig(config);
        // DisplayManager/MainWindow are the primary consumers of this
        displayManager->configure(config);
    }
}

//////////////////////////////////////////////////////////////////////
//
// Actions
//
//////////////////////////////////////////////////////////////////////

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

    mobius->doAction(action);
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
    if (audioPrepared) {
        trace("Supervisor::prepareToPlay called again\n");
        trace("samplesPerBlockExpected %d sampleRate %lf\n", samplesPerBlockExpected, sampleRate);
    }
    else {
        trace("Supervisor::prepareToPlay called for the first time\n");
        trace("samplesPerBlockExpected %d sampleRate %lf\n", samplesPerBlockExpected, sampleRate);

        //mobius.corePrepare(samplesPerBlockExpected, sampleRate);

        mobiusContainer.prepareToPlay(samplesPerBlockExpected, sampleRate);
        
        audioPrepared = true;
    }
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
    audioBlocksReceived++;

    // this clears everything we are expected to write to
    // until the engine is fully functional, start with this to avoid noise
    // NO: unclear whether this just clears the output buffers or the inputs
    // too, need to read more, till then let the test scaffolding handle it
    //bufferToFill.clearActiveBufferRegion();

    // outer object has a startSample and numSamples
    // the buffer can apparently be larger than the amount we're asked to fill
    if (audioLastSourceStartSample != bufferToFill.startSample) {
        trace("Supervisor: audio source start sample %d\n", bufferToFill.startSample);
        audioLastSourceStartSample = bufferToFill.startSample;
    }
    if (audioLastSourceNumSamples != bufferToFill.numSamples) {
        trace("Supervisor: audio source num samples %d\n", bufferToFill.numSamples);
        audioLastSourceNumSamples = bufferToFill.numSamples;
    }

    // this has the raw float data, arranged in channels
    // interestingly, the AudioBuffer class does not seem to be in the header
    // file path generated by projucer, examples do not do this, they do:
    // auto* inBuffer = bufferToFill.buffer->getReadPointer (actualInputChannel,
    
    // AudioBuffer<float>& buffer = bufferToFill.buffer;

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

    if (!audioPrepared) {
        if (!audioUnpreparedBlocksTrace) {
            trace("Supervisor::getNextAudioBlock called in an unprepared state\n");
            audioUnpreparedBlocksTrace = true;
        }
    }
    else {
        mobiusContainer.getNextAudioBlock(bufferToFill);
    }
}

void Supervisor::releaseResources()
{
    //mobius.coreStop();
    trace("Supervisor::releaseResources\n");

    mobiusContainer.releaseResources();
    
    audioPrepared = false;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

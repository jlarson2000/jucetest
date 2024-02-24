/**
 * Singleton class that manages MIDI devices for an application.
 * Constructed, owned, and accessed through Supervisor.
 * 
 * Initially adapted from the HandlingMidiEvents tutorial.
 *
 * AudioDeviceManager Notes
 *
 * Example has a static member of this, docs say
 * "Creates a default AudioDeviceManager"
 *
 * "This class keeps tracks of a currently-selected audio device, through with which it
 * continuously streams data from an audio callback, as well as one or more midi inputs."
 * The idea is that your application will create one global instance of this object, and let
 * it take care of creating and deleting specific types of audio devices internally. So when
 * the device is changed, your callbacks will just keep running without having to worry
 * about this."
 *
 * To use an AudioDeviceManager, create one, and use initialise() to set it up.
 * Then call addAudioCallback() to register your audio callback with it, and use that to
 * process your audio data.
 *
 * The manager also acts as a handy hub for incoming midi messages, allowing a listener to
 * register for messages from either a specific midi device, or from whatever the current
 * default midi input device is. The listener then doesn't have to worry about re-registering
 * with different midi devices if they are changed or deleted.
 * 
 * And yet another neat trick is that amount of CPU time being used is measured and available
 * with the getCpuUsage() method.
 *
 * The AudioDeviceManager is a ChangeBroadcaster, and will send a change message to listeners
 * whenever one of its settings is changed.
 *
 * Okay, that partially explains the changing MIDI devices problem.  I guess it maintains
 * the listeners and will just stop calling them if it goes away.
 *
 * The MIDI example does not call initialize() so it seems necessary only if you
 * want audio, MIDI is ligher weight?
 *
 * MainComponent is an AudioAppComponent, the example isn't.
 * Yes, AudioAppComponent already has an AudioDeviceManager.  So we shouldn't make one here,
 * get it from our MainComponent
 * 
 */

#include <JuceHeader.h>

#include "util/Trace.h"
#include "model/MobiusConfig.h"
#include "ui/config/LogPanel.h"

#include "Supervisor.h"
#include "MidiManager.h"

/**
 * Technically this should be an enforced singleton since
 * we expect to be in complete control over the devices.
 */
MidiManager::MidiManager()
{
    // used to show relative arrival time of midi messages, from the tutorial
    // "Returns the number of millisecs since a fixed event (usually system startup).
    // This has the same function as getMillisecondCounter(), but returns a more accurate
    // value, using a higher-resolution timer if one is available."
    // jsl - the divide by 1000 is done to match the same calculation
    // done to MidiMessage::getTimeStamp when created by MidiInput
    // " The message's timestamp is set to a value equivalent to (Time::getMillisecondCounter() / 1000.0) to specify the time when the message arrived"
    startTime = (juce::Time::getMillisecondCounterHiRes() * 0.001);
}

MidiManager::~MidiManager()
{
    shutdown();
}

/**
 * Allow devices to be closed without destructing.
 */
void MidiManager::shutdown()
{
    juce::AudioDeviceManager& deviceManager = Supervisor::Instance->getAudioDeviceManager();

    if (!lastInputId.isEmpty()) {
        deviceManager.removeMidiInputDeviceCallback (lastInputId, this);
    }
}

void MidiManager::addListener(Listener* l)
{
    if (!listeners.contains(l))
        listeners.add(l);
}

void MidiManager::removeListener(Listener* l)
{
    listeners.removeAllInstancesOf(l);
}

void MidiManager::logMessage(juce::String msg)
{
    if (log != nullptr)
      log->add(msg);
    else
      trace("%s\n", msg.toUTF8());
}

/**
 * Load information about the input and output devices.
 * Done once at startup, I suppose we should allow this
 * to keep happening so you can plug in new devices and
 * have us respond.  Do we get any events when that happens?
 * Or is it relatively inexpensive just to keep reading them
 * ever time?
 *
 * MidiDeviceInfo has only name and identifier.  Interestingly
 * has a tie() method used by the != and == operators.  Seems to
 * be a pattern I should learn.
 *
 * This will be called from the UI thread
 */
void MidiManager::load()
{
    // this will copy/replace the last array, assuming it's data only
    // with no important side state
    inputs = juce::MidiInput::getAvailableDevices();
    outputs = juce::MidiOutput::getAvailableDevices();

    if (log != nullptr) {
        log->add("MIDI Inputs:");
        for (int i = 0 ; i < inputs.size() ; i++) {
            log->add("    " + inputs[i].name);
        }

        log->add("MIDI Outputs:");
        for (int i = 0 ; i < outputs.size() ; i++) {
            log->add("    " + outputs[i].name);
        }
    }

    // what to do about lastLoaded?
    // I suppose if they fell off the lists we should remove
    // our callback 

    loaded;
}

void MidiManager::loadIfNecessary()
{
    if (!loaded) {
        load();
        loaded = true;
    }
}

/**
 * Open the requsted input device, and close the one currently open.
 * I imagine you can have more than one of these open at a time,
 * need to support that.
 */
void MidiManager::setInput(juce::String name)
{
    juce::String newId = findInputDeviceId(name);

    if (newId.isEmpty()) {
        logMessage("Unknown MIDI input device: " + name);
    }
    else if (newId != lastInputId) {
        logMessage("Opening MIDI input device: " + name);

        juce::AudioDeviceManager& deviceManager = Supervisor::Instance->getAudioDeviceManager();
        
        // stop following the previous device
        if (!lastInputId.isEmpty()) {
            deviceManager.removeMidiInputDeviceCallback(lastInputId, this);
        }

        // auto-enable device
        // unclear under what circumstances devices become disabled
        // maybe they start that way?
        if (! deviceManager.isMidiInputDeviceEnabled(newId))
          deviceManager.setMidiInputDeviceEnabled(newId, true);

        // follow this device
        deviceManager.addMidiInputDeviceCallback (newId, this);

        lastInputId = newId;
    }
}

void MidiManager::setOutput(juce::String name)
{
    logMessage("Opening output: " + name);
}

void MidiManager::configure(MobiusConfig* config)
{
    const char* inputName = config->getMidiInput();
    if (inputName != nullptr) {
        setInput(juce::String(inputName));
    }

    const char* outputName = config->getMidiOutput();
    if (outputName != nullptr) {
        setOutput(juce::String(outputName));
    }
}

/**
 * Example does this:
 *  if (deviceManager.isMidiInputDeviceEnabled (input.identifier))
 *
 * Find out the significance of being enabled and what it means
 * for the presentation of a selection list.
 */
juce::StringArray MidiManager::getInputNames()
{
    loadIfNecessary();
    juce::StringArray names;
    // need to start using this iteration style everywhere
    for (auto input : inputs) {
        names.add(input.name);
    }

    return names;
}

juce::StringArray MidiManager::getOutputNames()
{
    loadIfNecessary();
    juce::StringArray names;
    // need to start using this iteration style everywhere
    for (auto output : outputs) {
        names.add(output.name);
    }

    return names;
}

/**
 * Given a device name, return the id
 * Wrestled with passing a reference to either inputs or outputs arrays
 * here but couldn't get auto to work, explore someday
 */
// juce::String MidiManager::findDeviceId(juce::Array<MidiDeviceInfo> devs, juce::String name)
juce::String MidiManager::findInputDeviceId(juce::String name)
{
    juce::String id;

    loadIfNecessary();

    // not sure why this from the example doesn't work
    // Error	C3531	'dev': a symbol whose type contains 'auto' must have an initializer
    // for (auto dev : inputs) {

    for (int i = 0 ; i < inputs.size() ; i++) {
        if (inputs[i].name == name) {
            id = inputs[i].identifier;
            break;
        }
    }
    return id;
}

//////////////////////////////////////////////////////////////////////
//
// MIDI Device Callbacks
//
// These methods are called from the device handler thread
// NOT the application message thread so have to be careful
//
//////////////////////////////////////////////////////////////////////

/**
 * This one is pure virtual and must be overridden
 * 
 * "A MidiInput object will call this method when a midi event arrives. It'll be called on
 * a high-priority system thread, so avoid doing anything time-consuming in here, and avoid
 * making any UI calls. You might find the MidiBuffer class helpful for queueing incoming
 * messages for use later."
 *
 * "The message's timestamp is set to a value equivalent to
 * (Time::getMillisecondCounter() / 1000.0) to specify the time when the message arrived"
 *
 */
void MidiManager::handleIncomingMidiMessage (juce::MidiInput* source,
                                             const juce::MidiMessage& message)
{
    postLogMessage(message, source->getName());
    postListenerMessage(message, source->getName());
}

/**
 * This one is optional, don't need Sysex yet but I'm curious
 *
 * "If a long sysex message is broken up into multiple packets, this callback is made
 * for each packet that arrives until the message is finished, at which point the
 * normal handleIncomingMidiMessage() callback will be made with the entire message.
 * The message passed in will contain the start of a sysex, but won't be finished
 * with the terminating 0xf7 byte."
 *
 * Interesting, why would you want to be informed of partial messages?
 */
void MidiManager::handlePartialSysexMessage(juce::MidiInput* source,
                                            const juce::uint8 *messageData,
                                            int numBytesSoFar,
                                            double timestamp)
{
    //if (log != nullptr) {
    //log->add("Partial Sysex received!");
}

/**
 * Extreme Subtlety Alert
 * 
 * This from the example and I don't understand how it works, but it's a wey
 * from the MIDI interrupt handler to put somethign on the UI message queue
 * to be handled in the main event loop.
 *
 * This was an inner class, but I don't think it matters because
 * the owner pointer (MainContentComponent) was passed in to the constructor
 * and no parent class members were referenced, i.e. it is not a closure.
 *
 * I changed the owner type from MainContentComponent to MidiManager
 * and the class name to reflect that it can only do logging.
 */
class LoggingMessageCallback : public juce::CallbackMessage
{
  public:
    LoggingMessageCallback (MidiManager* o, const juce::MidiMessage& m, const juce::String& s)
        : owner (o), message (m), source (s)
    {}

    // this appears to be what Juce will call when it handles this
    // message in the event loop
    void messageCallback() override
    {
        if (owner != nullptr) {
            // and we can call any method on the owner
            owner->logMidiMessage (message, source);
        }
    }

    // this capture the owner pointer and the arguments we want to
    // pass to the method
    // Tutorial used
    // 
    // Component::SafePointer<MidiManager> owner;
    // 
    // Holds a pointer to some type of Component, which automatically becomes null if the
    // component is deleted.
    // If you're using a component which may be deleted by another event that's outside of
    // your control, use a SafePointer instead of a normal pointer to refer to it, and you
    // can test whether it's null before using it to see if something has deleted it.
    //
    // The ComponentType template parameter must be Component, or some subclass of Component.
    // You may also want to use a WeakReference<Component> object for the same purpose.

    // in this case, MidiManager is not a Component so we couldn't use that anyway
    // but it is a singleton managed by Supervisor and will not be deleted before
    // the message queue goes away
    // I think...
    // Still need to work through destruction order between Supervisor and MainComponent
    MidiManager* owner;

    // these are copied by value so safe
    juce::MidiMessage message;
    juce::String source;
};

void MidiManager::postLogMessage (const juce::MidiMessage& message, const juce::String& source)
{
    (new LoggingMessageCallback (this, message, source))->post();
}

/**
 * Another callback that sends the message to a Listener
 * Now that we have this, reimplement logging to just use a listener
 * we don't need to know what LogPanel is
 */
class ListenerMessageCallback : public juce::CallbackMessage
{
  public:
    ListenerMessageCallback (MidiManager* o, const juce::MidiMessage& m, const juce::String& s)
        : owner (o), message (m), source (s)
    {}

    // this appears to be what Juce will call when it handles this
    // message in the event loop
    void messageCallback() override
    {
        if (owner != nullptr) {
            // and we can call any method on the owner
            owner->notifyListener(message, source);
        }
    }
    
    MidiManager* owner;
    juce::MidiMessage message;
    juce::String source;
};

void MidiManager::postListenerMessage(const juce::MidiMessage& message, const juce::String& source)
{
    // don't bother if we don't have a listener
    if (listeners.size() > 0) {
        (new ListenerMessageCallback (this, message, source))->post();
    }
}

/**
 * We're back from beyond and on the main event thread
 */
void MidiManager::notifyListener(const juce::MidiMessage& message, juce::String& source)
{
    for (int i = 0 ; i < listeners.size() ; i++) {
        listeners[i]->midiMessage(message, source);
    }
}

//////////////////////////////////////////////////////////////////////
//
// Logging
//
//////////////////////////////////////////////////////////////////////

/**
 * Scraped from the HandlingMidiEvents tutorial, useful.
 * Source was the name of the MIDI input device
 *
 * getTimeStamp value depends on where the message came from.
 * When created by a MidiInput it will be:
 * "The message's timestamp is set to a value equivalent to
 * (Time::getMillisecondCounter() / 1000.0) to specify the time when
 * the message arrived.
 *
 */
void MidiManager::logMidiMessage(const juce::MidiMessage& message, juce::String& source)
{
    if (log != nullptr) {
        // showing time relative to when we were created
        auto time = message.getTimeStamp() - startTime;
        auto hours   = ((int) (time / 3600.0)) % 24;
        auto minutes = ((int) (time / 60.0)) % 60;
        auto seconds = ((int) time) % 60;
        auto millis  = ((int) (time * 1000.0)) % 1000;

        // render as a SMPTE-ish time code?
        auto timecode = juce::String::formatted ("%02d:%02d:%02d.%03d",
                                                 hours,
                                                 minutes,
                                                 seconds,
                                                 millis);

        auto description = getMidiMessageDescription (message);

        juce::String midiMessageString (timecode + "  -  " + description + " (" + source + ")");

        log->add(midiMessageString);
    }
}

/**
 * From the tutorial
 * This was static in the example, doesn't seem required 
 */
juce::String MidiManager::getMidiMessageDescription (const juce::MidiMessage& m)
{
    if (m.isNoteOn())           return "Note on "          + juce::MidiMessage::getMidiNoteName (m.getNoteNumber(), true, true, 3);
    if (m.isNoteOff())          return "Note off "         + juce::MidiMessage::getMidiNoteName (m.getNoteNumber(), true, true, 3);
    if (m.isProgramChange())    return "Program change "   + juce::String (m.getProgramChangeNumber());
    if (m.isPitchWheel())       return "Pitch wheel "      + juce::String (m.getPitchWheelValue());
    if (m.isAftertouch())       return "After touch "      + juce::MidiMessage::getMidiNoteName (m.getNoteNumber(), true, true, 3) +  ": " + juce::String (m.getAfterTouchValue());
    if (m.isChannelPressure())  return "Channel pressure " + juce::String (m.getChannelPressureValue());
    if (m.isAllNotesOff())      return "All notes off";
    if (m.isAllSoundOff())      return "All sound off";
    if (m.isMetaEvent())        return "Meta event";

    if (m.isController())
    {
        juce::String name (juce::MidiMessage::getControllerName (m.getControllerNumber()));

        if (name.isEmpty())
          name = "[" + juce::String (m.getControllerNumber()) + "]";

        return "Controller " + name + ": " + juce::String (m.getControllerValue());
    }

    return juce::String::toHexString (m.getRawData(), m.getRawDataSize());
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

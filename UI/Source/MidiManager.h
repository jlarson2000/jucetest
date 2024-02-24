/**
 * Singleton class that manages MIDI devices for an application.
 * Initially adapted from the HandlingMidiEvents tutorial.
 *
 * This will handle opening and closing of the configured devices
 * and receive the event callbacks.
 *
 * It can be given an optional LogPanel for logging.
 *
 */

#pragma once
#include <JuceHeader.h>

class MidiManager : public juce::MidiInputCallback
{
  public:

    /**
     * Interface of an object that wants to temporarily
     * listen for messages on the UI thread.
     */
    class Listener {
      public:
        virtual void midiMessage(const juce::MidiMessage& message, juce::String& source) = 0;
    };

    MidiManager();
    ~MidiManager();

    void addListener(Listener* l);
    void removeListener(Listener* l);

    // read device configuration from MobiusConfig
    void configure(class MobiusConfig* config);
    void shutdown();
    void load();
    
    // for testing, allow direct setting of the devices
    void setInput(juce::String name);
    void setOutput(juce::String name);

    // return the list of input devices and names
    // devices have both a name and an id, not sure if names
    // are guaranteed unique which is why ids are used internally
    // if not, then we'll need to expose ids
    juce::StringArray getInputNames();
    juce::StringArray getOutputNames();

    // Logging
    
    void setLog(class LogPanel* argLog) {
        log = argLog;
    }

    void logMessage(juce::String msg);

    // Example has these as private and the inheritance as private
    // which makes sense, but I don't understand how C++ allows access
    void handleIncomingMidiMessage (juce::MidiInput* source,
                                    const juce::MidiMessage& message) override;

    void handlePartialSysexMessage(juce::MidiInput* source,
                                   const juce::uint8 *messageData,
                                   int numBytesSoFar,
                                   double timestamp) override;

    // needs to be public so it can be called from a CallbackMessage
    void logMidiMessage(const juce::MidiMessage& message, juce::String& source);

    void notifyListener(const juce::MidiMessage& message, juce::String& source);

  private:

    class juce::Array<Listener*> listeners;
    
    // see notes on this in the .cpp
    //juce::AudioDeviceManager deviceManager;
    class LogPanel* log = nullptr;
    
    juce::Array<juce::MidiDeviceInfo> inputs;
    juce::Array<juce::MidiDeviceInfo> outputs;

    bool loaded = false;
    juce::String lastInputId;
    
    // tutorial captures this on creation to show relative times
    // when logging incomming MIDI messages
    double startTime;
    
    void loadIfNecessary();
    juce::String findInputDeviceId(juce::String name);

    void postLogMessage (const juce::MidiMessage& message, const juce::String& source);
    void postListenerMessage (const juce::MidiMessage& message, const juce::String& source);

    // static in the demo but I don't think it needs to be
    static juce::String getMidiMessageDescription (const class juce::MidiMessage& m);

};
    
    

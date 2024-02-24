
#include <JuceHeader.h>

#include <string>
#include <sstream>

#include "../../Supervisor.h"
#include "../../util/Trace.h"
#include "../../util/MidiUtil.h"
#include "../common/Form.h"

#include "ConfigEditor.h"
#include "BindingPanel.h"
#include "MidiPanel.h"

MidiPanel::MidiPanel(ConfigEditor* argEditor) :
    BindingPanel(argEditor, "MIDI Bindings")
{
    setName("MidiPanel");

    // now that BindingPanel is fully constructed
    // initialize the form so it can call down to our virtuals
    initForm();
}

MidiPanel::~MidiPanel()
{
    // remove lingering listener from MidiTracker
    MidiManager* mm = Supervisor::Instance->getMidiManager();
    mm->removeListener(this);
}

/**
 * Called by ConfigEditor when we're about to be made visible.
 * Since we're not using the usual Juce component dispatching
 * for keyboard events have to add/remove our listener to the
 * global key tracker.  Don't really like this but there aren't
 * many places that need to mess with keyboard tracking and this
 * makes it easier.
 */
void MidiPanel::showing()
{
    MidiManager* mm = Supervisor::Instance->getMidiManager();
    mm->addListener(this);
}

/**
 * Called by ConfigEditor when we're about to be made invisible.
 */
void MidiPanel::hiding()
{
    MidiManager* mm = Supervisor::Instance->getMidiManager();
    mm->removeListener(this);
}

/**
 * Called by BindingPanel as it iterates over all the bindings
 * stored in a BindingConfig list.  Return true if this is for keys.
 */
bool MidiPanel::isRelevant(Binding* b)
{
    // TriggerMidi is defined for some reason
    // but I don't think that can be seen in saved bindings
    return (b->getTrigger() == TriggerNote ||
            b->getTrigger() == TriggerProgram ||
            b->getTrigger() == TriggerControl ||
            b->getTrigger() == TriggerPitch);
}

/**
 * Return the string to show in the trigger column for a binding.
 * The Binding has a key code but we want to show a nice symbolic name.
 */
juce::String MidiPanel::renderSubclassTrigger(Binding* b)
{
    juce::String text;
    Trigger* trigger = b->getTrigger();
    
    if (trigger == TriggerNote) {
        // old utility
        char buf[32];
        MidiNoteName(b->getValue(), buf);
        // fuck, really need to figure out the proper way to concatenate strings
        text += juce::String(b->getChannel());
        text += ":";
        text += buf;
        // not interested in velocity
    }
    else if (trigger == TriggerProgram) {
        text += juce::String(b->getChannel());
        text += ":";
        text += "Pgm ";
        text += juce::String(b->getValue());
    }
    else if (trigger == TriggerControl) {
        text += juce::String(b->getChannel());
        text += ":";
        text += "CC ";
        text += juce::String(b->getValue());
    }
    else if (trigger == TriggerPitch) {
        // did anyone really use this?
        text += juce::String(b->getChannel());
        text += ":";
        text += "Pitch ";
        text += juce::String(b->getValue());
    }
    return text;
}

/**
 * Overload of a BindingPanel virtual to insert our fields in between
 * scope and arguments.  Messy control flow and has constructor issues
 * with initForm.  Would be cleaner to give Form a way to insert into
 * existing Forms.
 */
void MidiPanel::addSubclassFields()
{
    messageType = new Field("Type", Field::Type::String);
    juce::StringArray typeNames;
    // could have an array of Triggers for these
    typeNames.add("Note");
    typeNames.add("Control");
    typeNames.add("Program");
    typeNames.add("Pitch");
    messageType->setAllowedValues(typeNames);
    form.add(messageType);

    // would be nice to have a Type::Integer field below
    // a certain size to automatically render as a combo
    messageChannel = new Field("Channel", Field::Type::String);
    juce::StringArray channelNames;
    for (int i = 0 ; i < 16 ; i++) {
        channelNames.add(juce::String(i+1));
    }
    messageChannel->setAllowedValues(channelNames);
    form.add(messageChannel);

    // todo: need to make field smarter about text fields that
    // can only contain digits
    messageValue = new Field("Value", Field::Type::String);
    messageValue->setWidthUnits(4);
    form.add(messageValue);
    
    capture = new Field("MIDI Capture", Field::Type::Boolean);
    form.add(capture);
}

/**
 * Refresh the form fields to show the selected binding
 */
void MidiPanel::refreshSubclassFields(class Binding* b)
{
    Trigger* trigger = b->getTrigger();
    if (trigger == TriggerNote) {
        messageType->setValue(0);
    }
    else if (trigger == TriggerControl) {
        messageType->setValue(1);
    }
    else if (trigger == TriggerProgram) {
        messageType->setValue(2);
    }
    else if (trigger == TriggerPitch) {
        messageType->setValue(3);
    }
    else {
        // shouldn't be here, go back to Note
        messageType->setValue(0);
    }

    messageChannel->setValue(b->getChannel());
    messageValue->setValue(juce::String(b->getValue()));
}

/**
 * Put the value of the form fields into the Binding
 */
void MidiPanel::captureSubclassFields(class Binding* b)
{
    int index = messageType->getIntValue();
    switch (index) {
        case 0: b->setTrigger(TriggerNote); break;
        case 1: b->setTrigger(TriggerControl); break;
        case 2: b->setTrigger(TriggerProgram); break;
        case 3: b->setTrigger(TriggerPitch); break;
    }

    b->setChannel(messageChannel->getIntValue());
    b->setValue(messageValue->getIntValue());
}

void MidiPanel::resetSubclassFields()
{
    messageType->setValue(0);
    messageChannel->setValue(0);
    messageValue->setValue(juce::String());
}

void MidiPanel::midiMessage(const juce::MidiMessage& message, juce::String& source)
{
    if (capture->getBoolValue()) {
        int value = -1;
        if (message.isNoteOn()) {
            messageType->setValue(0);
            value = message.getNoteNumber();
        }
        else if (message.isController()) {
            messageType->setValue(1);
            value = message.getControllerNumber();
        }
        else if (message.isProgramChange()) {
            messageType->setValue(2);
            value = message.getProgramChangeNumber();
        }
        else if (message.isPitchWheel()) {
            messageType->setValue(3);
            // value is a 14-bit number, and is not significant
            // since there is only one pitch wheel
            value = 0;

        }
        if (value >= 0) {
            // channels are 1 based in Juce, 0 if sysex
            // channels are 0 based in Binding
            int ch = message.getChannel();
            if (ch > 0)
              messageChannel->setValue(ch - 1);
            messageValue->setValue(value);
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/



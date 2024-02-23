
#include <JuceHeader.h>

#include <string>
#include <sstream>

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
}

/**
 * Refresh the key field to show the selected binding
 * Uses the same rendering as the table cell
 */
void MidiPanel::refreshSubclassFields(class Binding* b)
{
}

/**
 * Put the value of the key field into the Binding
 * Undo the symbolic name transformation
 */
void MidiPanel::captureSubclassFields(class Binding* b)
{
}

void MidiPanel::resetSubclassFields()
{
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/



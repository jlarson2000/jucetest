
#include <JuceHeader.h>

#include <string>
#include <sstream>

#include "../../util/Trace.h"
#include "../../util/KeyCode.h"
#include "../common/Form.h"

#include "ConfigEditor.h"
#include "BindingPanel.h"
#include "KeyboardPanel.h"

KeyboardPanel::KeyboardPanel(ConfigEditor* argEditor) :
    BindingPanel(argEditor, "Keyboard Bindings")
{
    setName("KeyboardPanel");

    // now that BindingPanel is fully constructed
    // initialize the form so it can call down to our virtuals
    initForm();

    addKeyListener(this);
}

KeyboardPanel::~KeyboardPanel()
{
}

/**
 * Called by BindingPanel as it iterates over all the bindings
 * stored in a BindingConfig list.  Return true if this is for keys.
 */
bool KeyboardPanel::isRelevant(Binding* b)
{
    return (b->getTrigger() == TriggerKey);
}

/**
 * Return the string to show in the trigger column for a binding.
 * The Binding has a key code but we want to show a nice symbolic name.
 */
juce::String KeyboardPanel::renderSubclassTrigger(Binding* b)
{
    // not sure if this old utility works with Juce modifiers
    // assume unmodified for now
    char buf[32];
    GetKeyString(b->getValue(), buf);
    return juce::String(buf);
}

/**
 * Overload of a BindingPanel virtual to insert our fields in between
 * scope and arguments.  Messy control flow and has constructor issues
 * with initForm.  Would be cleaner to give Form a way to insert into
 * existing Forms.
 */
void KeyboardPanel::addSubclassFields()
{
    key = new Field("Key", Field::Type::String);
    key->setWidthUnits(4);
    form.add(key);

    capture = new Field("Capture", Field::Type::Boolean);
    form.add(capture);
}

/**
 * Refresh the key field to show the selected binding
 * Uses the same rendering as the table cell
 */
void KeyboardPanel::refreshSubclassFields(class Binding* b)
{
    key->setValue(renderTriggerCell(b));
}

/**
 * Put the value of the key field into the Binding
 * Undo the symbolic name transformation
 */
void KeyboardPanel::captureSubclassFields(class Binding* b)
{
    juce::var value = key->getValue();
    b->setValue(GetKeyCode(value.toString().toUTF8()));
}

void KeyboardPanel::resetSubclassFields()
{
    key->setValue(juce::var());
}

bool KeyboardPanel::keyPressed(const juce::KeyPress& keypress, juce::Component* originator)
{
    trace("Here %s\n", keypress.getTextDescription().toUTF8());

    char buf[32];
    GetKeyString(keypress.getKeyCode(), buf);

    if (capture->getBoolValue())
      key->setValue(juce::var(buf));
    
    return false;
}

bool KeyboardPanel::keyStateChanged(bool isKeyDown, juce::Component* originator)
{
    return false;
 }

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


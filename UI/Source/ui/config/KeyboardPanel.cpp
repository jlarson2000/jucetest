
#include <JuceHeader.h>

#include <string>
#include <sstream>

#include "../../KeyTracker.h"
#include "../../util/Trace.h"
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
}

KeyboardPanel::~KeyboardPanel()
{
    // make sure this doesn't linger
    KeyTracker::removeListener(this);
}

/**
 * Called by ConfigEditor when we're about to be made visible.
 * Since we're not using the usual Juce component dispatching
 * for keyboard events have to add/remove our listener to the
 * global key tracker.  Don't really like this but there aren't
 * many places that need to mess with keyboard tracking and this
 * makes it easier.
 */
void KeyboardPanel::showing()
{
    KeyTracker::addListener(this);
}

/**
 * Called by ConfigEditor when we're about to be made invisible.
 */
void KeyboardPanel::hiding()
{
    KeyTracker::removeListener(this);
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
    // not currently storing modifiers in the Binding
    return KeyTracker::getKeyText(b->getValue(), 0);
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
 * Capture current editing fields into the Binding.
 * Can be called with an empty [New] binding so must
 * initialize everything so it won't be filtered later
 * in XML rendering.
 */
void KeyboardPanel::captureSubclassFields(class Binding* b)
{
    b->setTrigger(TriggerKey);
    juce::var value = key->getValue();
    // todo: modifiers
    b->setValue(KeyTracker::parseKeyText(value.toString()));
}

void KeyboardPanel::resetSubclassFields()
{
    key->setValue(juce::var());
}

bool KeyboardPanel::keyPressed(const juce::KeyPress& keypress, juce::Component* originator)
{
    trace("Here %s\n", keypress.getTextDescription().toUTF8());

    if (capture->getBoolValue())
      key->setValue(juce::var(keypress.getTextDescription()));
    
    return false;
}

bool KeyboardPanel::keyStateChanged(bool isKeyDown, juce::Component* originator)
{
    return false;
}

void KeyboardPanel::keyTrackerDown(int code, int modifiers)
{
    //trace("KeyboardPanel::keyTrackerDown %d %d\n", code, modifiers);
    if (capture->getBoolValue())
      key->setValue(juce::var(KeyTracker::getKeyText(code, modifiers)));
}

void KeyboardPanel::keyTrackerUp(int code, int modifiers)
{
    //trace("KeyboardPanel::keyTrackerUp %d %d\n", code, modifiers);
}




/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


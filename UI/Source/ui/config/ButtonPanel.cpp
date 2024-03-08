
#include <JuceHeader.h>

#include <string>
#include <sstream>

#include "../../util/Trace.h"
#include "../../model/UIConfig.h"
#include "../../model/Binding.h"

#include "ConfigEditor.h"
#include "BindingPanel.h"
#include "ButtonPanel.h"

ButtonPanel::ButtonPanel(ConfigEditor* argEditor) :
    BindingPanel(argEditor, "Button Bindings")
{
    setName("ButtonPanel");

    // we don't need a trigger column
    bindings.setNoTrigger(true);
    
    // now that BindingPanel is fully constructed
    // initialize the form so it can call down to our virtuals
    initForm();
}

ButtonPanel::~ButtonPanel()
{
}

/**
 * We have been storing Button bindings in UIConfig as UIButtons.
 * Upgrade these to be regular Bindings like the rest.
 * This is called by BindingPanel.load to add things
 * to the BindingTable.   After this it will look at the
 * Binding list from MobiusConfig but it won't find anything
 * that pass isRelvant so these will be the only things added.
 *
 * If there were ugpgraded bindings in MobiusConfig and the
 * UIConfig was not updated on save to remove the old UIButtons
 * you'll get duplicates but I don't want to mess with duplicates
 * right now, really BindingTable should be doing that.
 */
void ButtonPanel::upgradeBindings()
{
    UIConfig* uiconfig = editor->getUIConfig();
    
    std::vector<std::unique_ptr<UIButton>>* srcButtons = uiconfig->getButtons();
    for (int i = 0 ; i < srcButtons->size() ; i++) {
        UIButton* button = srcButtons->at(i).get();

        // automatically filter out old  buttons that won't work
        if (targets.isValidTarget(button->getName())) {

            // create a temporary Binding to hold the converted
            // UIButton, this will be copied by BindingTable
            Binding b;
            b.trigger = TriggerUI;
            // we could only have had Function ops
            b.action = ActionFunction;
            b.setActionName(button->getName());
            b.setArguments(button->getArguments());

            bindings.add(&b);
        }
    }
}

/**
 * Called by BindingPanel when we've clicked the save button.
 * We upgraded the UIButton list in upgradeBindings to be
 * Bindings in the table, now that we're saving the MobiusConfig
 * with the upgraded Bindings we need to remove the originals
 * from UIConfig to avoid duplicates in the future.
 */
void ButtonPanel::saveBindingUpgrades()
{
    UIConfig* uiconfig = editor->getUIConfig();
    uiconfig->clearButtons();
    editor->saveUIConfig();
}

/**
 * Called by BindingPanel as it iterates over all the bindings
 * stored in a BindingSet.  Return true if this is for keys.
 *
 * Since button bindings are stored in UIConfig as UIButton objects
 * rather than in MobiusConfig as Bindings, they will all be relevant.
 * This should probably change.
 */
bool ButtonPanel::isRelevant(Binding* b)
{
    return (b->trigger == TriggerUI);
}

/**
 * Return the string to show in the trigger column for a binding.
 * The trigger column should be suppressed for buttons so we won't get here
 */
juce::String ButtonPanel::renderSubclassTrigger(Binding* b)
{
    return juce::String();
}

/**
 * Add fields to the BindingPanel form, we have none.
 */
void ButtonPanel::addSubclassFields()
{
}

/**
 * Refresh local fields to reflect the selected binding.
 */
void ButtonPanel::refreshSubclassFields(Binding* b)
{
}

/**
 * Capture current editing fields into the Binding.
 * Can be called with an empty [New] binding so must
 * initialize everything so it won't be filtered later
 * in XML rendering.
 *
 * Button bindings do not have a value, only an operation.
 */
void ButtonPanel::captureSubclassFields(class Binding* b)
{
    b->trigger = TriggerUI;
}

void ButtonPanel::resetSubclassFields()
{
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


#include <JuceHeader.h>

#include <string>
#include <sstream>

#include "../../util/Trace.h"
#include "../../model/UIConfig.h"
#include "../common/Form.h"

#include "ConfigEditor.h"
#include "ButtonPanel.h"

/**
 * Placeholder name we use for a new UIButton created with the New command button
 * that has not been assigned a target.
 */
const char* NewButtonName = "[New]";

ButtonPanel::ButtonPanel(ConfigEditor* argEditor) :
    ConfigPanel{argEditor, "Buttons", ConfigPanelButton::Save | ConfigPanelButton::Cancel, false}
{
    setName("ButtonPanel");

    content.addAndMakeVisible(table);
    // this is usually what you want but I could imagine wanting
    // to bulk delete buttons, could also do this by auto selecting
    // the one below after delete
    table.setMultipleSelectionEnabled(false);
    table.setColumnTitles(juce::StringArray({"Target", "Arguments"}));
    table.setColumnWidth(0, 200);
    table.setColumnWidth(1, 100);
    table.setHeaderHeight(22);
    table.setRowHeight(22);
    table.addListener(this);

    content.addAndMakeVisible(targets);
    // todo: options to suppress tabs
    // don't do this yet!! defer this till load() and we're ready to be shown
    //targets.init(editor->getMobiusConfig());
    // targets.addListener(this);

    content.addAndMakeVisible(commands);
    commands.add("New");
    commands.add("Update");
    commands.add("Delete");
    commands.autoSize();
    commands.addListener(this);
    
    content.addAndMakeVisible(arguments);
    arguments.setWidthUnits(15);
    arguments.render();
    arguments.addListener(this);
    
    // we can either auto size at this point or try to
    // make all config panels a uniform size
    setSize (900, 600);
}

ButtonPanel::~ButtonPanel()
{
}



/**
 * ConfigPanel overload to load state.
 */
void ButtonPanel::load()
{
    if (!loaded) {
        // let the target panel know the names of the things it can target
        targets.configure(editor->getMobiusConfig());
        
        UIConfig* config = editor->getUIConfig();

        // copy the button vector for editing
        // todo: copy constructor would be nice
        buttons.clear();
        std::vector<std::unique_ptr<UIButton>>* srcButtons = config->getButtons();
		for (int i = 0 ; i < srcButtons->size() ; i++) {
            UIButton* button = srcButtons->at(i).get();
            if (targets.isValidTarget(button->getName())) {
                UIButton* copyOfButton = new UIButton();
                copyOfButton->setName(button->getName());
                copyOfButton->setArguments(button->getArguments());
                buttons.add(copyOfButton);
            }
            else {
                // could try to upgrade it
            }
        }
        rebuildTable();

        // force this true for testing
        changed = true;
    }
}

/**
 * Rebuild the table after addition or deletion.
 * Could be smarter about incremental changes to the cell
 * array but why bother.
 */
void ButtonPanel::rebuildTable()
{
    table.clear();
    for (int i = 0 ; i < buttons.size() ; i++) {
        UIButton* button = buttons[i];
        table.setCell(i, 0, juce::String(button->getName()));
        table.setCell(i, 1, juce::String(button->getArguments()));
    }
    table.updateContent();
}

void ButtonPanel::save()
{
    if (changed) {
        UIConfig* config = editor->getUIConfig();

        config->clearButtons();
        while (buttons.size() > 0) {
            UIButton* button = buttons.removeAndReturn(0);
            // filter out lingering "[New]" buttons that weren't assigned a target
            // should also be filtering ones with invalid names
            const char* name = button->getName();
            if (name != nullptr && strcmp(name, NewButtonName) != 0)
              config->addButton(button);
            else {
                // resolved, discard it
                delete button;
            }
        }
        editor->saveUIConfig();
        changed = false;
    }
}

void ButtonPanel::cancel()
{
    buttons.clear();
    changed = false;
}

/**
 * When a row is selected in the binding table, adjust
 * the target component to display the values of that binding.
 * Not really necessary for the target name, but the arguments
 * are important so we can modify and update them.
 */
void ButtonPanel::tableTouched(SimpleTable* argTable)
{
    // in our case we only have one table, ignore the arg
    int row = table.getSelectedRow();
    UIButton* button = buttons[row];
    const char* name = button->getName();
    if (name == nullptr || strcmp(name, NewButtonName) == 0) {
        targets.reset();
    }
    else {
        targets.showSelectedTarget(button->getName());
    }

    // I guess show args if we have them even with
    // an unspedified target name
    juce::var args = juce::var(button->getArguments());
    arguments.setValue(args);
    arguments.repaint();
}

void ButtonPanel::buttonClicked(juce::String name)
{
    // is this the best way to compare them?
    if (name == juce::String("New")) {
        UIButton* neu = new UIButton();

        // this is the way we used to do it, pulling
        // over whatever was selected in the target panel
        // but I'm not liking this, it's confusing to
        // see replicated bindings if you had previously
        // selected one and the target display moved to show it
        // and left it active
        bool captureCurrentTarget = false;
        
        if (!targets.isTargetSelected() || !captureCurrentTarget) {
            // todo: alert and ignore, or go with
            // [New] and update it later?
            neu->setName(NewButtonName);
            targets.reset();
            // arg can't be null so build a var with void value
            arguments.setValue(juce::var());
        }
        else {
            juce::String name = targets.getSelectedTarget();
            neu->setName(name.toUTF8());
            juce::var value = arguments.getValue();
            neu->setArguments(value.toString().toUTF8());
            neu->setName(NewButtonName);
        }
        
        buttons.add(neu);
        rebuildTable();
        // select it, it will be the last
        table.setSelectedRow(buttons.size() - 1);
    }
    else if (name == juce::String("Update")) {
        int row = table.getSelectedRow();
        if (row >= 0) {
            if (!targets.isTargetSelected()) {
                // todo: try an alert here
            }
            else {
                UIButton* b = buttons[row];
                juce::String name = targets.getSelectedTarget();
                b->setName(name.toUTF8());
                juce::var value = arguments.getValue();
                b->setArguments(value.toString().toUTF8());
                table.setCell(row, 0, name);
                table.setCell(row, 1, value.toString());
                table.updateContent();
            }
        }
    }
    else if (name == juce::String("Delete")) {
        int row = table.getSelectedRow();
        if (row >= 0) {
            buttons.remove(row);
            rebuildTable();
            // auto-select the one after it?
        }
    }
}

void ButtonPanel::fieldChanged(Field* field)
{
}

//////////////////////////////////////////////////////////////////////
//
// Component
//
//////////////////////////////////////////////////////////////////////

void ButtonPanel::resized()
{
    ConfigPanel::resized();
    
    juce::Rectangle<int> area = getLocalBounds();

    // leave some space at the top
    area.removeFromTop(20);
    // and on the left
    area.removeFromLeft(20);

    // let's fix the size of the table for now rather
    // adapt to our size
    table.setBounds(area.getX(), area.getY(), 500, 400);

    // commands was auto sized, just set position
    commands.setTopLeftPosition(area.getX(), table.getY() + table.getHeight() + 10);
    
    area.removeFromLeft(table.getWidth() + 50);
    targets.setBounds(area.getX(), area.getY(), 300, 400);
    // arguments will have been autosized
    arguments.setTopLeftPosition(area.getX(), targets.getY() + targets.getHeight() + 10);
}    

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


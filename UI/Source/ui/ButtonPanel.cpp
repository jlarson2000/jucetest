
#include <JuceHeader.h>

#include <string>
#include <sstream>

#include "../model/UIConfig.h"

#include "ConfigEditor.h"
#include "Form.h"

#include "ButtonPanel.h"

ButtonPanel::ButtonPanel(ConfigEditor* argEditor) :
    ConfigPanel{argEditor, "Buttons", ConfigPanelButton::Save | ConfigPanelButton::Cancel, false}
{
    setName("ButtonPanel");

    content.addAndMakeVisible(table);
    table.setColumnTitles(juce::StringArray({"Button", "Target"}));
    table.setColumnWidth(0, 200);
    table.setColumnWidth(1, 50);
    table.setHeaderHeight(22);
    table.setRowHeight(22);
    table.setListener(this);

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
        UIConfig* config = editor->getUIConfig();

        // copy the button vector for editing
        // todo: copy constructor would be nice
        buttons.clear();
        std::vector<std::unique_ptr<UIButton>>* srcButtons = config->getButtons();
		for (int i = 0 ; i < srcButtons->size() ; i++) {
            UIButton* button = srcButtons->at(i).get();
            UIButton* copyOfButton = new UIButton();
            copyOfButton->setName(button->getName());
            buttons.add(copyOfButton);
		}

        // force this true for testing
        changed = true;
    }
}

void ButtonPanel::save()
{
    if (changed) {
        UIConfig* config = editor->getUIConfig();

        config->clearButtons();
        while (buttons.size() > 0) {
            UIButton* button = buttons.removeAndReturn(0);
            config->addButton(button);
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

void ButtonPanel::tableTouched(SimpleTable* table)
{
}

//////////////////////////////////////////////////////////////////////
//
// Component
//
//////////////////////////////////////////////////////////////////////

void ButtonPanel::resized()
{
    juce::Rectangle<int> area = getLocalBounds();

    // leave some space at the top
    area.removeFromTop(20);
    // and on the left
    area.removeFromLeft(20);

    // let's fix the size of the table for now rather
    // adapt to our size
    table.setBounds(area.getX(), area.getY(), 500, 500);
    area.removeFromLeft(table.getWidth() + 50);
}    

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


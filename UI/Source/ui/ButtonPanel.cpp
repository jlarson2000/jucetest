
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
    render();
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

//////////////////////////////////////////////////////////////////////
//
// Rendering
//
//////////////////////////////////////////////////////////////////////

// getting a warning on the setColumnTitles call
// 1>C:\dev\jucetest\UI\Source\ui\ButtonPanel.cpp(80,44): warning C4239: nonstandard extension used: 'argument': conversion from 'juce::StringArray' to 'juce::StringArray &'
// 1>C:\dev\jucetest\UI\Source\ui\ButtonPanel.cpp(80,44): message : A non-const reference may only be bound to an lvalue
1>UI_App.vcxproj -> C:\dev\jucetest\UI\Builds\VisualStudio2022\x64\Debug\App\UI.exe


void ButtonPanel::render()
{
    content.addAndMakeVisible(table);
    
    table.setColumnTitles(juce::StringArray({"Button"}));
    table.setTopLeftPosition(10, 10);
    table.render();

    // we can either auto size at this point or try to
    // make all config panels a uniform size
    setSize (900, 600);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/



#include <JuceHeader.h>

#include <string>
#include <sstream>

#include "../../model/MobiusConfig.h"

#include "../common/Form.h"
#include "../JuceUtil.h"

#include "ConfigEditor.h"

#include "ScriptPanel.h"


ScriptPanel::ScriptPanel(ConfigEditor* argEditor) :
    ConfigPanel{argEditor, "Scripts", ConfigPanelButton::Save | ConfigPanelButton::Cancel, false}
{
    setName("ScriptPanel");

    content.addAndMakeVisible(table);
    
    // we can either auto size at this point or try to
    // make all config panels a uniform size
    setSize (900, 600);
}

ScriptPanel::~ScriptPanel()
{
}

/**
 * Simpler than Presets and Setups because we don't have multiple objects to deal with.
 * Load fields from the master config at the start, then commit them directly back
 * to the master config.
 */
void ScriptPanel::load()
{
    if (!loaded) {
        MobiusConfig* config = editor->getMobiusConfig();

        ScriptConfig* sconfig = config->getScriptConfig();
        if (sconfig != nullptr) {
            // this makes it's own copy
            table.setScripts(sconfig);
        }

        loaded = true;
        // force this true for testing
        changed = true;
    }
}

void ScriptPanel::save()
{
    if (changed) {
        MobiusConfig* config = editor->getMobiusConfig();

        ScriptConfig* newConfig = table.capture();
        config->setScriptConfig(newConfig);

        editor->saveMobiusConfig();

        loaded = false;
        changed = false;
    }
}

void ScriptPanel::cancel()
{
    table.clear();
    loaded = false;
    changed = false;
}

void ScriptPanel::resized()
{
    ConfigPanel::resized();
    
    juce::Rectangle<int> area = getLocalBounds();

    // leave some space at the top
    area.removeFromTop(20);
    // and on the left
    area.removeFromLeft(20);

    // let's fix the size of the table for now rather
    // adapt to our size
    int width = table.getPreferredWidth();
    int height = table.getPreferredHeight();
    table.setBounds(area.getX(), area.getY(), width, height);

    // noting eles right now
}    

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/



#include <JuceHeader.h>

#include <string>
#include <sstream>

#include "../../model/MobiusConfig.h"
#include "../../model/Parameter.h"

#include "../common/Form.h"
#include "../JuceUtil.h"

#include "ParameterField.h"
#include "ConfigEditor.h"

#include "ScriptPanel.h"


ScriptPanel::ScriptPanel(ConfigEditor* argEditor) :
    ConfigPanel{argEditor, "Scripts", ConfigPanelButton::Save | ConfigPanelButton::Cancel, false}
{
    // debugging hack
    setName("ScriptPanel");

    // place it in the content panel
    // content.addAndMakeVisible(form);
    label.setBounds(100, 100, 100, 20);
    
    content.addAndMakeVisible(&label);
    
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

        // capture ScriptConfig

        // force this true for testing
        changed = true;
    }
}

void ScriptPanel::save()
{
    if (changed) {
        MobiusConfig* config = editor->getMobiusConfig();

        // save ScriptConfig

        editor->saveMobiusConfig();
        changed = false;
    }
}

void ScriptPanel::cancel()
{
    changed = false;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


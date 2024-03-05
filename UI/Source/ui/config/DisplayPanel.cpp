
#include <JuceHeader.h>

#include <string>
#include <sstream>

#include "../../model/MobiusConfig.h"
#include "../../model/UIParameter.h"

#include "../common/Form.h"
#include "../JuceUtil.h"

#include "ParameterField.h"
#include "ConfigEditor.h"

#include "DisplayPanel.h"


DisplayPanel::DisplayPanel(ConfigEditor* argEditor) :
    ConfigPanel{argEditor, "Displays", ConfigPanelButton::Save | ConfigPanelButton::Cancel, false}
{
    // debugging hack
    setName("DisplayPanel");

    // place it in the content panel
    // content.addAndMakeVisible(form);
    label.setBounds(100, 100, 100, 20);
    
    content.addAndMakeVisible(&label);
    
    // we can either auto size at this point or try to
    // make all config panels a uniform size
    setSize (900, 600);
}

DisplayPanel::~DisplayPanel()
{
}

/**
 * Simpler than Presets and Setups because we don't have multiple objects to deal with.
 * Load fields from the master config at the start, then commit them directly back
 * to the master config.
 */
void DisplayPanel::load()
{
    if (!loaded) {
        MobiusConfig* config = editor->getMobiusConfig();

        // capture DisplayConfig

        // force this true for testing
        changed = true;
    }
}

void DisplayPanel::save()
{
    if (changed) {
        MobiusConfig* config = editor->getMobiusConfig();

        // save DisplayConfig

        editor->saveMobiusConfig();
        changed = false;
    }
}

void DisplayPanel::cancel()
{
    changed = false;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


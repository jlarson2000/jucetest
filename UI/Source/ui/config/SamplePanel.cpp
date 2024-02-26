
#include <JuceHeader.h>

#include <string>
#include <sstream>

#include "../../model/MobiusConfig.h"
#include "../../model/Parameter.h"

#include "../common/Form.h"
#include "ParameterField.h"
#include "ConfigEditor.h"

#include "SamplePanel.h"


SamplePanel::SamplePanel(ConfigEditor* argEditor) :
    ConfigPanel{argEditor, "Samples", ConfigPanelButton::Save | ConfigPanelButton::Cancel, false}
{
    // debugging hack
    setName("SamplePanel");

    // place it in the content panel
    // content.addAndMakeVisible(form);

    // we can either auto size at this point or try to
    // make all config panels a uniform size
    setSize (900, 600);
}

SamplePanel::~SamplePanel()
{
}

/**
 * Simpler than Presets and Setups because we don't have multiple objects to deal with.
 * Load fields from the master config at the start, then commit them directly back
 * to the master config.
 */
void SamplePanel::load()
{
    if (!loaded) {
        MobiusConfig* config = editor->getMobiusConfig();

        // capture SampleConfig

        // force this true for testing
        changed = true;
    }
}

void SamplePanel::save()
{
    if (changed) {
        MobiusConfig* config = editor->getMobiusConfig();

        // save SampleConfig

        editor->saveMobiusConfig();
        changed = false;
    }
}

void SamplePanel::cancel()
{
    changed = false;
}


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


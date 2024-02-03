/**
 * Class managing most configuration editing dialogs.
 * Old Mobius implemented these with popup windows, we're now doing
 * these with simple Juce components overlayed over the main window.
 * 
 * There are a number of panels focused in a particular area of the
 * configuration: global, presets, setups, bindings.  Only one of these
 * may be visible at a time.
 *
 * This wrapper allows for the possible experimentation with popup windows
 * if we ever decide to go there, isolating MainComponent from the details.
 *
 */

#include <JuceHeader.h>

#include "ConfigEditor.h"

ConfigEditor::ConfigEditor(juce::Component* argOwner)
{
    owner = argOwner;

    owner->add(&global);
    owner->add(&preset);
    owner->add(&setup);
}

ConfigEditor::~ConfigEditor()
{
    // RIAA will destruct the various panels
}

void ConfigEditor::showGlobal()
{
    show(&global);
}

void ConfigEditor::showPresets()
{
    show(&preset);
}

void ConfigEditor::showSetups()
{
    show(&setup);
}

void ConfigEditor::closeAll()
{
    // TODO: should this have the side effect of canceling the current editing session?
    show(nullptr);
}

void ConfigEditor::show(ConfigPanel* p)
{
    showOrHide(&global, p);
    showOrHide(&preset, p);
    showOrHide(&setup, p);
}

void ConfigEditor::showOrHide(ConfigPanel* p, ConfigPanel* selected)
{
    // should we handle centering here or in ConfigPanel?
    p->setVisible(p == selected);
}

//////////////////////////////////////////////////////////////////////
//
// ConfigPanel Callbacks
//
//////////////////////////////////////////////////////////////////////

/**
 * Called by the panel when it is done
 */
void ConfigEditor::close(ConfigPanel* p)
{
    p->setVisible(false);
}

/**
 * Determine the path to the MobiusConfig file.
 */
const char* ConfigEditor::getConfigFilePath()
{
    // todo: determine the best way to find this
    const char* path = "c:/dev/jucetest/UI/Source/mobius.xml";
    return path;
}

/**
 * Called by the ConfigPanel to read the MobiusConfig.
 * The master config object is managed by ConfigEditor
 * the panels are allowed to make modifications to it
 * and ask us to save it.  Each panel must not overlap
 * on the changes it makes to the MobiusConfig.
 *
 * Might be better to have the panel return us just the
 * changes and have us splice it into the master config?
 */
MobiusConfig* ConfigEditor::getMobiusConfig()
{
    if (masterConfig == nullptr) {
        const char* path = getConfigFilePath();
        char* xml = ReadFile(path);
        if (xml != nullptr) {
            XmlRenderer xr;
            masterConfig = xr.parseMobiusConfig(xml);
            // todo: display parse errors
            delete xml;
        }
    }

    return masterConfig;
}

/**
 * Called by the ConfigPanel after it has made modifications
 * to the MobiusConfig returned by getMobiusConfig.
 */
void ConfigEditor::saveMobiusConfig()
{
    if (masterConfig != nullptr) {
        const char* path = getConfigFilePath();
        XmlRenderer xr;
        char* xml = xr.render(masterConfig);
        WriteFile(path, xml);
        delete xml;
    }
}

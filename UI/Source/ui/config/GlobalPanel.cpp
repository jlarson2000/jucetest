
#include <JuceHeader.h>

#include <string>
#include <sstream>

#include "../../model/MobiusConfig.h"
#include "../../model/Parameter.h"

#include "../common/Form.h"
#include "ParameterField.h"
#include "ConfigEditor.h"

#include "GlobalPanel.h"


GlobalPanel::GlobalPanel(ConfigEditor* argEditor) :
    ConfigPanel{argEditor, "Global Parameters", ConfigPanelButton::Save | ConfigPanelButton::Cancel, false}
{
    // debugging hack
    setName("GlobalPanel");
    render();
}

GlobalPanel::~GlobalPanel()
{
}

/**
 * Simpler than Presets and Setups because we don't have multiple objects to deal with.
 * Load fields from the master config at the start, then commit them directly back
 * to the master config.
 */
void GlobalPanel::load()
{
    if (!loaded) {
        MobiusConfig* config = editor->getMobiusConfig();

        loadGlobal(config);

        // force this true for testing
        changed = true;
    }
}

void GlobalPanel::save()
{
    if (changed) {
        MobiusConfig* config = editor->getMobiusConfig();
        saveGlobal(config);
        editor->saveMobiusConfig();
        changed = false;
    }
}

void GlobalPanel::cancel()
{
    changed = false;
}

/**
 * Load the global confif into the parameter fields
 */
void GlobalPanel::loadGlobal(MobiusConfig* config)
{
    juce::Array<Field*> fields;
    form.gatherFields(fields);
    for (int i = 0 ; i < fields.size() ; i++) {
        Field* f = fields[i];
        ParameterField* pf = dynamic_cast<ParameterField*>(f);
        if (pf != nullptr)
          pf->loadValue(config);
    }
}

/**
 * Save the fields back into the master config.
 */
void GlobalPanel::saveGlobal(MobiusConfig* config)
{
    juce::Array<Field*> fields;
    form.gatherFields(fields);
    for (int i = 0 ; i < fields.size() ; i++) {
        Field* f = fields[i];
        ParameterField* pf = dynamic_cast<ParameterField*>(f);
        if (pf != nullptr)
          pf->saveValue(config);
    }
}

//////////////////////////////////////////////////////////////////////
//
// Rendering
//
//////////////////////////////////////////////////////////////////////

void GlobalPanel::render()
{
    initForm();
    form.render();

    // place it in the content panel
    content.addAndMakeVisible(form);

    // we can either auto size at this point or try to
    // make all config panels a uniform size
    setSize (900, 600);
}

/**
 * These parameters show in the old dialog but are no longer needed.
 *  Custom Message File, CustomMessageFileParameter
 *  Message Duration/MessageDurationParameter
 *   this is actually a UIType, not in MobiusConfig
 *  Dual Plugin Edit Window
 *
 * These are in MobiusConfig but are edited in dedicated panels
 *   AudioInputParameter
 *   AudioOutputParameter
 *   MidiInputParameter
 *   MidiOutputParameter
 *
 * See parameter-shit for others that are defined but obscure.
 * 
 */
void GlobalPanel::initForm()
{
    addField("Miscellaneous", QuickSaveParameter);
    addField("Miscellaneous", LongPressParameter);
    addField("Miscellaneous", SpreadRangeParameter);
    addField("Miscellaneous", NoiseFloorParameter);
    addField("Miscellaneous", IntegerWaveFileParameter);
    addField("Miscellaneous", MonitorAudioParameter);
    addField("Miscellaneous", AutoFeedbackReductionParameter);
    addField("Miscellaneous", GroupFocusLockParameter);
    addField("Miscellaneous", MidiExportParameter);
    addField("Miscellaneous", HostMidiExportParameter);

    addField("Limits", TracksParameter);
    addField("Limits", TrackGroupsParameter);
    addField("Limits", MaxLoopsParameter);
    addField("Limits", PluginPortsParameter);

    addField("Functions", FocusLockFunctionsParameter);
    addField("Functions", MuteCancelFunctionsParameter);
    addField("Functions", ConfirmationFunctionsParameter);

    addField("Modes", AltFeedbackDisableParameter);

    addField("Advanced", TracePrintLevelParameter);
    addField("Advanced", TraceDebugLevelParameter);
    addField("Advanced", MaxSyncDriftParameter);
    addField("Advanced", SaveLayersParameter);
    addField("Advanced", LogStatusParameter);

    // old dialog had some red warning text at the top
    
    addField("Advanced", OscEnableParameter);
    addField("Advanced", OscTraceParameter);
    addField("Advanced", OscInputPortParameter);
    addField("Advanced", OscOutputPortParameter);
    addField("Advanced", OscOutputHostParameter);
}

void GlobalPanel::addField(const char* tab, Parameter* p)
{
    form.add(new ParameterField(p), tab, 0);
}


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


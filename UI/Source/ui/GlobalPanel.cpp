
#include <JuceHeader.h>

#include <string>
#include <sstream>

#include "../model/MobiusConfig.h"
#include "../model/Parameter.h"

#include "ConfigEditor.h"
#include "Form.h"
#include "ParameterField.h"

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
    form.add("Miscellaneous", QuickSaveParameter);
    form.add("Miscellaneous", LongPressParameter);
    form.add("Miscellaneous", SpreadRangeParameter);
    form.add("Miscellaneous", NoiseFloorParameter);
    form.add("Miscellaneous", IntegerWaveFileParameter);
    form.add("Miscellaneous", MonitorAudioParameter);
    form.add("Miscellaneous", AutoFeedbackReductionParameter);
    form.add("Miscellaneous", GroupFocusLockParameter);
    form.add("Miscellaneous", MidiExportParameter);
    form.add("Miscellaneous", HostMidiExportParameter);

    form.add("Limits", TracksParameter);
    form.add("Limits", TrackGroupsParameter);
    form.add("Limits", MaxLoopsParameter);
    form.add("Limits", PluginPortsParameter);

    form.add("Functions", FocusLockFunctionsParameter);
    form.add("Functions", MuteCancelFunctionsParameter);
    form.add("Functions", ConfirmationFunctionsParameter);

    form.add("Modes", AltFeedbackDisableParameter);

    form.add("Advanced", TracePrintLevelParameter);
    form.add("Advanced", TraceDebugLevelParameter);
    form.add("Advanced", MaxSyncDriftParameter);
    form.add("Advanced", SaveLayersParameter);
    form.add("Advanced", LogStatusParameter);

    // old dialog had some red warning text at the top
    
    form.add("Advanced", OscEnableParameter);
    form.add("Advanced", OscTraceParameter);
    form.add("Advanced", OscInputPortParameter);
    form.add("Advanced", OscOutputPortParameter);
    form.add("Advanced", OscOutputHostParameter);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


/**
 * A form panel for editing presets.
 */

#include <JuceHeader.h>

#include "../model/MobiusConfig.h"
#include "../model/Parameter.h"
#include "Form.h"
#include "ParameterField.h"
#include "PresetPanel.h"

PresetPanel::PresetPanel(ConfigEditor* argEditor) :
    ConfigPanel{argEditor, "Presets", ConfigPanelButton::Save | ConfigPanelButton::Cancel}
{
    render();
}

PresetPanel::~PresetPanel()
{
    // presets will delete itself and the cached objects
}

void PresetPanel::show()
{
    if (!loaded) {
        // clone the Preset list into a local copy
        presets.clear();
        MobiusConfig* config = editor.getMobiusConfig();
        if (config != nullptr) {
            // convert the linked list to an OwnedArray
            Preset* plist = config->getPresets();
            while (plist != nullptr) {
                XmlRenderer xr;
                Preset* p = xr.clone(plist);
                presets.add(p);
                plist = plist->getNext();
            }
        }
        loaded = true;
    }

    selectedPreset = 0;
    loadPreset(selectedPreset);
}

void PresetPanel::save()
{
}

void PresetPanel::revert()
{
}

void PresetPanel::cancel()
{
}

bool PresetPanel::isActive()
{
    return false;
}

/**
 * Load a preset into the parameter fields
 */
void PresetPanel::loadPreset(int index)
{
    Preset* p = presets[index];
    if (p != nullptr) {
        Form::Iterator it(&form);
        while (it.hasNext()) {
            ParameterField* f = (ParameterField*)it.next();
            f->loadValue(p);
        }
    }
}

/**
 * Save one of the edited presets back to the master config
 * Think...should save/cancel apply to the entire list of presets
 * or only the one currently being edited.  I think it would be confusing
 * to keep an editing transaction over the entire list
 * When a preset is selected, it should throw away changes that
 * are in progress to the current preset.
 */
void PresetPanel::savePreset(int index)
{
    Preset* p = presets[index];
    if (p != nullptr) {
        Form::Iterator it(&form);
        while (it.hasNext()) {
            ParameterField* f = (ParameterField*)it.next();
            f->saveValue(p);
        }
    }
}

Preset* PresetPanel::getSelectedPreset()
{
    Preset* p = nullptr;
    if (presets.size() > 0) {
        if (selectedPreset < 0 || selectedPreset >= presets.size()) {
            // shouldn't happen, default back to first
            selectedPreset = 0;
        }
        p = presets[selectedPreset];
    }
    return p;
}

void PresetPanel::render()
{
    initForm();
    form.render();
}

void PresetPanel::initForm()
{
    add("General", LoopCountParameter);
    add("General", SubCycleParameter);
    add("General", MaxUndoParameter);
    add("General", MaxRedoParameter);
    add("General", NoFeedbackUndoParameter);
    add("General", AltFeedbackEnableParameter);

#if 0
    // Quantize
    add("Quantize", QuantizeModeParameter);
    add("Quantize", SwitchQuantizeParameter);
    add("Quantize", BounceQuantizeParameter);
    add("Quantize", OverdubQuantizedParameter);

    // Record

    add("Record", RecordThresholdParameter);
    add("Record", AutoRecordBarsParameter);
    add("Record", AutoRecordTempoParameter);
    add("Record", RecordSpeedChangesParameter);
    add("Record", RecordResetsFeedbackParameter);

    // Switch
    add("Switch", EmptyLoopAction);
    add("Switch", EmptyTrackActionParameter);
    add("Switch", TrackLeaveActionParameter);
    add("Switch", TimeCopyModeParameter);
    add("Switch", SoundCopyModeParameter);
    add("Switch", SwitchLocationParameter);
    add("Switch", SwitchDurationParameter);
    add("Switch", ReturnLocationParameter);
    add("Switch", SwitchVelocitySensitiveParameter);
    // column 2
    add("Switch", RecordTransferParameter, 1);
    add("Switch", OverdubTransferParameter, 1);
    add("Switch", ReverseTransferParameter, 1);
    add("Switch", SpeedTransferParameter, 1);
    add("Switch", PitchTransferParameter, 1);
        
    // Functions
    add("Functions", MultiplyModeParameter);
    add("Functions", ShuffleModeParameter);
    add("Functions", MuteModeParameter);
    add("Functions", MuteCancelParameter);
    add("Functions", SlipModeParameter);
    add("Functions", SlipTimeParameter);
    add("Functions", WindowSlideUnitParameter);
    add("Functions", WindowSlideAmountParameter);
    add("Functions", WindowEdgeAmountParameter);
    // column 2
    add("Functions", OverdubWhileRoundingParameter, 1);
        
    // Effects
    add("Effects", SpeedSequenceParameter);
    add("Effects", PitchSequenceParameter);
    add("Effects", SpeedShiftRestartParameter);
    add("Effects", PitchShiftRestartParameter);
    add("Effects", SpeedStepRangeParameter);
    add("Effects", SpeedBendRangeParameter);
    add("Effects", PitchStepRangeParameter);
    add("Effects", PitchBendRangeParameter);
    add("Effects", TimeStretchRangeParameter);

    // Sustain
    add("Sustain", SustainFunctionsParameter);
#endif
}

void PresetPanel::add(const char* tab, Parameter* p, int column)
{
    form.add(new ParameterField(p), tab, column);
}

            

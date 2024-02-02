/**
 * A form panel for editing presets.
 */

#include <JuceHeader.h>

#include "../model/MobiusConfig.h"
#include "../model/Parameter.h"
#include "Form.h"
#include "ParameterField.h"
#include "PresetPanel.h"

PresetPanel::PresetPanel() : ConfigPanel{"Presets", ConfigPanelButton::Save | ConfigPanelButton::Cancel}
{
    render();
}

PresetPanel::~PresetPanel()
{
    // presets will delete itself and the cached objects
}

void PresetPanel::load()
{
    if (!loaded) {
        presets.clear();
        MobiusConfig* config = readMobiusConfig();
        if (config != nullptr) {
            // convert the linked list to an OwnedArray
            Preset* plist = config->getPresets();
            Preset* next = nullptr;
            while (plist != nullptr) {
                presets.add(plist);
                next = plist->getNext();
                // since these are no longer linked, null this out so
                // we can delete it without cascading
                plist->setNext(nullptr);
            }
        }
        // take it from the MobiusConfig
        config->setPresets(nullptr);
        delete config;
        loaded = true;
    }

    selectedPreset = 0;
    loadPreset(selectedPreset);
}

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

            

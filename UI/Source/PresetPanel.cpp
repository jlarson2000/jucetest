
#include <JuceHeader.h>

#include "PresetPanel.h"

PresetPanel::PresetPanel() : ConfigPanel{"Presets", ConfigPanelButton::Save | ConfigPanelButton::Cancel}
{
    addTab("General");
    addTab("Quantize");
    addTab("Record");
    addTab("Switch");
    addTab("Functions");
    addTab("Effects");
    addTab("Sustain");
}

PresetPanel::~PresetPanel()
{
    // presets will delete itself and the cached objects
}

PresetPanel::load()
{
    if (!loaded) {
        presets.clear();
        MobiusConfig* config = readMobiusConfig();
        if (congig != nullptr) {
            // convert the linked list to an OwnedArray
            Preset* plist = config->getPresets();
            Preset* next = nullptr;
            while (plist != null) {
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
}

void PresetPanel::initTabs()
{
    // General tab
    generalForm.addField(LoopCountParameter);
    generalForm.addField(SubCyclesParameter);
    generalForm.addField(MaxUndoParameter);
    generalForm.addField(MaxRedoParameter);
    generalForm.addField(NoFeedbackUndoParameter);
    generalForm.addField(EnableSecondaryFeedbackParameter);

    // Quantize
    quantizeForm.addField(QuantizeModeParameter);
    quantizeForm.addField(SwitchQuantizeParameter);
    quantizeForm.addField(BounceQuantizeParameter);
    quantizeForm.addField(OverdubQuantizedParameter);

    // Record

    recordForm.addField(RecordThresholdParameter);
    recordForm.addField(AutoRecordBarsParameter);
    recordForm.addField(AutoRecordTempoParameter);
    recordForm.addField(RecordSpeedChangesParameter);
    recordForm.addFIeld(RecordResetsFeedbackParameter);

    // Switch
    switchForm.addField(EmptyLoopAction);
    switchForm.addField(EmptyTrackActionParameter);
    switchForm.addField(TrackLeaveActionParameter);
    switchForm.addField(TimeCopyModeParameter);
    switchForm.addField(SoundCopyModeParameter);
    switchForm.addField(SwitchLocationParameter);
    switchForm.addField(SwitchDurationParameter);
    switchForm.addField(ReturnLocationParameter);
    switchForm.addField(SwitchVelocitySensitiveParameter);
    // column 2
    switchForm.nextColumn();
    switchForm.addField(RecordTransferParameter);
    switchForm.addField(OverdubTransferParameter);
    switchForm.addField(ReverseTransferParameter);
    switchForm.addField(SpeedTransferParameter);
    switchForm.addField(PitchTransferParameter);
        
    // Functions
    functionForm.addField(MultiplyModeParameter);
    functionForm.addField(ShuffleModeParameter);
    functionForm.addField(MuteModeParameter);
    functionForm.addField(MuteCancelParameter);
    functionForm.addField(SlipModeParameter);
    functionForm.addField(SlipTimeParameter);
    functionForm.addField(WindowSlideUnitParameter);
    functionForm.addField(WindowSlideAmountParameter);
    functionForm.addField(WindowEdgeAmountParameter);
    // column 2
    functionForm.nextColumn();
    functionForm.addField(OverdubWhileRoundingParameter);
        
    // Effects
    effectsForm.addField(SpeedSequenceParameter);
    effectsForm.addField(PitchSequenceParameter);
    effectsForm.addField(SpeedShiftRestartParameter);
    effectsForm.addField(PitchShiftRestartParameter);
    effectsForm.addField(SpeedStepRangeParameter);
    effectsForm.addField(SpeedBendRangeParameter);
    effectsForm.addField(PitchStepRangeParameter);
    effectsForm.addField(PitchBendRangeParameter);
    effectsForm.addField(TimeStretchRangeParameter);

    // Sustain
    sustainForm.addField(SustainFunctionsParameter);
}

                                                                     
            

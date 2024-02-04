/**
 * A form panel for editing presets.
 */

#include <JuceHeader.h>

#include "../model/MobiusConfig.h"
#include "../model/Parameter.h"
#include "../model/XmlRenderer.h"

#include "Form.h"
#include "ParameterField.h"
#include "ConfigEditor.h"
#include "PresetPanel.h"
#include "JuceUtil.h"

PresetPanel::PresetPanel(ConfigEditor* argEditor) :
    ConfigPanel{argEditor, "Presets", ConfigPanelButton::Save | ConfigPanelButton::Cancel, true}
{
    // debugging hack
    setName("PresetPanel");
    render();
}

PresetPanel::~PresetPanel()
{
    // members will delete themselves
}

//////////////////////////////////////////////////////////////////////
//
// ConfigPanel overloads
//
//////////////////////////////////////////////////////////////////////

/**
 * Called by ConfigEditor when asked to edit presets.
 * 
 * If this is the first time being shown, copy the Preset list from the
 * master configuration and load the first preset.
 *
 * If we are already active, just display what we currently have.
 * ConfigEditor will handle visibility.
 */
void PresetPanel::load()
{
    if (!loaded) {
        // build a list of names for the object selector
        juce::Array<juce::String> names;
        // clone the Preset list into a local copy
        presets.clear();
        MobiusConfig* config = editor->getMobiusConfig();
        if (config != nullptr) {
            // convert the linked list to an OwnedArray
            Preset* plist = config->getPresets();
            while (plist != nullptr) {
                // we shouldn't need to use an XML transform,
                // just make Preset copyable
                XmlRenderer xr;
                Preset* p = xr.clone(plist);
                // it shouldn't have one but make sure it doesn't have
                // a lingering next pointer
                p->setNext(NULL);
                presets.add(p);
                // note this needs to be in a local to prevent a leak
                juce::String js(plist->getName());
                names.add(js);
                plist = plist->getNext();
            }
        }
        
        // this will also auto-select the first one
        objectSelector.setObjectNames(names);

        loadPreset(selectedPreset);

        loaded = true;
        changed = false;
    }
}

/**
 * Called by the Save button in the footer.
 * 
 * Save all presets that have been edited during this session
 * back to the master configuration.
 *
 * Tell the ConfigEditor we are done.
 */
void PresetPanel::save()
{
    if (changed) {
        // build a new Preset linked list
        Preset* plist = nullptr;
        Preset* last = nullptr;
        
        for (int i = 0 ; i < presets.size() ; i++) {
            Preset* p = presets[i];
            if (last == nullptr)
              plist = p;
            else
              last->setNext(p);
            last = p;
        }

        // we took ownership of the objects so
        // clear the owned array but don't delete them
        presets.clear(false);

        MobiusConfig* config = editor->getMobiusConfig();
        // this will also delete the current preset list
        config->setPresets(plist);

        loaded = false;
        changed = false;
    }
    else if (loaded) {
        // throw away preset copies
        presets.clear();
        loaded = false;
    }
}

/**
 * Throw away all editing state.
 */
void PresetPanel::cancel()
{
    // delete the copied presets
    presets.clear();
    loaded = false;
    changed = false;
}

//////////////////////////////////////////////////////////////////////
//
// ObjectSelector overloads
// 
//////////////////////////////////////////////////////////////////////

void PresetPanel::selectObject(int ordinal)
{
}

void PresetPanel::newObject()
{
}

void PresetPanel:: deleteObject()
{
}

void PresetPanel::revertObject()
{
}

void PresetPanel::renameObject(juce::String newName)
{
}

//////////////////////////////////////////////////////////////////////
//
// Internal Methods
// 
//////////////////////////////////////////////////////////////////////

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

    // at this point the combobox is in sync
    selectedPreset = 0;
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

//////////////////////////////////////////////////////////////////////
//
// Form Rendering
//
//////////////////////////////////////////////////////////////////////

void PresetPanel::render()
{
    initForm();
    form.render();

    // place it in the content panel
    content.addAndMakeVisible(form);

    // at this point the component hierarhcy has been fully constructed
    // but not sized, until we support bottom up sizing start with
    // a fixed size, this will cascade resized() down the child hierarchy
    setSize (500, 500);
}

void PresetPanel::initForm()
{
    add("General", LoopCountParameter);
    add("General", SubCycleParameter);
    add("General", MaxUndoParameter);
    add("General", MaxRedoParameter);
    add("General", NoFeedbackUndoParameter);
    add("General", AltFeedbackEnableParameter);

    add("Quantize", QuantizeParameter);
    add("Quantize", SwitchQuantizeParameter);
    add("Quantize", BounceQuantizeParameter);
    add("Quantize", OverdubQuantizedParameter);

#if 0
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

            
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

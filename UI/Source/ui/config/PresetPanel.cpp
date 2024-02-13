/**
 * A form panel for editing presets.
 */

#include <JuceHeader.h>

#include "../../model/MobiusConfig.h"
#include "../../model/Parameter.h"
#include "../../model/XmlRenderer.h"

#include "../common/Form.h"
#include "../JuceUtil.h"

#include "ParameterField.h"
#include "ConfigEditor.h"
#include "PresetPanel.h"

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
        // force this true for testing
        changed = true;
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
        // copy visible state back into the Preset
        // need to also do this when the selected preset is changed
        savePreset(selectedPreset);
        
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
        editor->saveMobiusConfig();

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
        juce::Array<Field*> fields;
        form.gatherFields(fields);
        for (int i = 0 ; i < fields.size() ; i++) {
          Field* f = fields[i];
          ParameterField* pf = dynamic_cast<ParameterField*>(f);
          if (pf != nullptr)
            pf->loadValue(p);
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
        juce::Array<Field*> fields;
        form.gatherFields(fields);
        for (int i = 0 ; i < fields.size() ; i++) {
          Field* f = fields[i];
          ParameterField* pf = dynamic_cast<ParameterField*>(f);
          if (pf != nullptr)
            pf->saveValue(p);
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

    // until we get auto-sizing worked out, make this plenty wide
    // MainComponent is currently 1000x1000
    setSize (900, 600);
}

void PresetPanel::initForm()
{
    addField("General", LoopCountParameter);
    addField("General", SubCycleParameter);
    addField("General", MaxUndoParameter);
    addField("General", MaxRedoParameter);
    addField("General", NoFeedbackUndoParameter);
    addField("General", AltFeedbackEnableParameter);

    addField("Quantize", QuantizeParameter);
    addField("Quantize", SwitchQuantizeParameter);
    addField("Quantize", BounceQuantizeParameter);
    addField("Quantize", OverdubQuantizedParameter);

    // Record

    addField("Record", RecordThresholdParameter);
    addField("Record", AutoRecordBarsParameter);
    addField("Record", AutoRecordTempoParameter);
    addField("Record", SpeedRecordParameter);
    addField("Record", RecordResetsFeedbackParameter);

    // Switch
    addField("Switch", EmptyLoopActionParameter);
    addField("Switch", EmptyTrackActionParameter);
    addField("Switch", TrackLeaveActionParameter);
    addField("Switch", TimeCopyParameter);
    addField("Switch", SoundCopyParameter);
    addField("Switch", SwitchLocationParameter);
    addField("Switch", SwitchDurationParameter);
    addField("Switch", ReturnLocationParameter);
    addField("Switch", SwitchVelocityParameter);
    // column 2
    addField("Switch", RecordTransferParameter, 1);
    addField("Switch", OverdubTransferParameter, 1);
    addField("Switch", ReverseTransferParameter, 1);
    addField("Switch", SpeedTransferParameter, 1);
    addField("Switch", PitchTransferParameter, 1);
        
    // Functions
    addField("Functions", MultiplyModeParameter);
    addField("Functions", ShuffleModeParameter);
    addField("Functions", MuteModeParameter);
    addField("Functions", MuteCancelParameter);
    addField("Functions", SlipModeParameter);
    addField("Functions", SlipTimeParameter);
    addField("Functions", WindowSlideUnitParameter);
    addField("Functions", WindowSlideAmountParameter);
    addField("Functions", WindowEdgeUnitParameter);
    addField("Functions", WindowEdgeAmountParameter);
    // column 2
    addField("Functions", RoundingOverdubParameter, 1);
        
    // Effects
    addField("Effects", SpeedSequenceParameter);
    addField("Effects", PitchSequenceParameter);
    addField("Effects", SpeedShiftRestartParameter);
    addField("Effects", PitchShiftRestartParameter);
    addField("Effects", SpeedStepRangeParameter);
    addField("Effects", SpeedBendRangeParameter);
    addField("Effects", PitchStepRangeParameter);
    addField("Effects", PitchBendRangeParameter);
    addField("Effects", TimeStretchRangeParameter);

    // Sustain
    // this was never used, keep it out until we need it
    // I see Sustainable in various places around Action
    // handling, maybe we replaced it with that?
    //form.add("Sustain", SustainFunctionsParameter);
}

void PresetPanel::addField(const char* tab, Parameter* p, int col)
{
    form.add(new ParameterField(p), tab, col);
}


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

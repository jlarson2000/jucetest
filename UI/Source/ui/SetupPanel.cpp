
#include <JuceHeader.h>

#include <string>
#include <sstream>

#include "../model/Parameter.h"
#include "../model/MobiusConfig.h"
#include "../model/Setup.h"
#include "../model/XmlRenderer.h"

#include "ConfigEditor.h"
#include "ParameterField.h"

#include "SetupPanel.h"

SetupPanel::SetupPanel(ConfigEditor* argEditor) :
    ConfigPanel{argEditor, "Track Setups", ConfigPanelButton::Save | ConfigPanelButton::Cancel, true}
{
    setName("SetupPanel");
    render();
}

SetupPanel::~SetupPanel()
{
}

//////////////////////////////////////////////////////////////////////
//
// ConfigPanel overloads
//
//////////////////////////////////////////////////////////////////////

/**
 * Called by ConfigEditor when asked to edit Setups.
 */
void SetupPanel::load()
{
    if (!loaded) {
        // build a list of names for the object selector
        juce::Array<juce::String> names;
        // clone the Setup list into a local copy
        setups.clear();
        MobiusConfig* config = editor->getMobiusConfig();
        if (config != nullptr) {
            // convert the linked list to an OwnedArray
            Setup* plist = config->getSetups();
            while (plist != nullptr) {
                // we shouldn't need to use an XML transform,
                // just make these copyable
                XmlRenderer xr;
                Setup* s = xr.clone(plist);
                // it shouldn't have one but make sure it doesn't have
                // a lingering next pointer
                s->setNext(NULL);
                setups.add(s);
                // note this needs to be in a local to prevent a leak
                juce::String js(plist->getName());
                names.add(js);
                plist = plist->getNext();
            }
        }
        
        // this will also auto-select the first one
        objectSelector.setObjectNames(names);

        loadSetupFields();

        loaded = true;
        // force this true for testing
        changed = true;
    }
}

/**
 * Called by the Save button in the footer.
 * 
 * Save all setups that have been edited during this session
 * back to the master configuration.
 *
 * Tell the ConfigEditor we are done.
 */
void SetupPanel::save()
{
    if (changed) {
        // copy visible state back into the object
        saveSetupFields();
        
        // build a new linked list
        Setup* plist = nullptr;
        Setup* last = nullptr;
        
        for (int i = 0 ; i < setups.size() ; i++) {
            Setup* s = setups[i];
            if (last == nullptr)
              plist = s;
            else
              last->setNext(s);
            last = s;
        }

        // we took ownership of the objects so
        // clear the owned array but don't delete them
        setups.clear(false);

        MobiusConfig* config = editor->getMobiusConfig();
        // this will also delete the current preset list
        config->setSetups(plist);

        loaded = false;
        changed = false;
    }
    else if (loaded) {
        // throw away preset copies
        setups.clear();
        loaded = false;
    }
}

/**
 * Throw away all editing state.
 */
void SetupPanel::cancel()
{
    // delete the copied setups
    setups.clear();
    loaded = false;
    changed = false;
}

//////////////////////////////////////////////////////////////////////
//
// ObjectSelector overloads
// 
//////////////////////////////////////////////////////////////////////

void SetupPanel::selectObject(int ordinal)
{
}

void SetupPanel::newObject()
{
}

void SetupPanel:: deleteObject()
{
}

void SetupPanel::revertObject()
{
}

void SetupPanel::renameObject(juce::String newName)
{
}

//////////////////////////////////////////////////////////////////////
//
// Internal Methods
// 
//////////////////////////////////////////////////////////////////////

/**
 * Load a setup into the parameter fields
 * This is more complicated than Preset because we're dealing
 * with two objects, the outer Setup that has fields common
 * to all tracks and an inner SetupTrack that has track-specific fields
 * We can tell the difference by looking at the parameter scope.
 */
void SetupPanel::loadSetupFields()
{
    Setup* s = setups[selectedSetup];
    if (s != nullptr) {
        juce::Array<Field*> fields;
        form.gatherFields(fields);
        for (int i = 0 ; i < fields.size() ; i++) {
          Field* f = fields[i];
          ParameterField* pf = dynamic_cast<ParameterField*>(f);
          if (pf != nullptr) {
              Parameter* p = pf->getParameter();
              if (p->scope == PARAM_SCOPE_SETUP) {
                  pf->loadValue(s);
              }
              else if (p->scope == PARAM_SCOPE_TRACK) {
                  SetupTrack* st = s->getTrack(selectedTrack);
                  pf->loadValue(st);
              }
          }
        }
    }
}

/**
 * Save the current state of the editing fields back to the Setup.
 * There are two objects involved here, the outer setup and the
 * inner SetupTrack.
 *
 * This must be called when either the Setup is changed by the
 * object selector, or when the visible track is changed with
 * the track radio button.
 */
void SetupPanel::saveSetupFields()
{
    Setup* s = setups[selectedSetup];
    if (s != nullptr) {
        juce::Array<Field*> fields;
        form.gatherFields(fields);
        for (int i = 0 ; i < fields.size() ; i++) {
          Field* f = fields[i];
          ParameterField* pf = dynamic_cast<ParameterField*>(f);
          if (pf != nullptr) {
              Parameter* p = pf->getParameter();
              if (p->scope == PARAM_SCOPE_SETUP) {
                  pf->saveValue(s);
              }
              else if (p->scope == PARAM_SCOPE_TRACK) {
                  SetupTrack* st = s->getTrack(selectedTrack);
                  pf->saveValue(st);
              }
          }
        }
    }
}

//////////////////////////////////////////////////////////////////////
//
// Form Rendering
//
//////////////////////////////////////////////////////////////////////

void SetupPanel::render()
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

void SetupPanel::initForm()
{
    // old mobius had a set of radios at the top of the
    // Tracks tab to select the target track and a set
    // of buttons at the bottom for initialize and capture
    
    form.add("Tracks", TrackNameParameter);
    form.add("Tracks", DefaultSyncSourceParameter);    // overrides default SyncSourceParameter
    form.add("Tracks", TrackSyncUnitParameter);
    form.add("Tracks", TrackPresetParameter);
    form.add("Tracks", GroupParameter);
    form.add("Tracks", FocusParameter);
    form.add("Tracks", InputLevelParameter);
    form.add("Tracks", OutputLevelParameter);
    form.add("Tracks", FeedbackLevelParameter);
    form.add("Tracks", AltFeedbackLevelParameter);
    form.add("Tracks", PanParameter);
    form.add("Tracks", MonoParameter);

    // these were arranged on a 4x4 sub-grid
    form.add("Tracks", AudioInputPortParameter);
    form.add("Tracks", AudioOutputPortParameter);
    form.add("Tracks", PluginInputPortParameter);
    form.add("Tracks", PluginOutputPortParameter);

    form.add("Synchronization", SyncSourceParameter);   // should be DefaultSyncSourceParameter
    form.add("Synchronization", DefaultTrackSyncUnitParameter);
    form.add("Synchronization", SlaveSyncUnitParameter);
    form.add("Synchronization", BeatsPerBarParameter);
    form.add("Synchronization", RealignTimeParameter);
    form.add("Synchronization", OutRealignModeParameter);
    form.add("Synchronization", MuteSyncModeParameter);
    form.add("Synchronization", ResizeSyncAdjustParameter);
    form.add("Synchronization", SpeedSyncAdjustParameter);
    form.add("Synchronization", MinTempoParameter);
    form.add("Synchronization", MaxTempoParameter);
    form.add("Synchronization", ManualStartParameter);

    // Other

    form.add("Other", InitialTrackParameter);

    // this one has special values
    form.add(buildResetablesField(), "Other");

    // Binding Overlay
}

Field* SetupPanel::buildResetablesField()
{
    Field* field = new ParameterField(ResetablesParameter);
    juce::StringArray values;
    juce::StringArray displayValues;
    
    // values are defined by Parameter flags
	for (int i = 0 ; i < Parameter::Parameters.size() ; i++) {
        Parameter* p = Parameter::Parameters[i];
        if (p->resettable) {
            values.add(p->getName());
            displayValues.add(p->getDisplayName());
        }
    }

    // Mobius sorted the displayName list, should do the same!

    field->setAllowedValues(values);
    field->setAllowedValueLabels(displayValues);

    return field;
}

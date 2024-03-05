
#include <JuceHeader.h>

#include <string>
#include <sstream>

#include "../../util/Trace.h"
#include "../../model/UIParameter.h"
#include "../../model/MobiusConfig.h"
#include "../../model/Setup.h"
#include "../../model/XmlRenderer.h"

#include "../common/SimpleRadio.h"

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
                Setup* s = new Setup(plist);
                setups.add(s);
                names.add(juce::String(plist->getName()));
                plist = (Setup*)(plist->getNext());
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
        editor->saveMobiusConfig();

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
              UIParameter* p = pf->getParameter();
              if (p->scope == ScopeSetup) {
                  pf->loadValue(s);
              }
              else if (p->scope == ScopeTrack) {
                  SetupTrack* st = s->getTrack(selectedTrack);
                  pf->loadValue(st);
              }
          }
        }
    }
    // initial selection will be the first, any need to change it?
    trackSelector->setSelection(2);
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
              UIParameter* p = pf->getParameter();
              if (p->scope == ScopeSetup) {
                  pf->saveValue(s);
              }
              else if (p->scope == ScopeTrack) {
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

    // Track panel is special
    trackSelector = new SimpleRadio();
    int ntracks = 8; // TODO: need to get this from config
    juce::StringArray trackNumbers;
    for (int i = 0 ; i < ntracks ; i++) {
        trackNumbers.add(juce::String(i+1));
    }
    trackSelector->setButtonLabels(trackNumbers);
    trackSelector->setLabel("Track");
    trackSelector->setSelection(0);
    trackSelector->setListener(this);
    trackSelector->render();

    // this will take ownership of the Component
    FormPanel* formPanel = form.getPanel("Tracks");
    formPanel->addHeader(trackSelector);
    
    initButton = new SimpleButton("Initialize");
    initButton->addListener(this);
    initAllButton = new SimpleButton("Initialize All");
    initAllButton->addListener(this);
    
    captureButton = new SimpleButton("Capture");
    captureButton->addListener(this);
    captureAllButton = new SimpleButton("Capture All");
    captureAllButton->addListener(this);

    Panel* buttons = new Panel(Panel::Orientation::Horizontal);
    buttons->addOwned(initButton);
    buttons->addOwned(initAllButton);
    buttons->addOwned(captureButton);
    buttons->addOwned(captureAllButton);
    buttons->autoSize();
    // would be nice to have a panel option that adjusts the
    // width of all the buttons to be the same
    // buttons->setUniformWidth(true);
    formPanel->addFooter(buttons);

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
    
    addField("Tracks", UIParameterTrackName);
    addField("Tracks", UIParameterDefaultSyncSource);    // overrides default SyncSourceParameter
    addField("Tracks", UIParameterTrackSyncUnit);
    addField("Tracks", UIParameterPreset);
    addField("Tracks", UIParameterGroup);
    addField("Tracks", UIParameterFocus);
    addField("Tracks", UIParameterInput);
    addField("Tracks", UIParameterOutput);
    addField("Tracks", UIParameterFeedback);
    addField("Tracks", UIParameterAltFeedback);
    addField("Tracks", UIParameterPan);
    addField("Tracks", UIParameterMono);

    // these were arranged on a 4x4 sub-grid
    addField("Tracks", UIParameterAudioInputPort);
    addField("Tracks", UIParameterAudioOutputPort);
    addField("Tracks", UIParameterPluginInputPort);
    addField("Tracks", UIParameterPluginOutputPort);

    addField("Synchronization", UIParameterSyncSource);   // should be DefaultSyncSourceParameter
    addField("Synchronization", UIParameterDefaultTrackSyncUnit);
    addField("Synchronization", UIParameterSlaveSyncUnit);
    addField("Synchronization", UIParameterBeatsPerBar);
    addField("Synchronization", UIParameterRealignTime);
    addField("Synchronization", UIParameterOutRealign);
    addField("Synchronization", UIParameterMuteSyncMode);
    addField("Synchronization", UIParameterResizeSyncAdjust);
    addField("Synchronization", UIParameterSpeedSyncAdjust);
    addField("Synchronization", UIParameterMinTempo);
    addField("Synchronization", UIParameterMaxTempo);
    addField("Synchronization", UIParameterManualStart);

    // Other

    addField("Other", UIParameterActiveTrack);

    // this one has special values
    //form.add(buildResetablesField(), "Other");

    // Binding Overlay
}

void SetupPanel::addField(const char* tab, UIParameter* p)
{
    form.add(new ParameterField(p), tab, 0);
}

#if 0
Field* SetupPanel::buildResetablesField()
{
    Field* field = new ParameterField(UIParameterResetables);
    juce::StringArray values;
    juce::StringArray displayValues;
    
    // values are defined by Parameter flags
	for (int i = 0 ; i < UIParameter::Parameters.size() ; i++) {
        UIParameter* p = UIParameter::Parameters[i];
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
#endif

//////////////////////////////////////////////////////////////////////
//
// Listneners
//
//////////////////////////////////////////////////////////////////////

void SetupPanel::radioSelected(SimpleRadio* r, int index)
{
    Trace(1, "Track %d\n", index);
}

void SetupPanel::buttonClicked(juce::Button* b)
{
    Trace(1, "Button %s\n", b->getButtonText().toUTF8());
}



#include <JuceHeader.h>

#include <string>
#include <sstream>

#include "../model/Parameter.h"

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

void SetupPanel::load()
{
}

void SetupPanel::save()
{
}

void SetupPanel::cancel()
{
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
    // Active Track
    // Restore After Reset
    // Binding Overlay
}

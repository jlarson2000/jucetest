
#include <JuceHeader.h>

#include <string>
#include <sstream>

#include "../util/qtrace.h"

#include "SetupPanel.h"

SetupPanel::SetupPanel(ConfigEditor* argEditor) :
    ConfigPanel{argEditor, "Track Setups", ConfigPanelButton::Save | ConfigPanelButton::Cancel, true},
    tabs {juce::TabbedButtonBar::Orientation::TabsAtTop}
{
    // debugging hack
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


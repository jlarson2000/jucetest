
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
}

    

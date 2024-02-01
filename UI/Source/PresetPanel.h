/**
 * ConfigPanel to edit presets
 */

#pragma once

#include <JuceHeader.h>

#include "ConfigPanel.h"

class PresetPanel : public ConfigPanel 
{
  public:
    PresetPanel();
    ~PresetPanel();

  private:
    
    bool loaded = false;
    juce::OwnedArray<Preset*> presets;
    int selectedPreset = 0;
};

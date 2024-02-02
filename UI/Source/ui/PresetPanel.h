/**
 * ConfigPanel to edit presets
 */

#pragma once

#include <JuceHeader.h>

#include "../model/Parameter.h"
#include "../model/Preset.h"

#include "Form.h"
#include "ConfigPanel.h"

class PresetPanel : public ConfigPanel 
{
  public:
    PresetPanel();
    ~PresetPanel();

    void load();
    void render();
    
  private:

    void loadPreset(int index);
    void savePreset(int index);
    Preset* getSelectedPreset();

    void initForm();
    void add(const char* tab, Parameter* p, int column = 0);
    
    bool loaded = false;
    juce::OwnedArray<Preset> presets;
    int selectedPreset = 0;

    Form form;

};

/**
 * ConfigPanel to edit presets
 */

#pragma once

#include <JuceHeader.h>

#include "../../model/Preset.h"
#include "../common/Form.h"
#include "ConfigPanel.h"

class PresetPanel : public ConfigPanel 
{
  public:
    PresetPanel(class ConfigEditor*);
    ~PresetPanel();

    // ConfigPanel overloads
    void load();
    void save();
    void cancel();

    // ObjectSelector overloads
    void selectObject(int ordinal);
    void newObject() override;
    void deleteObject() override;
    void revertObject() override;
    void renameObject(juce::String) override;

  private:

    void render();
    void initForm();
    void addField(const char* tab, class UIParameter* p, int col = 0);
    
    void loadPreset(int index);
    void savePreset(int index);
    Preset* getSelectedPreset();
    
    juce::OwnedArray<Preset> presets;
    // another copy for revert
    juce::OwnedArray<Preset> revertPresets;
    int selectedPreset = 0;

    Form form;

};

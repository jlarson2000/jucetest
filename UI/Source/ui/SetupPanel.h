/**
 * ConfigPanel to edit track setups
 */

#pragma once

#include <JuceHeader.h>

#include "../model/Setup.h"

#include "Form.h"
#include "ConfigPanel.h"

class SetupPanel : public ConfigPanel 
{
  public:
    SetupPanel(class ConfigEditor *);
    ~SetupPanel();

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
    
    void loadSetup(int index);
    void saveSetup(int index);
    Setup* getSelectedSetup();
    
    bool active = false;
    juce::OwnedArray<Setup> setups;
    int selectedSetup = 0;

    Form form;
};

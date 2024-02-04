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

    FieldGrid fields;
    juce::TabbedComponent tabs;
    
};

/**
 * Panel to edit UI button bindings
 */

#pragma once

#include <JuceHeader.h>

#include "ConfigPanel.h"
#include "BindingPanel.h"

class ButtonPanel : public BindingPanel
{
  public:
    ButtonPanel(class ConfigEditor *);
    ~ButtonPanel();

    // BindingPanel overloads
    juce::String renderSubclassTrigger(Binding* b);
    bool isRelevant(class Binding* b);
    void addSubclassFields() override;
    void refreshSubclassFields(class Binding* b);
    void captureSubclassFields(class Binding* b);
    void resetSubclassFields();

    void upgradeBindings() override;
    void saveBindingUpgrades() override;
    
  private:

};


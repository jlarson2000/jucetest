/**
 * Panel to edit keyboard bindings
 */

#pragma once

#include <JuceHeader.h>

#include "../common/SimpleTable.h"
#include "../common/ButtonBar.h"
#include "../common/Field.h"

#include "ConfigPanel.h"
#include "BindingPanel.h"

class KeyboardPanel : public BindingPanel, public juce::KeyListener
{
  public:
    KeyboardPanel(class ConfigEditor *);
    ~KeyboardPanel();

    juce::String renderSubclassTrigger(Binding* b);
    bool isRelevant(class Binding* b);
    void addSubclassFields() override;
    void refreshSubclassFields(class Binding* b);
    void captureSubclassFields(class Binding* b);
    void resetSubclassFields();

    bool keyPressed(const juce::KeyPress& key, juce::Component* originator) override;
    bool keyStateChanged(bool isKeyDown, juce::Component* originator) override;

  private:

    Field* key = nullptr;
    Field* capture = nullptr;
    
};

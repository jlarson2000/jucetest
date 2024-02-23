/**
 * Panel to edit keyboard bindings
 */

#pragma once

#include <JuceHeader.h>

#include "../../KeyTracker.h"
#include "../common/SimpleTable.h"
#include "../common/ButtonBar.h"
#include "../common/Field.h"

#include "ConfigPanel.h"
#include "BindingPanel.h"

class KeyboardPanel : public BindingPanel, public juce::KeyListener, public KeyTracker::Listener
{
  public:
    KeyboardPanel(class ConfigEditor *);
    ~KeyboardPanel();

    void showing() override;
    void hiding() override;

    juce::String renderSubclassTrigger(Binding* b);
    bool isRelevant(class Binding* b);
    void addSubclassFields() override;
    void refreshSubclassFields(class Binding* b);
    void captureSubclassFields(class Binding* b);
    void resetSubclassFields();

    bool keyPressed(const juce::KeyPress& key, juce::Component* originator) override;
    bool keyStateChanged(bool isKeyDown, juce::Component* originator) override;

    void keyTrackerDown(int code, int modifiers);
    void keyTrackerUp(int code, int modifiers);
    
  private:

    Field* key = nullptr;
    Field* capture = nullptr;
    
};

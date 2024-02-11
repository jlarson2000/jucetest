/**
 * ButtonPanel to edit display buttons
 */

#pragma once

#include <JuceHeader.h>

#include "../model/UIConfig.h"

#include "ConfigPanel.h"
#include "SimpleTable.h"

class ButtonPanel : public ConfigPanel, public SimpleTable::Listener
{
  public:
    ButtonPanel(class ConfigEditor *);
    ~ButtonPanel();

    // ConfigPanel overloads
    void load();
    void save();
    void cancel();

    void resized();

    void tableTouched(SimpleTable* table);
    
  private:

    void render();

    juce::OwnedArray<UIButton> buttons;
    SimpleTable table;

    
};

/**
 * ButtonPanel to edit display buttons
 */

#pragma once

#include <JuceHeader.h>

#include "../common/SimpleTable.h"
#include "../common/ButtonBar.h"
#include "../common/Field.h"

#include "ConfigPanel.h"
#include "BindingTargetPanel.h"

class ButtonPanel :
   public ConfigPanel, public SimpleTable::Listener, public ButtonBar::Listener, public Field::Listener
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
    void buttonClicked(juce::String name);
    void fieldSet(Field* field);
    
  private:

    void render();
    void rebuildTable();
    
    juce::OwnedArray<class UIButton> buttons;
    SimpleTable table;
    BindingTargetPanel targets;
    ButtonBar commands;
    Field arguments {"Arguments", Field::Type::String};
};

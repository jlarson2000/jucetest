/**
 * Panel to edit keyboard bindings
 */

#pragma once

#include <JuceHeader.h>

#include "../common/SimpleTable.h"
#include "../common/ButtonBar.h"
#include "../common/Field.h"

#include "ConfigPanel.h"
#include "BindingTargetPanel.h"

class KeyboardPanel :
   public ConfigPanel, public SimpleTable::Listener, public KeyboardBar::Listener, public Field::Listener
{
  public:
    KeyboardPanel(class ConfigEditor *);
    ~KeyboardPanel();

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
    
    juce::OwnedArray<class Binding> bindings;
    SimpleTable table;
    BindingTargetPanel targets;
    ButtonBar commands;
    Field arguments {"Arguments", Field::Type::String};

    class Binding* copyBinding(Binding* src);

};

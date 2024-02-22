/**
 * Panel to edit MIDI bindings
 */

#pragma once

#include <JuceHeader.h>

#include "../common/SimpleTable.h"
#include "../common/ButtonBar.h"
#include "../common/Field.h"

#include "ConfigPanel.h"
#include "BindingPanel.h"

class MidiPanel : public BindingPanel
{
  public:
    MidiPanel(class ConfigEditor *);
    ~MidiPanel();

    juce::String renderSubclassTrigger(Binding* b);
    bool isRelevant(class Binding* b);
    void addSubclassFields() override;
    void refreshSubclassFields(class Binding* b);
    void captureSubclassFields(class Binding* b);
    void resetSubclassFields();

  private:

    Field* key = nullptr;

};

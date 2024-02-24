/**
 * Panel to edit MIDI bindings
 */

#pragma once

#include <JuceHeader.h>

#include "../../MidiManager.h"
#include "../common/SimpleTable.h"
#include "../common/ButtonBar.h"
#include "../common/Field.h"

#include "ConfigPanel.h"
#include "BindingPanel.h"

class MidiPanel : public BindingPanel, public MidiManager::Listener
{
  public:
    MidiPanel(class ConfigEditor *);
    ~MidiPanel();

    void showing() override;
    void hiding() override;

    juce::String renderSubclassTrigger(Binding* b);
    bool isRelevant(class Binding* b);
    void addSubclassFields() override;
    void refreshSubclassFields(class Binding* b);
    void captureSubclassFields(class Binding* b);
    void resetSubclassFields();

    void midiMessage(const class juce::MidiMessage& message, juce::String& source);

  private:

    Field* messageType = nullptr;
    Field* messageChannel = nullptr;
    Field* messageValue = nullptr;
    Field* capture = nullptr;
};

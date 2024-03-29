/*
 * Arranges a configurable list of ActionButtons in a row with auto
 * wrapping and sizing.
 *
 * Reads the list of buttons to display from UIConfig
 * Resolves UIButton bindings to Actions.
 */

#pragma once

#include <JuceHeader.h>

// for DynamicConfigListener
#include "../../Supervisor.h"

#include "ActionButton.h"

class ActionButtons : public juce::Component, public juce::Button::Listener, public Supervisor::DynamicConfigListener
{
  public:

    ActionButtons(class MobiusDisplay*);
    ~ActionButtons();

    void configure(class MobiusConfig* config);
    int getPreferredHeight(juce::Rectangle<int>);
    void layout(juce::Rectangle<int>);
    
    void resized() override;
    void paint (juce::Graphics& g) override;
    void buttonClicked(juce::Button* b) override;
    void buttonStateChanged(juce::Button* b) override;
    
    void add(class ActionButton* b);

    void dynamicConfigChanged(class DynamicConfig* config);
    
  private:

    // experiment with sustainable buttons
    bool enableSustain = true;

    class MobiusDisplay* display;
    juce::OwnedArray<class ActionButton> buttons;

    void centerRow(int start, int end, int rowWidth, int availableWidth);

    void addButton(ActionButton* b);
    void removeButton(ActionButton* b);
    void buildButtons(class MobiusConfig* c);
    void assignTriggerIds();
    void buttonUp(ActionButton* b);
};

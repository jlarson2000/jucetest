/*
 * Arranges a configurable list of ActionButtons in a row with auto
 * wrapping and sizing.
 *
 * Reads the list of buttons to display from UIConfig
 * Resolves UIButton bindings to Actions.
 */

#pragma once

#include <JuceHeader.h>

#include "ActionButton.h"

class ActionButtons : public juce::Component, public juce::Button::Listener
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
    
    void add(class ActionButton* b);

  private:

    class MobiusDisplay* display;
    juce::OwnedArray<class ActionButton> buttons;

    void centerRow(int start, int end, int rowWidth, int availableWidth);
};

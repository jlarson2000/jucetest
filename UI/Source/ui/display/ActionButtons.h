/*
 * Arranges a configurable list of ActionButtons in a row with auto
 * wrapping and sizing.
 *
 * Reads the list of buttons to display from UIConfig
 * Resolves UIButton bindings to Actions.
 */

#pragma once

#include <JuceHeader.h>

/**
 * Helper component used to center the buttons within each row
 */
class ActionButtonRow : public juce::Component
{
  public:
    ActionButtonRow() {};
    ~ActionButtonRow() {};
};

class ActionButtons : public juce::Component, public juce::Button::Listener
{
  public:

    ActionButtons();
    ~ActionButtons();

    void configure(class UIConfig* config);
    void layout(juce::Rectangle<int>);
    
    void resized() override;
    void paint (juce::Graphics& g) override;
    void buttonClicked(juce::Button* b);
    
  private:

    void centerRow(int start, int end, int rowWidth, int availableWidth);

    juce::String formatButtonName(class UIButton *src);

    juce::OwnedArray<class ActionButton> buttons;
    
};

    

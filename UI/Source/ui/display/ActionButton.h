/*
 * An extension of juce::Button to associate the visible
 * button with a Mobius Action.  These are arranged in a
 * configurable row by ActionButtons.
 */

#pragma once

#include <JuceHeader.h>

#include "../../model/UIAction.h"

class ActionButton : public juce::TextButton, public juce::Button::Listener
{
  public:

    ActionButton();
    ActionButton(class UIButton* src);
    ~ActionButton();

    int getPreferredWidth(int height);

    void buttonClicked(juce::Button* b) override;
    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted,
                     bool shouldDrawButtonAsDown) override;
    
    class UIAction* getAction();

  private:

    UIAction action;

    void paintButton(juce::Graphics& g, juce::Colour background, juce::Colour text);

    juce::String formatButtonName(UIButton *src);
    void initAction(UIButton* src);
    
};



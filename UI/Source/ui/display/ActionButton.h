/*
 * An extension of juce::Button to associate the visible
 * button with a Mobius Action.  These are arranged in a
 * configurable row by ActionButtons.
 */

#pragma once

#include <JuceHeader.h>

class ActionButton : public juce::TextButton
{
  public:

    // todo: construct with a resolved Action
    ActionButton(juce::String name);
    ~ActionButton();

    int getPreferredWidth(int height);

    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted,
                     bool shouldDrawButtonAsDown) override;

    void execute();
    
  private:

    void paintButton(juce::Graphics& g, juce::Colour background, juce::Colour text);


};



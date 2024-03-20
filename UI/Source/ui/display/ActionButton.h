/*
 * An extension of juce::Button to associate the visible
 * button with a Mobius Action.  These are arranged in a
 * configurable row by ActionButtons.
 */

#pragma once

#include <JuceHeader.h>

#include "../../model/UIAction.h"

class ActionButton : public juce::TextButton
{
  public:

    ActionButton();
    ActionButton(class Binding* src);
    ActionButton(class DynamicAction* src);
    ~ActionButton();

    int getPreferredWidth(int height);

    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted,
                     bool shouldDrawButtonAsDown) override;
    
    class UIAction* getAction();

    void setDynamic(bool b) {
        dynamic = true;
    }

    bool isDynamic() {
        return dynamic;
    }

    void setTriggerId(int id);

    // called by ActionButtons as down/up transitions are detected
    void setDownTracker(bool b);
    bool isDownTracker();
    
  private:

    UIAction action;
    bool dynamic = false;
    bool downTracker = false;
    
    void paintButton(juce::Graphics& g, juce::Colour background, juce::Colour text);

    juce::String formatButtonName(class Binding *src);
    
};



/**
 * A component that displays a conigurable set of StatusElements
 * to display and control Mobius runtime state.
 *
 * Found at the in the center of MobiusDisplay.
 */

#pragma once

#include <JuceHeader.h>

#include "../common/SimpleButton.h"
#include "ActionButton.h"
#include "ActionButtons.h"

class StatusArea : public juce::Component, public juce::Button::Listener
{
  public:
    
    StatusArea(class MobiusDisplay*);
    ~StatusArea();

    void configure(class UIConfig* config);
    
    void resized() override;
    void paint(juce::Graphics& g) override;

    void buttonClicked(juce::Button* b) override;
    
  private:

    class MobiusDisplay* display;
    SimpleButton button;
    ActionButton abutton {nullptr};
    ActionButtonRow row;
    
    void drawText(juce::Graphics& g, const char* text, int x, int y);
    
};


    

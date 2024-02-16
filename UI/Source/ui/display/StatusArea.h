/**
 * A component that displays a conigurable set of StatusElements
 * to display and control Mobius runtime state.
 *
 * Found at the in the center of MobiusDisplay.
 */

#pragma once

#include <JuceHeader.h>

#include "ModeElement.h"

class StatusArea : public juce::Component
{
  public:
    
    StatusArea(class MobiusDisplay*);
    ~StatusArea();

    void configure(class UIConfig* config);
    void update(class MobiusState* state);
    
    void resized() override;
    void paint(juce::Graphics& g) override;
    
  private:

    class MobiusDisplay* display;
    
    ModeElement mode {this};

    void drawText(juce::Graphics& g, const char* text, int x, int y);
    
};


    

/*
 * The Mobius status display area.
 * This will be inside MainWindow under the menu bar.
 */

#pragma once

#include <JuceHeader.h>

#include "display/ActionButtons.h"

class MainDisplay : public juce::Component
{
  public:

    MainDisplay();
    ~MainDisplay();
    void configure(class UIConfig* config);

    void drawText(juce::Graphics& g, const char* text, int x, int y);
    
    void resized() override;
    void paint (juce::Graphics& g) override;

  private:

    ActionButtons buttons;
    
};    

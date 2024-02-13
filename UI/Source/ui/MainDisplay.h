/*
 * The Mobius status display area.
 * This will be inside MainWindow under the menu bar.
 */

#pragma once

#include <JuceHeader.h>

#define MobiusBlue 0xFF8080FF
#define MobiusGreen 0xFF00b000
#define MobiusRed 0xFFf40b74
#define MobiusYellow 0xFFFFFF00
#define MobiusPink 0xFFFF8080
#define MobiusDarkYellow 0xFFe0bd00

class MainDisplay : public juce::Component
{
  public:

    MainDisplay();
    ~MainDisplay();

    void drawText(juce::Graphics& g, const char* text, int x, int y);

    void resized() override;
    void paint (juce::Graphics& g) override;

  private:

 
};    

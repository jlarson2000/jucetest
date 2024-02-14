/**
 * Manages everying related the Mobius user interface.
 * Here is where we might swap in different display styles
 * based on user preference.
 */

#pragma once

#include <JuceHeader.h>

class DisplayManager
{
  public:

    DisplayManager(class Supervisor* super, juce::Component* main);
    ~DisplayManager();

    void configure(class UIConfig* config);
    void configure(class MobiusConfig* config);

  private:

    class Supervisor* supervisor;
    juce::Component* mainComponent;

    // one of the selected displays
    std::unique_ptr<class MainWindow> mainWindow;
    
};    

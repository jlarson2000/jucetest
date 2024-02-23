/**
 * Class that manages the mapping between external events
 * and actions sent to the Mobius engine.
 */

#pragma once

#include <JuceHeader.h>

#include "KeyTracker.h"

class Binderator : public KeyTracker::Listener
{
  public:

    Binderator(class Supervisor* super);
    ~Binderator();

    void configure(class MobiusConfig* config);
    void start();
    void stop();
    
    void keyTrackerDown(int code, int modifiers);
    void keyTrackerUp(int code, int modifiers);

  private:

    class Supervisor* supervisor = nullptr;
    bool started = false;

    juce::OwnedArray<class UIAction> keyActions;

    void installAction(class Binding* b);

    
};

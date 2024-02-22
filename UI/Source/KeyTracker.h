#pragma once

#include <JuceHeader.h>

class KeyTracker
{
  public:

    class Listener {
      public:
        virtual void keyTrackerDown(int code) = 0;
        virtual void keyTrackerUp(int code) = 0;
    };
    
    KeyTracker();
    ~KeyTracker();
    void addListener(Listener* l);

    bool keyPressed(const juce::KeyPress& key, juce::Component* originator);
    bool keyStateChanged(bool isKeyDown, juce::Component* originator);

  private:

    juce::Array<Listener*> listeners;
    
};

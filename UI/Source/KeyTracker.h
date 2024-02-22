#pragma once

#include <JuceHeader.h>

class KeyTracker
{
  public:

    KeyTracker();
    ~KeyTracker();

    bool keyPressed(const juce::KeyPress& key, juce::Component* originator);
    bool keyStateChanged(bool isKeyDown, juce::Component* originator);

  private:

};

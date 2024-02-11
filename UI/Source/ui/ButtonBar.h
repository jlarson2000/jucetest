
#pragma once

#include <JuceHeader.h>

class ButtonBar : public juce::Component, public juce::Button::Listener
{
  public:

    class Listener {
      public:
        virtual void buttonClicked(juce::String name) = 0;
    };

    ButtonBar();
    ~ButtonBar();

    void add(juce::String name);

    void resized();

    void addListener(Listener* l) {
        listener = l;
    }
    
    // Button::Listener
    virtual void buttonClicked(juce::Button* b);

  private:

    juce::OwnedArray<juce::TextButton> buttons;
    Listener* listener;
    
};

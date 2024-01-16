
#pragma once

#include <JuceHeader.h>

class PopupTest : public juce::Component, juce::Button::Listener
{
  public:

    class Listener
    {
      public:
        virtual void popupClosed() = 0;
    };

    PopupTest();
    ~PopupTest() override;

    // Component
    void paint (juce::Graphics& g) override;
    void resized() override;

    // Button::Listener
    void buttonClicked(juce::Button* b);
    
    // local
    void center();
    void setListener(Listener* listener);
    
  private:
    
    juce::Label label;
    juce::TextButton closeButton;
    Listener* listener;
    
};

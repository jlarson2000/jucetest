
#pragma once

#include <JuceHeader.h>

class PopupTest : public juce::Component
{
  public:

    PopupTest();
    ~PopupTest() override;
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void center();
    
  private:
    
    juce::Label label;
};

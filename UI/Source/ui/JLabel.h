
#pragma once

#include <JuceHeader.h>

class JLabel : public juce::Component
{
  public:

    JLabel();
    JLabel(juce::String);
    JLabel(const char*);
    ~JLabel();

    void setText(juce::String);
    // TODO: fonts, colors
    
    void resized();
    
  private:

    juce::Label label;
};


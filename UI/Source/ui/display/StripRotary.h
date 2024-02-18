/**
 * Base class of a strip element that shows a rotary control.
 */

#pragma once

class StripRotary : public StripElement
{
  public:
    
    StripRotary(class TrackStrip* parent, class StripElementDefinition* def);
    ~StripRotary();

    int getPreferredHeight() override;
    int getPreferredWidth() override;

    void resized() override;
    void paint(juce::Graphics& g) override;
    
  protected:

    int min;
    int max;
    int value;
    
    juce::Slider slider;
    
};

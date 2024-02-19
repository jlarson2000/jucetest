/**
 * Base class of a strip element that shows a rotary control.
 */

#pragma once

#include <JuceHeader.h>

#include "../../model/UIAction.h"

class StripRotary : public StripElement, public juce::Slider::Listener
{
  public:
    
    StripRotary(class TrackStrip* parent, class StripElementDefinition* def);
    ~StripRotary();

    int getPreferredHeight() override;
    int getPreferredWidth() override;

    void resized() override;
    void paint(juce::Graphics& g) override;
    
    // don't care about DragStarted and DragEnded
    virtual void sliderValueChanged(juce::Slider* slider);
    virtual void sliderDragStarted(juce::Slider* slider);
    virtual void sliderDragEnded(juce::Slider* slider);
    
  protected:

    int min;
    int max;
    int value;
    
    juce::Slider slider;
    UIAction action;
    const char* label = nullptr;
    bool dragging;
};

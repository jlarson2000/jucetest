
#include <JuceHeader.h>

#include "Colors.h"
#include "TrackStrip.h"
#include "StripElement.h"
#include "StripRotary.h"


const int BoxWidth = 30;
const int BoxHeight = 20;
const int RotaryDiameter = 50;

StripRotary::StripRotary(class TrackStrip* parent, StripElementDefinition* def) :
    StripElement(parent, def)
{
    // set an inital range we'll change later
    slider.setRange(0.0f, 127.0f, 1);
    slider.setColour(juce::Slider::ColourIds::textBoxTextColourId, juce::Colours::black);
    slider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    // position, readOnly, width, height
    slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, true, BoxWidth, BoxHeight);
    addAndMakeVisible(slider);
}

StripRotary::~StripRotary()
{
}

int StripRotary::getPreferredWidth()
{
    return (BoxWidth > RotaryDiameter) ? BoxWidth : RotaryDiameter;
}

int StripRotary::getPreferredHeight()
{
    // how to these divide up the space?
    return RotaryDiameter + BoxHeight;
}

void StripRotary::resized()
{
    slider.setBounds(getLocalBounds());
}

void StripRotary::paint(juce::Graphics& g)
{
}

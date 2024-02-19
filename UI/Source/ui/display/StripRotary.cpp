
#include <JuceHeader.h>

#include "Colors.h"
#include "TrackStrip.h"
#include "StripElement.h"
#include "StripRotary.h"

// not using value boxes any more
const int BoxWidth = 30;
const int BoxHeight = 20;

const int RotaryDiameter = 60;
const int LabelFontHeight = 14;

// kludge: the rotary draws with a large gap at the bottom
// can't find a way to reduce that so we'll draw the label
// over the bottom part to tighten it up
// so the LabelGap will actually be negative
const int LabelGap = -10;

StripRotary::StripRotary(class TrackStrip* parent, StripElementDefinition* def) :
    StripElement(parent, def)
{
    // set an inital range we'll change later
    slider.setRange(0.0f, 127.0f, 1);
    slider.setColour(juce::Slider::ColourIds::textBoxTextColourId, juce::Colours::black);
    slider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    // position, readOnly, width, height
    // disable the text box, it adds clutter and we don't need to
    // type in numbers for things like output level
    // started with TextBoxBelow but it never did display the number
    // which although the old interface did, we can do without since
    // you can't do that in the middle of the component without overriding
    // the look and feel
    slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    slider.addListener(this);

    addAndMakeVisible(slider);

    // initialize the action we will send
    action.trigger = TriggerUI;
    // hate these, figure out why it's necessary
    action.triggerMode = TriggerModeContinuous;
    action.target = TargetParameter;

    // save this since we always want these labeled
    label = (def->displayName != nullptr) ? def->displayName : def->name;

    dragging = false;
}


StripRotary::~StripRotary()
{
}

/**
 * We always want these labeled.
 * Have some flexibility on the font here..
 * Not showing the slider value box so don't have to deal with it
 */
int StripRotary::getPreferredWidth()
{
    int maxWidth = 0;
    
    if (label != nullptr) {
        juce::Font font(LabelFontHeight);
        maxWidth = font.getStringWidth(label);
    }

    if (RotaryDiameter > maxWidth)
      maxWidth = RotaryDiameter;

    return maxWidth;
}

int StripRotary::getPreferredHeight()
{
    return RotaryDiameter + LabelFontHeight + LabelGap;
}

void StripRotary::resized()
{
    // since the label is usually wider than the
    // minimum slider size, the rotary will also
    // enlarge to fill the space, not bad but
    // depending on how they stack may want to constrain it

    // sigh, still way too much default space around the rotary
    // how can we constrain it, it doesn't seem to track the width
    //slider.setBounds(0, 0, RotaryDiameter, RotaryDiameter);
    // let it fill width to center
    slider.setBounds(0, 0, getWidth(), RotaryDiameter);
}

void StripRotary::paint(juce::Graphics& g)
{
    // trying to figure out where the knob bottom is
    // because there is a large gap
    //g.setColour(juce::Colours::red);
    //g.drawRect(0, 0, getWidth(), RotaryDiameter);
    
    int top = RotaryDiameter + LabelGap;
    g.setColour(juce::Colour(MobiusBlue));
    g.setFont(juce::Font(LabelFontHeight));
    g.drawText(juce::String(label), 0, top, getWidth(), LabelFontHeight,
               juce::Justification::centred);
}

// expected to be overridden to do something
void StripRotary::sliderValueChanged(juce::Slider* slider)
{
}

void StripRotary::sliderDragStarted(juce::Slider* slider)
{
    dragging = true;
}

void StripRotary::sliderDragEnded(juce::Slider* slider)
{
    dragging = false;
}

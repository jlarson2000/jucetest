//
// 

#include <JuceHeader.h>

#include "PopupTest.h"

PopupTest::PopupTest()
{
    addAndMakeVisible (label);
    label.setFont (juce::Font (16.0f, juce::Font::bold));
    label.setText ("Popup Test", juce::dontSendNotification);
    label.setColour (juce::Label::textColourId, juce::Colours::lightgreen);
    label.setJustificationType (juce::Justification::centred);
    
    setSize (200, 200);

    setAlwaysOnTop(true);
}

PopupTest::~PopupTest()
{
}

void PopupTest::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::blue);

    juce::Rectangle<int> r = getBounds();
    g.setColour(juce::Colours::sandybrown);
    g.drawRect(r);
}

void PopupTest::resized()
{
    label.setBounds (10,  100, getWidth() - 20,  30);
}

void PopupTest::center()
{
    int pwidth = getParentWidth();
    int pheight = getParentHeight();
    int mywidth = getWidth();
    int myheight = getHeight();
    
    if (mywidth > pwidth) mywidth = pwidth;
    if (myheight > pheight) myheight = pheight;

    int left = (pwidth - mywidth) / 2;
    int top = (pheight - myheight) / 2;
    setTopLeftPosition(left, top);
}

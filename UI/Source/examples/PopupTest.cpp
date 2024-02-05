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
    
    setSize (500, 500);

    setAlwaysOnTop(true);

    addAndMakeVisible(closeButton);
    closeButton.setButtonText("Close");
    closeButton.addListener(this);
    
}

PopupTest::~PopupTest()
{
}

void PopupTest::setListener(Listener* l)
{
    listener = l;
}

void PopupTest::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::blue);
}

void PopupTest::resized()
{
    label.setBounds (10,  100, getWidth() - 20,  30);
    // closeButton.setTopLeftPosition(10, 300);
    closeButton.setBounds(10, 300, 100, 30);
}

/**
 * Called by MainComponent to center this within the available space
 * We should be able to do this ourselves
 */
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

/**
 * juce::Button::Listener
 * Subclasses are responsible for overloading this to take
 * the appropriate action.  Here, we just notify the parent
 * to disable the component.
 *
 * hmm, think more about "this" as a listener and redirecting
 * to the subclass implementation, will that work?
 */
void PopupTest::buttonClicked(juce::Button* b)
{
    if (listener != nullptr)
      listener->popupClosed();
}


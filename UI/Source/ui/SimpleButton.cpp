/*
 * Provides a basic labeled button.
 * Unlike the other "Simple" component extensions we don't
 * need one as much for Button since the base interface
 * isn't bad but keep let's start with it for size shenanigans.
 */

#include <JuceHeader.h>
#include "SimpleButton.h"

SimpleButton::SimpleButton()
{
    setName("SimpleButton");
}

SimpleButton::SimpleButton(const char* text)
{
    setName("SimpleButton");
    setButtonText(juce::String(text));
    render();
}

SimpleButton::~SimpleButton()
{
}

void SimpleButton::render()
{
    // could auto-size these based on text, let's start with fixed width
    // for initial testing
    // juce::String text = getButtonText();
    
    setSize(100, 30);
}

void SimpleButton::resized()
{
}


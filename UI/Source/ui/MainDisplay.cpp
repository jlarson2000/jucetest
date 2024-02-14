/*
 * The Mobius status display area.
 * This will be inside MainWindow under the menu bar.
 */

#pragma once

#include <JuceHeader.h>

#include "../model/UIConfig.h"
#include "display/Colors.h"
#include "display/ActionButtons.h"
#include "JuceUtil.h"

#include "MainDisplay.h"

MainDisplay::MainDisplay()
{
    setName("MainDisplay");

    addAndMakeVisible(buttons);
}

MainDisplay::~MainDisplay()
{
}

/**
 * Configure ourselves after a UIConfig change
 */
void MainDisplay::configure(UIConfig* config)
{
    buttons.configure(config);
    // need to resize to adjust for new button layout
    resized();
}

void MainDisplay::resized()
{
    juce::Rectangle<int> area = getLocalBounds();

    // leave a gap between the MainWindow menu and the top of the buttons
    area.removeFromTop(20);
    
    // we call layout() rather than resized() to auto-calculate
    // the necessary height for all buttons
    buttons.layout(area);

    area.removeFromTop(buttons.getHeight());

    // add the display items compoent
    // add the track strips

    JuceUtil::dumpComponent(this);
}

void MainDisplay::paint(juce::Graphics& g)
{
    drawText(g, "Mobius", 100, 100);
}

void MainDisplay::drawText(juce::Graphics& g, const char* text, int x, int y)
{
    juce::Font font(20.0f);

    g.setColour(juce::Colour(MobiusBlue));
    g.setFont(font);
    g.drawText(text, x, y, font.getStringWidth(text), font.getHeight(), juce::Justification::left);
}

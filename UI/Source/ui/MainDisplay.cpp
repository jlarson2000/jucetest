/*
 * The Mobius status display area.
 * This will be inside MainWindow under the menu bar.
 */

#pragma once

#include <JuceHeader.h>

#include "MainDisplay.h"

MainDisplay::MainDisplay()
{
}

MainDisplay::~MainDisplay()
{
}

void MainDisplay::resized()
{
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

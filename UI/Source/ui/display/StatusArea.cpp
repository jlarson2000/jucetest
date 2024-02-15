
#include <JuceHeader.h>

#include "MobiusDisplay.h"
#include "Colors.h"
#include "StatusArea.h"

StatusArea::StatusArea(MobiusDisplay* parent)
{
    setName("StatusArea");
    display = parent;
}

StatusArea::~StatusArea()
{
}

void StatusArea::configure(UIConfig* config)
{
}

void StatusArea::resized()
{
}

void StatusArea::paint(juce::Graphics& g)
{
    drawText(g, "Mobius", 100, 100);
}

void StatusArea::drawText(juce::Graphics& g, const char* text, int x, int y)
{
    juce::Font font(20.0f);

    g.setColour(juce::Colour(MobiusBlue));
    g.setFont(font);
    g.drawText(text, x, y, font.getStringWidth(text), font.getHeight(), juce::Justification::left);
}

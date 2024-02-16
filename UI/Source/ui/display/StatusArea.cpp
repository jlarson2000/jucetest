
#include <JuceHeader.h>

#include "../../util/Trace.h"

#include "MobiusDisplay.h"
#include "Colors.h"
#include "StatusArea.h"

StatusArea::StatusArea(MobiusDisplay* parent)
{
    setName("StatusArea");
    display = parent;

    button.setButtonText("Foo");
    button.addListener(this);

    //row.add(&abutton);
    addAndMakeVisible(row);

    //abutton.addListener(this);
    //addAndMakeVisible(abutton);
}

StatusArea::~StatusArea()
{
}

void StatusArea::configure(UIConfig* config)
{
}

void StatusArea::resized()
{
    //button.setBounds(100, 150, 50, 30);
    //abutton.setBounds(100, 150, 50, 30);
    row.setBounds(100, 150, 300, 30);
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

void StatusArea::buttonClicked(juce::Button* b)
{
    trace("Button clicked\n");
}

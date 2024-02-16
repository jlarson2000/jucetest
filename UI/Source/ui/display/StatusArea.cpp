
#include <JuceHeader.h>

#include "../../util/Trace.h"

#include "MobiusDisplay.h"
#include "Colors.h"
#include "StatusArea.h"

StatusArea::StatusArea(MobiusDisplay* parent)
{
    setName("StatusArea");
    display = parent;

    addChildComponent(mode);
}

StatusArea::~StatusArea()
{
}

void StatusArea::configure(UIConfig* config)
{
    // todo: selective visibility based on config
    mode.setVisible(true);
}

void StatusArea::update(MobiusState* state)
{
    mode.update(state);
}

void StatusArea::resized()
{
    // todo: get location from config
    mode.setBounds(100, 100, mode.getPreferredWidth(), mode.getPreferredHeight());
}

void StatusArea::paint(juce::Graphics& g)
{
    //drawText(g, "Mobius", 100, 100);
}

void StatusArea::drawText(juce::Graphics& g, const char* text, int x, int y)
{
    juce::Font font(20.0f);

    g.setColour(juce::Colour(MobiusBlue));
    g.setFont(font);
    g.drawText(text, x, y, font.getStringWidth(text), font.getHeight(), juce::Justification::left);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

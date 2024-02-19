
#include <JuceHeader.h>

#include "../../model/UIConfig.h"
#include "../../model/MobiusState.h"
#include "../../model/UIAction.h"

#include "Colors.h"
#include "StatusArea.h"
#include "TrackStrip.h"

#include "FloatingStripElement.h"

FloatingStripElement::FloatingStripElement(StatusArea* area) :
    StatusElement(area, "FloatingStripElement")
{
    addAndMakeVisible(&strip);
}

FloatingStripElement::~FloatingStripElement()
{
}

void FloatingStripElement::configure(UIConfig* config)
{
    strip.configure(config);
}

void FloatingStripElement::update(MobiusState* state)
{
    strip.update(state);
}

// hmm, StatusElement does not have the layout()
// interface, have to do it twice, fix!
/*
void FloatingStripElement::layout()
{
    strip.layout();
    setSize(strip.getWidth(), strip.getHeight());
}
*/

int FloatingStripElement::getPreferredHeight()
{
    // kludge we don't have any space yet
    strip.layout(getBoundsInParent());
    return strip.getHeight();
}

int FloatingStripElement::getPreferredWidth()
{
    strip.layout(getBoundsInParent());
    return strip.getWidth();
}

void FloatingStripElement::resized()
{
    //strip.setBounds(getLocalBounds());
}

void FloatingStripElement::paint(juce::Graphics& g)
{
    // borders, labels, etc.
    StatusElement::paint(g);
}

void FloatingStripElement::doAction(UIAction* action)
{
    area->doAction(action);
}

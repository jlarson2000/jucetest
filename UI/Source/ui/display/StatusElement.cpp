
#include <JuceHeader.h>

#include "StatusArea.h"
#include "StatusElement.h"

StatusElement::StatusElement(StatusArea* parent)
{
    setName("StatusElement");   // normally the subclass overrides this
    area = parent;
}

StatusElement::~StatusElement()
{
}

void StatusElement::configure(UIConfig* config)
{
}

void StatusElement::resized()
{
}

void StatusElement::paint(juce::Graphics& g)
{
}



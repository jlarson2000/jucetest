
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

void StatusElement::update(MobiusState* state)
{
}

// these should probably be pure virtual
// any useful thing to do in a default implementation?

int StatusElement::getPreferredWidth()
{
    return 0;
}

int StatusElement::getPreferredHeight()
{
    return 0;
}

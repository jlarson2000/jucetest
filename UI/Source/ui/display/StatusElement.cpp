
#include <JuceHeader.h>

#include "../../util/Trace.h"

#include "Colors.h"
#include "StatusArea.h"
#include "StatusElement.h"

/**
 * Base class constructor called only by the subclasses.
 * The name is set as the Component ID so we can search for it
 * with findChildWithID.  That will also be the name used
 * in the UILocaion model.
 *
 * For diagnostic trace we have so far used the Component name to
 * label them so duplicate it there.  The current expectation is that
 * Component name will be the class name which doesn't have to be the
 * same as the UILocation mame if you wanted to simplify them.
 * But keeping them in sync is fine for now.  Could also change
 * JuceUtil to use the componentID and use that consistently everywhere.
 */
StatusElement::StatusElement(StatusArea* parent, const char* name)
{
    setComponentID(name);
    setName(name);
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
    return 100;
}

int StatusElement::getPreferredHeight()
{
    return 20;
}

/**
 * todo: might want some default painting for borders, labels,
 * and size dragging.  Either the subclass must call back up to this
 * or we have a different paintElement function.
 */
void StatusElement::paint(juce::Graphics& g)
{
    // start by bordering everything
    g.setColour(juce::Colour(MobiusBlue));
    g.drawRect(getLocalBounds(), 1);
}

//////////////////////////////////////////////////////////////////////
//
// Mouse Tracking
//
//////////////////////////////////////////////////////////////////////

void StatusElement::mouseEnter(const juce::MouseEvent& e)
{
    trace("mouse enter\n");
}

void StatusElement::mouseExit(const juce::MouseEvent& e)
{
    trace("mouse exit\n");
}

void StatusElement::mouseDown(const juce::MouseEvent& e)
{
    dragger.startDraggingComponent(this, e);
    dragging = true;
}

void StatusElement::mouseDrag(const juce::MouseEvent& e)
{
    dragger.dragComponent(this, e, nullptr);
}

void StatusElement::mouseUp(const juce::MouseEvent& e)
{
    if (dragging) {
        if (e.getDistanceFromDragStartX() != 0 ||
            e.getDistanceFromDragStartY() != 0) {

            // is this the same, probably not sensitive to which button
            if (!e.mouseWasDraggedSinceMouseDown()) {
                trace("Juce didn't think it was dragging\n");
            }
            trace("New location %d %d\n", getX(), getY());
            //area->saveLocation(this);
            dragging = false;
        }
        else if (e.mouseWasDraggedSinceMouseDown()) {
            trace("Juce thought we were dragging but the position didn't change\n");
        }
    }
    else if (e.mouseWasDraggedSinceMouseDown()) {
        trace("Juce thought we were dragging\n");
    }

    dragging = false;
}

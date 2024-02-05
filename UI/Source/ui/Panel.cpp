/**
 * A basic container component with automatic layout options.
 */

#include <JuceHeader.h>
#include "Panel.h"

Panel::Panel()
{
}

Panel::Panel(Orientation o)
{
    orientation = o;
}

Panel::~Panel()
{
}

void Panel::setOrientation(Orientation o)
{
    orientation = o;
    // TODO: assume this is a constructor time thing
    // don't really need to support dynamic orientation changes
}
    
void Panel::getPreferredWidth()
{
    int width = 0;

    const juce::Array<Component*>& children = getChildren();
    for (int i = 0 ; i < children.size() ; i++) {
        Component* child = children[i];
        int cwidth = child->getWidth();
        if (orientation == Vertical) {
            if (cwidth > width)
              width = cwidth;
        }
        else {
            width += cwidth;
        }
    }

    return width;
}

void Panel::getPreferredHeight()
{
    int height = 0;
    
    const juce::Array<Component*>& children = getChildren();

    for (int i = 0 ; i < children.size() ; i++) {
        Component* child = children[i];
        int cheight = child->getHeight();
        if (orientation == Vertical) {
            height += cheight;
        }
        else {
            if (cheight > height)
              height = cheight;
        }
    }

    return height;
}

void Panel::resized()
{
    // todo: clalculate the minimum boundbox for the children
    // for now, take what we get
    auto area = getLocalBounds();
    const juce::Array<Component*>& children = getChildren();

    // TODO: allow some optional padding around the edges
    int left = 0;
    int top = 0;

    for (int i = 0 ; i < children.size() ; i++) {
        Component* child = children[i];
        child.setTopLeftPosition(left, top);
        if (orientation == Vertical) {
            top += child.getHeight();
        }
        else {
            left += child.getWidth();
        }
    }
}


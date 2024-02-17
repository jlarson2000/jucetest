/**
 * Manages a configurable set of display elements that show various
 * parts of the Mobius engine runtime state.
 *
 * The elements may be selectively enabled with their locations specified
 * by the user using mouse dragging.
 *
 * Elements are currently expected to size themselves using the getPreferred
 * methods though it might be interesting to have them grow or shrink depending on
 * the size of the containing area and the other elements being displayed.
 * Would be really cool to allow the user to specify sizes, so you might want a larger
 * counter, but a really small layer list, etc.
 *
 * Configuration of the elements is stored in UIConfig under the StatusArea/StatusElement
 * tags.  This corresponds to the old Locations/Location model.
 *
 * Unlike ActionButtons, since we have a fixed set of possible child
 * components we can keep them as member objects and don't have to
 * maintain an OwnedArray.
 */

#include <JuceHeader.h>

#include "../../util/Trace.h"
#include "../../model/UIConfig.h"

#include "MobiusDisplay.h"
#include "Colors.h"
#include "StatusArea.h"

StatusArea::StatusArea(MobiusDisplay* parent)
{
    setName("StatusArea");
    display = parent;

    addElement(&mode);
    addElement(&beaters);
    addElement(&meter);
}

void StatusArea::addElement(StatusElement* el)
{
    // will be made visible later in configure()
    addChildComponent(el);
    elements.add(el);
}


StatusArea::~StatusArea()
{
}

void StatusArea::update(MobiusState* state)
{
    for (int i = 0 ; i < elements.size() ; i++) {
        StatusElement* el = elements[i];
        if (el->isVisible())
          el->update(state);
    }
}

void StatusArea::resized()
{
    // The elements will already have been positioned and sized by configure
    // do we need to do it again here?  these don't adapt to container size
    // if any components have juce::Buttons we might have that weird mouse
    // sensitivity problem if the parent wasn't sized before sizing the children
    //mode.setBounds(mode.getX(), mode.getY(), mode.getWidth(), mode.getHeight());
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
/**
 * Callback from the elements to update the saved position after dragging.
 * We're going to ignore this for now and save all the changes during
 * shutdown via saveConfiguration which will be indirectly called by Supervisor
 */
void StatusArea::saveLocation(StatusElement* e)
{
    trace("Component %s wants to be at %d %d\n",
          e->getComponentID(), e->getX(), e->getY());
}

//////////////////////////////////////////////////////////////////////
//
// Configuration
//
//////////////////////////////////////////////////////////////////////

/**
 * We could make the config authoritative over visibility and hide things
 * that are missing.  During development though I'd like to be able to add elements
 * and have them in a default location ready to be dragged rather than having to
 * keep going back to UIConfig.
 *
 * Child components were already added in the constructor, we make them visible.
 */
void StatusArea::configure(UIConfig* config)
{
    std::vector<std::unique_ptr<UILocation>>* locations = config->getLocations();
	if (locations->size() > 0) {
		for (int i = 0 ; i < locations->size() ; i++) {
            UILocation* location = locations->at(i).get();
            // we can only have StatusElements so safe to cast
            // if not, will ahve to use dynamic_cast
            StatusElement* el = (StatusElement*)findChildWithID(location->getName());
            if (el == nullptr) {
                // didn't match any element names, shouldn't happen normally
                // ignore and should remove from list
                trace("Unknown StatusElement %s\n", location->getName());
            }
            else {
                el->setTopLeftPosition(location->x, location->y);
                // these only override the defaults
                int width = (location->width > 0) ? location->width : el->getPreferredWidth();
                int height = (location->height > 0) ? location->height : el->getPreferredHeight();
                el->setSize(width, height);
                if (!location->disabled)
                  el->setVisible(true);
            }
        }
    }

    // if there are any new elements that weren't on the locations list
    // add them and force them on so they can be positioned correctly
    for (int i = 0 ; i < elements.size() ; i++) {
        addMissing(elements[i]);
    }
}

/**
 * If a status element is defined but was not on the location list,
 * add it with default characteristics so it can be seen and dragged into place.
 * Temporary aid for development so we can add new elements without having
 * to remember to update the UIConfig
 */
void StatusArea::addMissing(StatusElement* el)
{
    if (el->getWidth() == 0) {
        trace("Boostrapping location for StatusElement %s\n", el->getComponentID());
        // didn't size it in configure
        // put them at top/left I guess, could center them but won't
        // know that till later
        el->setTopLeftPosition(0, 0);
        el->setSize(el->getPreferredWidth(), el->getPreferredHeight());
        el->setVisible(true);
    }
}

/**
 * Save configuration before exiting.
 * This assumes Supervisor will will walk down the hierarchy asking
 * for contributions.  Since we're the only one right now that has anything
 * to save we could instead push changes as they happen but requires
 * walking up to get Supervisor.  I kind of like the notion of letting
 * anything save changes, could be useful.
 *
 * At this point, everthing should be on the child list provided that
 * we used addMissing.  Can therefore have the side effect of cleaning
 * up the Location list by replacing it entirely rather than incrementally.
 * Handy if you want to change names of things.
 */
bool StatusArea::saveConfiguration(UIConfig* config)
{
    // todo: if it makes any difference could be smarter
    // about detecting changes to avoid writing the config file
    // on every shutdown, makes life messier here though
    bool changes = true;
    
    config->clearLocations();
    for (int i = 0 ; i < elements.size() ; i++) {
        StatusElement* el = elements[i];
        juce::String name = el->getComponentID();
        if (name.length() == 0) {
            // bad dog, bad
            trace("StatusElement with no ID %s, both angry and disappointed\n",
                  el->getName());
        }
        else {
            UILocation* loc = new UILocation();
            loc->setName(name.toUTF8());
            loc->x = el->getX();
            loc->y = el->getY();
            // only size if it differs from the default
            if (el->getWidth() != el->getPreferredWidth())
              loc->width = el->getWidth();
            if (el->getHeight() != el->getPreferredHeight())
              loc->height = el->getHeight();
            config->addLocation(loc);
        }
    }
    return changes;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

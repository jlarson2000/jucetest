/*
 * Arranges a configurable list of ActionButtons in a row with auto
 * wrapping and sizing.
 *
 * Reads the list of buttons to display from UIConfig
 */

#include <JuceHeader.h>

#include "../../model/UIConfig.h"
#include "../JuceUtil.h"
#include "ActionButton.h"
#include "ActionButtons.h"
#include "MobiusDisplay.h"

ActionButtons::ActionButtons(MobiusDisplay* argDisplay)
{
    setName("ActionButtons");
    display = argDisplay;
}

ActionButtons::~ActionButtons()
{
}

void ActionButtons::add(ActionButton* b)
{
    addAndMakeVisible(b);
    b->addListener(this);
}

/**
 * Read the button bindings from the UIConfig and build the list
 * of ActionButtons.  This can be called after the component is
 * constructed to add/remove buttons.  The UIConfig is not
 * under our control and must not be remembered.
 */
void ActionButtons::configure(UIConfig* config)
{
    // remove the current set of buttons
    for (int i = 0 ; i < buttons.size() ; i++) {
      removeChildComponent(buttons[i]);
    }
    buttons.clear();

    std::vector<std::unique_ptr<UIButton>>* uiButtons = config->getButtons();
	if (uiButtons->size() > 0) {
		for (int i = 0 ; i < uiButtons->size() ; i++) {
            UIButton* button = uiButtons->at(i).get();
            ActionButton* b = new ActionButton(button);
            // could use a Listener pattern or just point it
            // back to us since we're the only ones that use them
            b->addListener(this);
            addAndMakeVisible(b);
            buttons.add(b);
		}
	}
}

/**
 * Size oursleves so we can fit all the buttons.
 * Buttons are of variable width depending on their text since
 * some, especially with arguments, might be long.  Continue on one
 * line wrapping when we get to the end of the available space.
 * Expected to be called by the parent's resized() method when
 * it knows the available width.  Our resized() method will then
 * do nothing since the work has been done.
 *
 * Alternately could have a getPreferredHeight and a normal
 * resized() but we would have to do the same positioning calculations
 * in both so why bother.
 *
 * NB: There is a very subtle issue with layout() doing both layout
 * and sizing.  The buttons appear like expected but they are not
 * responsive.  It seems Button requires the parent to be of a predetermined
 * size BEFORE adding the children to initialize mouse sensitive areas
 * or something.  If you position and size the buttons, THEN set the parent
 * size it doesn't work.  Unfortunate because getPreferredHeight does all the
 * work, then we set parent bounds, then we do that work again to layout the
 * buttons.
 *
 * Or it might be something about breaking the usual rules of resized not
 * cascading to the children even though they are already in the desired positions.
 *
 * Well it started working for no obvious reason.
 * If you get here again, what seems to be always reliable is for parent::resized
 * to call getPreferredHeight, then have it call setBounds again even though
 * layout will have already done that, then in our resized() do the layout again.
 * That lead to unresponsive buttons, but after flailing around it started working
 * so I'm not sure what the magic was.
 */
void ActionButtons::layout(juce::Rectangle<int> bounds)
{
    int availableWidth = bounds.getWidth();
    int topOffset = 0;
    int leftOffset = 0;
    // todo: make this configurable ?
    int buttonHeight = 25;

    // before we start, bound the parent with all of the available size
    // so the button mouse listeners have something to work with
    // note that if this calls resized() it will be ignored
    // hmm, this didn't work
    //setBounds(bounds);

    int rowStart = 0;
    for (int i = 0 ; i < buttons.size() ; i++) {
        ActionButton* b = buttons[i];
        int minWidth = b->getPreferredWidth(buttonHeight);
        b->setSize(minWidth, buttonHeight);

        // leave a gap
        if (leftOffset > 0) leftOffset += 2;
        
        if (leftOffset + minWidth > availableWidth) {
            // center the current row and start a new one
            // I suppose we should handle the edge case where
            // the available width isn't enough for a single button
            // but just let it truncate
            centerRow(rowStart, i, leftOffset, availableWidth);
            leftOffset = 0;
            topOffset += buttonHeight;
            rowStart = i;
        }
        b->setTopLeftPosition(leftOffset, topOffset);
        leftOffset += minWidth;
    }

    // close off the last row
    if (leftOffset > 0) {
        centerRow(rowStart, buttons.size(), leftOffset, availableWidth);
        topOffset += buttonHeight;
    }

    // now adjust our height to only use what we needed
    setSize(availableWidth, topOffset);

    JuceUtil::dumpComponent("ActionButtons::layout", this);
}

/**
 * Hack trying to work around the unresponsive buttons problem.
 * Not used right now but keep around in case we have to resurect that.
 */
int ActionButtons::getPreferredHeight(juce::Rectangle<int> bounds)
{
    layout(bounds);
    return getHeight();
}

void ActionButtons::centerRow(int start, int end, int rowWidth, int availableWidth)
{
    int centerOffset = (availableWidth - rowWidth) / 2;
    for (int i = start ; i < end ; i++) {
        ActionButton* b = buttons[i];
        // so we have a getX but not a setX, makes perfect sense
        b->setTopLeftPosition(b->getX() + centerOffset, b->getY());
    }
}

/**
 * This is an unusual component in that the parent is expected to
 * call layout() rather than just setSize() in it's resized() method.
 * Since that does the work, when we eventually get to Juce calling
 * our resized() we don't have to do anything.
 *
 * See comments for layout() about a subtle mouse sensitivity problem
 * that seemed to have introduced.  For a time I had to call layout()
 * again here after setting our size, but then it started working.
 */
void ActionButtons::resized()
{
    // shouldn't get here, parent is supposed to call layout() instead
    //layout(getLocalBounds());
}

void ActionButtons::paint(juce::Graphics& g)
{
    // draw a border for testing layout
    //g.setColour(juce::Colours::red);
    //g.drawRect(getLocalBounds(), 1);
}

/**
 * Rather than having each ActionButton propagate the UIAction we'll
 * forward all the clicks up here and do it.
 */
void ActionButtons::buttonClicked(juce::Button* src)
{
    // todo: figure out of dynamic_cast has any performance implications
    ActionButton* ab = (ActionButton*)src;
    display->doAction(ab->getAction());
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

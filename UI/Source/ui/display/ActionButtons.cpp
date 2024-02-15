/*
 * Arranges a configurable list of ActionButtons in a row with auto
 * wrapping and sizing.
 *
 * Reads the list of buttons to display from UIConfig
 * Resolves UIButton bindings to Actions.
 */

#include <JuceHeader.h>

#include "../../model/UIConfig.h"
#include "ActionButton.h"
#include "ActionButtons.h"

ActionButtons::ActionButtons()
{
    setName("ActionButtons");
}

ActionButtons::~ActionButtons()
{
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
            b->addListener(this);
            // todo: resolve Action
            addAndMakeVisible(b);
            buttons.add(b);
		}
	}
}

/**
 * Format a name for the button.  Normally this will be a Function
 * name but for more complex binding with arguments we add some
 * annotations.
 *
 * todo: standardize this format and use it consistently
 * Not storing the target type yet
 */
juce::String ActionButtons::formatButtonName(UIButton *src)
{
    juce::String name = juce::String(src->getName());
    if (src->getArguments() != nullptr) {
        // formatting here could be more complicated with
        // normalization and capitalization
        name += "(";
        name += src->getArguments();
        name += ")";
    }
    return name;
}

/**
 * Size oursleves so we can fit all the buttons.
 * Buttons are of variable width depending on their text since
 * some, especially with arguments, might be long.  Continue on one
 * line wrapping when we get to the end of the available space.
 * Expected to be called by the parent's resized() method when
 * it knows the available width instead of calling resized.
 *
 * Alternately could have a getPreferredHeight and a normal
 * resized() but we would have to do the same positioning calculations
 * in both so why bother.
 */
#if 0
void ActionButtons::layoutEasy(juce::Rectangle<int> parentBounds)
{
    int availableWidth = parentBounds.getWidth();
    int topOffset = 0;
    int leftOffset = 0;
    // todo: make this configurable ?
    int buttonHeight = 25;

    for (int i = 0 ; i < buttons.size() ; i++) {
        ActionButton* b = buttons[i];
        int minWidth = b->getPreferredWidth(buttonHeight);
        b->setSize(minWidth, buttonHeight);

        // leave a gap
        if (leftOffset > 0) leftOffset += 2;
        
        if (leftOffset + minWidth > availableWidth) {
            // new row
            // I suppose we should handle the edge case where
            // the available width isn't enough for a single button
            // but just let it truncate
            leftOffset = 0;
            topOffset += buttonHeight;
        }
        b->setTopLeftPosition(leftOffset, topOffset);
        leftOffset += minWidth;
    }

    // now set our bounds to enclose the buttons
    int totalHeight = topOffset;
    // if we used this row
    if (leftOffset > 0) totalHeight += buttonHeight;
    setSize(availableWidth, totalHeight);
}
#endif

/**
 * Variant that centers the buttons within each row.
 * It would be easier in some ways to put each row in a
 * wrapper component and than center that.
 */
void ActionButtons::layout(juce::Rectangle<int> bounds)
{
    int availableWidth = bounds.getWidth();
    int topOffset = 0;
    int leftOffset = 0;
    // todo: make this configurable ?
    int buttonHeight = 25;

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

    // now set our bounds to enclose the buttons
    setBounds(bounds.getX(), bounds.getY(), bounds.getWidth(), topOffset);
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

void ActionButtons::resized()
{
    // shouldn't get here, parent is supposed to call layout() instead
}

void ActionButtons::paint(juce::Graphics& g)
{
    // draw a border for testing layout
    //g.setColour(juce::Colours::red);
    //g.drawRect(getLocalBounds(), 1);
}

void ActionButtons::buttonClicked(juce::Button* src)
{
    // todo: figure out of dynamic_cast has any performance implications
    ActionButton* ab = (ActionButton*)src;
    ab->execute();
}
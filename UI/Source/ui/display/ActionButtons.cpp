/*
 * Arranges a configurable list of ActionButtons in a row with auto
 * wrapping and sizing.
 *
 * Reads the list of buttons to display from UIConfig
 */

#include <JuceHeader.h>

#include "../../Supervisor.h"
#include "../../model/MobiusConfig.h"
#include "../../model/DynamicConfig.h"

#include "../JuceUtil.h"

#include "ActionButton.h"
#include "ActionButtons.h"
#include "MobiusDisplay.h"

ActionButtons::ActionButtons(MobiusDisplay* argDisplay)
{
    setName("ActionButtons");
    display = argDisplay;

    // this one is unusual in that we want to know about 
    // DynamicConfig changes to adjust the buttons
    Supervisor::Instance->addDynamicConfigListener(this);
}

ActionButtons::~ActionButtons()
{
    Supervisor::Instance->removeDynamicConfigListener(this);
}

void ActionButtons::add(ActionButton* b)
{
    addAndMakeVisible(b);
    b->addListener(this);
}

/**
 * Read the button bindings from the MobiusConfig and build the list
 * of ActionButtons.  This can be called after the component is
 * constructed to add/remove buttons.
 *
 * Formerly implemented against UIConfig.buttons
 * Now get them from the BindingSet in MobiusConfig
 */
void ActionButtons::configure(MobiusConfig* config)
{
    buildButtons(config);
}

void ActionButtons::addButton(ActionButton* b)
{
    b->addListener(this);
    addAndMakeVisible(b);
    buttons.add(b);
}

/**
 * Note that this deletes the button.
 * buttons.remove will call the destructor.
 */
void ActionButtons::removeButton(ActionButton* b)
{
    b->removeListener(this);
    removeChildComponent(b);
    // yuno remove(b) ?
    int index = buttons.indexOf(b);
    if (index >= 0)
      buttons.remove(index);
}

/**
 * We've got two ways to build out the button list, configure()
 * which pulls them from the MobiusConfig and dynamicConfigChanged()
 * which pulls them from the DynamicConfig.
 *
 * Both sources need to be merged.
 *
 * Here we do just the MobiusConfig buttons and preserve the dynamic
 * buttons and dynamicConfigChanged will do the reverse.
 */
void ActionButtons::buildButtons(MobiusConfig* config)
{
    // find the things to keep
    juce::Array<ActionButton*> dynamicButtons;
    for (int i = 0 ; i < buttons.size() ; i++) {
        ActionButton* b = buttons[i];
        if (b->isDynamic()) {
            dynamicButtons.add(b);
        }
    }

    // remove everything from the parent component
    for (int i = 0 ; i < buttons.size() ; i++) {
      removeChildComponent(buttons[i]);
    }

    // remove the ones we want to keep from the owned button list
    for (int i = 0 ; i < dynamicButtons.size() ; i++) {
        ActionButton* b = dynamicButtons[i];
        buttons.removeAndReturn(buttons.indexOf(b));
    }

    // delete what's left
    buttons.clear();

    // add the MobiusConfig buttons
    BindingSet* baseBindings = config->getBindingSets();
    if (baseBindings != nullptr) {
        Binding* binding = baseBindings->getBindings();
        while (binding != nullptr) {
            if (binding->trigger == TriggerUI) {
                ActionButton* b = new ActionButton(binding);
                addButton(b);
            }
            binding = binding->getNext();
        }
    }
    
    // and restore the dynamic buttons
    for (int i = 0 ; i < dynamicButtons.size() ; i++) {
        addButton(dynamicButtons[i]);
    }
}

/**
 * Rebuild the button list after the DynamicConfig changed.
 * Here we preserve the MobiusConfig buttons, and rebuild
 * the dynamic buttons.
 *
 * If the button was manually configured then leave it in place.
 * Otherwise completely rebuild the dynamic button list at the end.
 * Order will be random which is enough for now.
 */
void ActionButtons::dynamicConfigChanged(DynamicConfig* config)
{
    bool changes = false;
    
    // don't like iteration index assumptions so build
    // an intermediate list
    juce::StringArray thingsToKeep;
    juce::Array<ActionButton*> thingsToRemove;
    
    for (int i = 0 ; i < buttons.size() ; i++) {
        ActionButton* b = buttons[i];
        if (b->isDynamic()) {
            thingsToRemove.add(b);
        }
        else {
            thingsToKeep.add(b->getButtonText());
        }
    }
    
    // remove all the current dynamic buttons
    for (int i = 0 ; i < thingsToRemove.size() ; i++) {
        ActionButton* b = thingsToRemove[i];
        removeButton(b);
        // removeButton uses button.remove which deletes it
        changes = true;
    }

    // now add back the ones that aren't already there
    juce::OwnedArray<DynamicAction>* actions = config->getActions();
    for (int i = 0 ; i < actions->size() ; i++) {
        DynamicAction* action = (*actions)[i];
        if (action->button) {
            // did it we have it manually configured?
            if (!thingsToKeep.contains(action->name)) {
                ActionButton* b = new ActionButton(action);
                addButton(b);
                changes = true;
            }
        }
    }

    // okay, this is a weird one
    // it is not obvious to me how to do dynamic child component sizing with
    // the Juce way of thinking which is normally top down
    // we know we're contained in MobiusDisplay whose resized() method
    // will call down to our layout() so that should do the needful though
    // it's not good encapsulation
    if (changes) {
        display->resized();
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

/*
 * An extension of juce::Button to associate the visible
 * button with a Mobius Action.  These are arranged in a
 * configurable row by ActionButtons.
 * 
 * juce::ApplicationCommandManager feels a bit like our
 * ActionDispatcher, might be able to use that here for button/key
 * bindings but not sure how much that buys us.
 *
 * Smiple extending TextButton creates a nice usable button but attempting
 * to control the colors was futile.
 * 
 * TextButton has these ColourIds
 *   buttonColourId, buttonOnColourId, textColourOffId, textColourOnId
 *
 * Colors are fucking weird
 * Without setting anything background color is just slighly non-black and turns grey when pressed
 * textColourOffId sets the text color, but textColourOnId does nothing, probably
 * because this is not a toggle button.  It also turns a lighter grey when the mouse is over it.
 * So there are three colors in play here.
 *
 * buttonColourId only displays a lightened version of the color initially
 * When you click it it turns the desired color and stays there but clicking a second
 * button dims it and darkens the other.  It is behaving like a radio group.  If the window
 * loses focus, it dims, then goes back when it regains focus.
 *
 * buttonOnColourId does nothing if this is not a toggle button.
 * 
 * So there is a whole lot of automatic coloring UNLESS you set buttonOnColorId then
 * it starts behaving like a radio.
 *
 * To make this look like the old ones will probably have to override the draw methods
 * or use one of the more basic button classes.
 *
 * Calling setToggleState(true) even with buttonColourId reverts it back to the default
 * colors and change behavior.
 *
 * If you setToggleable(true) the background defaults to shades of grey, textColourOffId
 * is the default text color, and clicking does nothing.
 * setClickingToggleState does what you would expect except the text color turns white if you don't set it.
 * 
 * Can override the paintButton method which has this interesting comment:
 *   shouldDrawButtonAsHighlighted true if the button is either in the 'over' or 'down' state
 *   so there is built-in mouse tracking that changes colors when you are over it.
 *
 * Fuck it, override the paintButton method
 */

#include <JuceHeader.h>

#include "../../util/Trace.h"

#include "../../model/UIConfig.h"
#include "../../model/UIAction.h"
#include "Colors.h"
#include "ActionButton.h"

/**
 * Initialize the button to trigger an action defined by UIButton.
 * We do not retain a refernce to the UIButton.
 */
ActionButton::ActionButton(UIButton* uib)
{
    setName("ActionButton");
    
    // don't wait for mouse up
    setTriggeredOnMouseDown(true);
    setButtonText(formatButtonName(uib));

    initAction(uib);
}

ActionButton::~ActionButton()
{
}

/**
 * Calculate a good width for this button given a height.
 * 
 * TextButton does not appear to have a Font, it defaults to something.
 * 
 * Button doesn't seem to have a Font, it will apparently be an
 * unknown percentage of the given height.  So calling getStringWidth
 * won't be entirely accurate without knowing that percentage.  If we
 * use the full height the Font will be slightly larger than what will
 * end up being used which is okay for getting a width.  It will be a little
 * wider than necessary.
 *
 * Alternately TextButton has changeWidthToFitText which
 * "Resizes the button's width to fit neatly around its current text,
 * and gives it the specified height".  Let's start there.
 *
 * Now that we overrid the paintButton method we have more control over this.
 */
int ActionButton::getPreferredWidth(int height)
{
    // we're control Font now so don't need this
    // changeWidthToFitText(height);
    juce::Font font = juce::Font(height * 0.75);
    int minWidth = font.getStringWidth(getButtonText());
    // add some padding around the edges
    minWidth += 20;
    return minWidth;
}

/**
 * Draw buttons the old way.
 */
void ActionButton::paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown)
{
    if (shouldDrawButtonAsHighlighted) {
        if (shouldDrawButtonAsDown) {
            paintButton(g, juce::Colour(MobiusBlue), juce::Colour(MobiusRed));
        }
        else {
            paintButton(g, juce::Colour(MobiusBlue), juce::Colour(MobiusYellow));
        }
    }
    else {
        paintButton(g, juce::Colour(MobiusBlue), juce::Colours::black);
    }
}    

/**
 * Paint a basic rounded button with text in predictable colors
 * Since we're overriding paint, use a font that is .75 of the button height
 * Unclear what cornerSize means, try things until it looks right
 */
void ActionButton::paintButton(juce::Graphics& g, juce::Colour background, juce::Colour text)
{
    g.setColour(background);
    // can't fucking use Rectangle<int>
    juce::Rectangle<int> bounds = getLocalBounds();
    g.fillRoundedRectangle((float)bounds.getX(), (float)bounds.getY(),
                           (float)bounds.getWidth(), (float)bounds.getHeight(), 8.0f);
    g.setColour(text);
    g.setFont(juce::Font(getHeight() * 0.75f));
    g.drawText(getButtonText(), getLocalBounds(), juce::Justification::centred);
}    

//////////////////////////////////////////////////////////////////////
//
// Action
//
////////////////////////////////////////////////////////////////////////

/**
 * Format a name for the button.  Normally this will be a Function
 * name but for more complex binding with arguments we add some
 * annotations.
 *
 * todo: standardize this format and use it consistently
 * Not storing the target type yet
 * Probably should be on Binding for consistency in logging.
 */
juce::String ActionButton::formatButtonName(UIButton *src)
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
 * Initialize the action we request when clicked.
 * Button bindings used to be stored in the Binding list
 * but I went back to maintaining UIButton objects in UI
 * config.  Since most of the action parsing logic is related
 * to Binding, convert the UIButton contents to a Binding
 * to initialize the UIAction.
 * 
 */
void ActionButton::initAction(UIButton* src)
{
    Binding binding;

    binding.setTrigger(TriggerUI);
    // todo: could support Momentary for SUS functions
    // comments say this was used only for OSC but I guess
    // it applies to this now too
    binding.setTriggerMode(TriggerModeOnce);
    // buttons do not have a trigger value, channel

    // the new UIButton model does not have a target type yet
    // geez, I'm starting to think going back to a common Binding
    // model is better
    binding.setTarget(TargetFunction);
    binding.setName(src->getName());
    binding.setArgs(src->getArguments());
                    
    action.init(&binding);
}

UIAction* ActionButton::getAction()
{
    return &action;
}


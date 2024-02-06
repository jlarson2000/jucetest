/**
 * An object model for form fields that are rendered as Juce components.
 * 
 * DESIGN NOTES
 *
 * Since Fields are rendered using Juce, we will use Juce collections
 * here too instead of std:: collections where appropriate.
 *
 * Field could either inherit directly from Component or it could be
 * a parallel model that generates Components.  Tradeoffs are unclear
 * at this time, start with them being Components.
 * 
 * I started using native Juce label attachements for the field label
 * but I think it would be somewhat easier to just manage these ourselves
 * now that I understand it more.  Some notes on positining issues for
 * components with attached labels:
 * 
 * When you attach a label to a component it is displayed to the left or top of the component.
 * You need to position the attached component so that the label had enough room
 * to display on the left or above, just attaching it does not create some sort of wrapper
 * component that understands this.  It follows the component around but is it's own
 * component that just happens to get bounds automatically from the attached component,
 * I guess filling whatever space is available.
 * Examples show giving it Justification::right but that doesn't seem to matter.
 *
 * Forum comments from 2021:
 *  It looks like juce::Label::attachToComponent is used in a few places in the codebase by other JUCE
 * components, so it’s likely it was only added to suit the needs of those other components.
 *
 * You could create a simple LabeledComponent that can be given a component and puts a label below it.
 * It also removes the need for you to manage a label and the corresponding component, you can just manage
 * the one component which will make your code simpler and cleaner.
 *
 * Yeah, there is very little magic here, and it might be easier to manage them independently.
 *
 * Sizing notes:
 *
 * Since rendered compoennts are all lightweight unless we're using native look and feel
 * they don't seem to have any specified preferred size, they'll adapt to the size we give them.
 * We'll guess some reasonable values.
 *
 */

#include <string>
#include <sstream>

#include <JuceHeader.h>

#include "../util/Trace.h"
#include "Field.h"

//////////////////////////////////////////////////////////////////////
//
// Field
//
//////////////////////////////////////////////////////////////////////

/**
 * Fields have a name property which conflicts with Component.
 * Could use getFieldName instead.
 */
Field::Field(juce::String argName, juce::String argDisplayName, Field::Type argType)
{
    juce::Component::setName("Field");  // clas sname for debugging
    name = argName;
    displayName = argDisplayName;
    type = argType;
}

Field::~Field()
{
}

void Field::setAllowedValues(const char** arg)
{
    allowedValues = juce::StringArray(arg);
}

void Field::setAllowedValues(juce::StringArray& src)
{
    allowedValues = src;
}

void Field::setAllowedValueLabels(const char** arg)
{
    allowedValueLabels = juce::StringArray(arg);
}

void Field::setAllowedValueLabels(juce::StringArray& src)
{
    allowedValueLabels = src;
}

/**
 * Once all properties of the field are specified, render
 * it with appropriate juce components and calculate the
 * desired display size.
 */
void Field::render()
{
    // TODO: use JLabel and stop using attachments
    addAndMakeVisible(label);
    label.setText (getDisplayableName(), juce::dontSendNotification);
    label.setFont (juce::Font (16.0f, juce::Font::bold));
    label.setColour (juce::Label::textColourId, juce::Colours::orange);
    label.setJustificationType (juce::Justification::left);

    // render methods will set renderType and renderer
    switch (type) {
        case Type::Integer: { renderInt(); } break;
        case Type::String: { renderString(); } break;
        case Type::Boolean: { renderBool(); } break;
    }

    if (renderer != nullptr) {
        addAndMakeVisible(renderer);
        label.attachToComponent(renderer, true);
    }

    autoSize();
}

void Field::renderString()
{
    // most sizing will be derived from the label font
    juce::Font font = label.getFont();
    int charHeight = font.getHeight();
    // for width it is unclear, need to dig into Juce source
    // I think it is a multiple of kerning, but unclear how to get that
    int charWidth = font.getStringWidth(juce::String("M"));

    if (allowedValues.isEmpty()) {
        renderType = RenderType::Text;
        renderer = &textbox;
        
        textbox.setEditable(true);
        textbox.setColour (juce::Label::backgroundColourId, juce::Colours::lightblue);
        // from the demo
        // textbox.onTextChange = [this] { uppercaseText.setText (inputText.getText().toUpperCase(), juce::dontSendNotification); };

        // let's start with font size plus a little padding
        int chars = widthUnits;
        // if not specified pick something big enough for a typical name
        if (chars == 0) 
          chars = 20;

        // add a little padding on top and bottom
        textbox.setSize(chars * charWidth, charHeight + 4);
    }
    else if (!multi) {
        renderType = RenderType::Combo;
        renderer = &combobox;
        
        int maxChars = 0;
        for (int i = 0 ; i < allowedValues.size() ; i++) {
            juce::String s = allowedValues[i];
            if (s.length() > maxChars)
              maxChars = s.length();
            // note that item ids must be non-zero
            combobox.addItem(s, i + 1);
        }
        combobox.setSelectedId(1);

        // figure out how to make this transparent
        combobox.setColour(juce::ComboBox::ColourIds::backgroundColourId, juce::Colours::white);
        combobox.setColour(juce::ComboBox::ColourIds::textColourId, juce::Colours::black);
        combobox.setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colours::black);
        // "base color for the button", what's this?
        // combobox.setColour(juce::ComboBox::ColourIds::buttonColourId, juce::Colours::black);
        combobox.setColour(juce::ComboBox::ColourIds::arrowColourId, juce::Colours::black);
        combobox.setColour(juce::ComboBox::ColourIds::focusedOutlineColourId, juce::Colours::red);

        combobox.setSize(maxChars * charWidth, charHeight + 4);
    }
    else {
        renderType = RenderType::List;
        renderer = &listbox;
        //listbox.setListValues(allowedValues);
        listbox.setValues(allowedValues);

        int maxChars = 0;
        for (int i = 0 ; i < allowedValues.size() ; i++) {
            juce::String s = allowedValues[i];
            if (s.length() > maxChars)
              maxChars = s.length();
        }

        // if they bothered to specify a width use that
        if (widthUnits > maxChars)
          maxChars = widthUnits;

        // might want to give these more air
        // limit the number we can display so they scroll
        int rows = allowedValues.size();
        if (rows > 4)
          rows = 4;
        listbox.setSize(maxChars * charWidth, (charHeight * rows) + 4);
    }
}

void Field::renderInt()
{
    // most sizing will be derived from the label font
    juce::Font font = label.getFont();
    int charHeight = font.getHeight();
    // for width it is unclear, need to dig into Juce source
    // I think it is a multiple of kerning, but unclear how to get that
    int charWidth = font.getStringWidth(juce::String("M"));

    if (min == 0 && max == 0) {
        renderType = RenderType::Text;
        renderer = &textbox;
        
        textbox.setEditable(true);
        // make them look different for testing
        textbox.setColour (juce::Label::backgroundColourId, juce::Colours::darkblue);

        int chars = widthUnits;
        // if not specified pick something big enough for a typical name
        if (chars == 0) 
          chars = 8;

        // add a little padding on top and bottom
        textbox.setSize(chars * charWidth, charHeight + 4);
    }
    else {
        // allow this to be preset
        if (renderType != RenderType::Rotary)
          renderType = RenderType::Slider;
        renderer = &slider;
        slider.setRange((double)min, (double)max, 1);
        slider.setColour(juce::Slider::ColourIds::textBoxTextColourId, juce::Colours::black);
        if (renderType == RenderType::Rotary) {
            slider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
        }

        if (renderType == RenderType::Rotary) {
            // make these big enough to be usable
            // these are weird and need more thought
            // like regular sliders they have a value box on the left, then the rotary
            // they need quite a bit of height to make them usable
            // what is interesting is that the compoents seem to be centering themselves
            // within the allowed height, not sure why
            int boxWidth = (widthUnits > 0) ? widthUnits : 40;
            int totalWidth = (charWidth * 8) + boxWidth;
            slider.setSize(totalWidth, boxWidth);
        }
        else {
            // sliders could adjust their width based on the range of values?
            // there are two parts to this, the box on the left that displays the value
            // and the slider, default box width seems to be about 8-10 characters
            // make the slider 100 for now
            // should be basing height on font height
            int width =  (charWidth * 8) + 200;
            slider.setSize(width, 20);
        }
    }
}

void Field::renderBool()
{
    renderType = RenderType::Check;
    renderer = &checkbox;
    
    addAndMakeVisible(checkbox);
    // could use this to put the text within the button area and not use labels
    //checkbox.setButtonText("foo");
    checkbox.setColour(juce::ToggleButton::ColourIds::textColourId, juce::Colours::black);
    checkbox.setColour(juce::ToggleButton::ColourIds::tickColourId, juce::Colours::red);
    checkbox.setColour(juce::ToggleButton::ColourIds::tickDisabledColourId, juce::Colours::black);

    // make it big enough to be useful, might want to adapt to font size?
    checkbox.setSize(20, 20);
}

/**
 * Calculate the minimum bounds for this field.
 */
void Field::autoSize()
{
    // start with the label
    juce::Font font = label.getFont();
    int width = font.getStringWidth(label.getText());
    int height = font.getHeight();

    // this seems to be not big enough, Juce did "..."
    width += 10;
    label.setSize(width, height);

    // assume renderer has left a suitable size
    if (renderer != nullptr) {
        width += renderer->getWidth();
        int rheight = renderer->getHeight();
        if (rheight > height)
          height = rheight;
    }

    setSize(width, height);
}

/**
 * Layout the subcomponents.  Assumes autoSize has already been called.
 * All we need to do is adjust the position of the renderer relative
 * to the label.
 */
void Field::resized()
{
    // do them all the same way for now, may want more control
    if (renderer != nullptr) {
        renderer->setTopLeftPosition(label.getWidth(), 0);
    }
}

void Field::paint(juce::Graphics& g)
{
    // temporarily border it for testing
    g.setColour(juce::Colours::red);
    g.drawRect(getLocalBounds(), 1);
}

/**
 * Set the value of a field and propagate it to the components.
 */
void Field::setValue(juce::var argValue)
{
    value = argValue;
    // todo: udpate the field
}

/**
 * Pull the value out of the components
 */
void Field::refreshValue()
{
    // todo: component magic to update value
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

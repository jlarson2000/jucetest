/**
 * An object model for form fields that are rendered as Juce components.
 * 
 * DESIGN NOTES
 *
 * Field could either inherit directly from Component or it could be
 * a parallel model that generates Components.  Tradeoffs are unclear
 * at this time, start with them being Components.  The introduction of managed/unmanaged
 * moves toward making these not Components but simply adding things to the parent.
 * 
 * Rendering
 *
 * Fields have deferred rendering as Juce components.  You create a Field then
 * set the various display properties then call render() to construct the necessary
 * Juce components that implement it.  Rendering will calculate and set the initial
 * minimum size.  This size is normally left alone.  
 *
 * Juce label attachments vs. managed labels
 *
 * Juce has some basic mechanisms for attaching a label to a component
 * and following it around.  

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
 * This may be enough, but I'm kind of liking having Field manage the positioning
 * directly rather than using attachments.  This fits better with the notion of "unmanaged"
 * labels which let's the container own the label.
 *
 * Managed vs. Unmanaged labels
 *
 * A managed label is when the label component is a child of the Field and the field
 * is responsible for positining it.  The label still sets it's own size.  The bounds for the field
 * must be large enough to accomodate the label.  To support label positioning and justification
 * the parent must give the field information about where the label is to be displayed and how
 * to position it.
 *
 * An unmanaged label is when the label component is a child of the parent and the field
 * does not position it.  The parent handles all label positioning.
 *
 * Managed is the default.
 * 
 * Sizing notes:
 *
 * Since rendered compoennts are all lightweight unless we're using native look and feel
 * they don't seem to have any specified preferred size, they'll adapt to the size we give them.
 * We'll guess some reasonable values.
 *
 * Parent size overrides:
 *
 * Fields will start out with their preferred minimum size.  If the parent gives it a larger
 * size, we could try to center within that area (assuming right justified labels). This won't
 * happen yet.
 *
 * Component required sizing
 *
 * It seems to be counter to the Juce way of thinking to let things size themselves
 * they always want th parent to size it and make it big enough for the unknown amount
 * 
 * From the forums...
 * Minimum widths:
 *   TextButton : getStringWidth(button.text) + button.height[/]
 *   ToggleButton : getStringWidth(button.text) + min(24, button.height) + 8[/]
 *   TextEditor : getStringWidth(text.largestWordcontent) + leftIndent (default 4px) + 1 + borderSize.left (default 1px) + borderSize.right (default 1px) => default sum is 7px [/]
 *    ComboBox : same as TextEditor[/][/list]
 *
 * Checkboxes (toggle buttons) are weird.
 * There is always some padding on the left but none on the right.
 * There seems to be no way to control this other than jiggering the x position provided you're
 * within a component with enough space.  If you just want to have a Checkbox by itself
 * not wrapped in anything you get the pad unless maybe a negative x position would work.
 * Probably have to do a custom button with it's own paint.  Someday...
 */

#include <string>
#include <sstream>

#include <JuceHeader.h>

#include "../util/Trace.h"
#include "JuceUtil.h"
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
    juce::Component::setName("Field");  // class sname for debugging
    name = argName;
    displayName = argDisplayName;
    type = argType;
    initLabel();
}

Field::Field(const char* argName, Field::Type argType)
{
    juce::Component::setName("Field");  // class sname for debugging
    name = juce::String(argName);
    type = argType;
    initLabel();
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

void Field::initLabel()
{
    // Set up the label with reasonable defaults that can
    // be overridden after construction but before rendering

    // TODO: use JLabel and stop using attachments
    label.setText (getDisplayableName(), juce::dontSendNotification);
    label.setFont (juce::Font (16.0f, juce::Font::bold));
    label.setColour (juce::Label::textColourId, juce::Colours::orange);
    // the default is centeredLeft, I think this is used when the
    // label is given bounds larger than necessary to contain the font text
    label.setJustificationType (juce::Justification::left);
}

//////////////////////////////////////////////////////////////////////
//
// Rendering
//
//////////////////////////////////////////////////////////////////////

/**
 * Once all properties of the field are specified, render
 * it with appropriate juce components and calculate initial
 * minimum display size.
 */
void Field::render()
{
    renderLabel();

    // render methods will set renderType and renderer
    switch (type) {
        case Type::Integer: { renderInt(); } break;
        case Type::String: { renderString(); } break;
        case Type::Boolean: { renderBool(); } break;
    }

    if (renderer != nullptr) {
        addAndMakeVisible(renderer);
        // make attachment optional
        if (!unmanagedLabel)
          label.attachToComponent(renderer, true);
    }

    // note well: label.attachToComponent seems to clear out the width
    // I guess because it tries to size based on the current size
    // of the component which is too small at this point
    // so we need to render and set it's size after attaching.
    // still don't fully understand how attachment works, but
    // we're likely to stop using that anyway
    renderLabel();

    // set the initial value if we have one
    if (!value.isVoid())
      loadValue();

    // calculate bounds using both the label and the renderer
    juce::Rectangle<int> size = getMinimumSize();
    setSize(size.getWidth(), size.getHeight());
}

/**
 * Label properties have been set by now.
 * Here calculate minimum size and add it as a child if necessary.
 */
void Field::renderLabel()
{
    juce::Font font = label.getFont();
    int width = font.getStringWidth(label.getText());
    int height = font.getHeight();

    // this seems to be not big enough, Juce did "..."
    width += 10;
    label.setSize(width, height);

    // label is a child comonent unless the parent wants to manage it
    if (!unmanagedLabel) {
        addAndMakeVisible(label);
    }
}

/**
 * Render a string field as either a text field, a combo box, or a select list
 * The size of the internal components will be set.
 * TODO: If we allow size overrides after construction, this might
 * need to carry forward to these, particularly the select list which we could
 * make taller.  Hmm, some forms might want a bigger select list to auto-size.
 */
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
        listbox.setValues(allowedValues);
        listbox.setValueLabels(allowedValueLabels);

        juce::StringArray& displayValues = allowedValues;
        if (allowedValueLabels.size() > 0)
          displayValues = allowedValueLabels;

        int maxChars = 0;
        for (int i = 0 ; i < displayValues.size() ; i++) {
            juce::String s = displayValues[i];
            if (s.length() > maxChars)
              maxChars = s.length();
        }

        // if they bothered to specify a width use that
        if (widthUnits > maxChars)
          maxChars = widthUnits;

        // the number of lines to display before scrolling starts
        // may be explicitly specified
        int rows = heightUnits;
        if (rows == 0) {
            rows = allowedValues.size();
            // constrain this for long lists, could make the max configurable
            if (rows > 4)
              rows = 4;
        }
        
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
    // seems to have a little padding on the left, is this what
    // the setConnected edge flags do?
    // ugh, button sizing is weird
    // 10,10 is too small and the bounds it needs seems to be more than that and it
    // gets clipped on the right
    // these don't do anything, sounds like they're only hints for LookAndFeel
    int edgeFlags =
        juce::Button::ConnectedEdgeFlags::ConnectedOnLeft |
        juce::Button::ConnectedEdgeFlags::ConnectedOnRight |
        juce::Button::ConnectedEdgeFlags::ConnectedOnTop |
        juce::Button::ConnectedEdgeFlags::ConnectedOnBottom;
    checkbox.setConnectedEdges(edgeFlags);
    
    checkbox.setSize(20, 20);

    // buttons are weird
    // without a label and just a 20x20 component there are a few pixels of padding
    // on the left and the right edge of the button overlaps with the test bounding rectangle
    // we draw around the field that's fine on the right, but why the left?
    // don't see a way to control that

}

/**
 * Calculate the minimum bounds for this field after rendering.
 *
 * Don't really need to be using Rectangle here since
 * we only care about width and height, but using Point
 * for x,y felt weird.  
 */
juce::Rectangle<int> Field::getMinimumSize()
{
    juce::Rectangle<int> size;
    int totalWidth = 0;
    int maxHeight = 0;

    // start with the attached label
    if (!unmanagedLabel) {
        totalWidth = label.getWidth();
        maxHeight = label.getHeight();
    }
    
    // assume renderer has left a suitable size
    if (renderer != nullptr) {
        totalWidth += renderer->getWidth();
        int rheight = renderer->getHeight();
        if (rheight > maxHeight)
          maxHeight = rheight;
    }

    size.setWidth(totalWidth);
    size.setHeight(maxHeight);

    return size;
}

int Field::getLabelWidth()
{
    return label.getWidth();
}

int Field::getRenderWidth()
{
    return renderer->getWidth();
}

//////////////////////////////////////////////////////////////////////
//
// Layout
//
//////////////////////////////////////////////////////////////////////

/**
 * Layout the subcomponents.
 * If we have a managed label, adjust the position of the renderer
 * relative to the label.  Width and Height have already been set
 * for both subcomponents.
 */
void Field::resized()
{
    // do them all the same way for now, may want more control
    if (renderer != nullptr) {
        int offset = 0;

        if (!unmanagedLabel) {
            // TODO: need more justification options besides
            // left adjacent
            offset += label.getWidth();
        }
        
        renderer->setTopLeftPosition(offset, 0);
    }
}

void Field::paint(juce::Graphics& g)
{
    // temporarily border it for testing
    g.setColour(juce::Colours::red);
    g.drawRect(getLocalBounds(), 1);
}

//////////////////////////////////////////////////////////////////////
//
// Value Management
//
//////////////////////////////////////////////////////////////////////


/**
 * Set the value of a field and propagate it to the components.
 * If the field has not been rendered yet, we just save it to the
 * var member until later.  This makes it a little easier to
 * build fields without needing to understand the order dependency.
 */
void Field::setValue(juce::var argValue)
{
    value = argValue;
    loadValue();
}

/**
 * Copy the intermediate value into the component after rendering.
 */
void Field::loadValue()
{
    // what will the components do with a void value?

    if (renderer == &textbox) {
        // See this discussion on why operator String() doesn't work
        // if you add (juce::String) cast, just pass it without a cast, subtle
        // https://forum.juce.com/t/cant-get-var-operator-string-to-compile/36627
        textbox.setText(value, juce::dontSendNotification);
    }
    else if (renderer == &checkbox) {
        checkbox.setToggleState((bool)value, juce::dontSendNotification);
    }
    else if (renderer == &combobox) {
        // should only get here if we had allowedValues
        int itemId = 0;
        for (int i = 0 ; i < allowedValues.size() ; i++) {
            juce::String s = allowedValues[i];
            // can't do (s == value) or (s == (juce::String)value)
            if (s == value.toString()) {
                itemId = i + 1;
            }
        }
        
        combobox.setSelectedId(itemId);
    }
    else if (renderer == &slider) {
        slider.setValue((double)value);
    }
    else if (renderer == &listbox) {
        // compare values value to allowedValues and
        // set all the included values
        juce::String csv = value.toString();
        // not supporting display names yet
        // don't know if Juce has any CSV utilities
        juce::StringArray values;
        JuceUtil::CsvToArray(csv, values);
        listbox.setSelectedValues(values);
    }
    else {
        // Field hasn't been rendered yet
    }
}

/**
 * Return the current field value.
 * If the field has been rendered get it from the component.
 */
juce::var Field::getValue()
{
    if (renderer == &textbox) {
        if (type == Type::Integer) {
            juce::String s = textbox.getText();
            value = s.getIntValue();
        }
        else {
            value = textbox.getText();
        }
    }
    else if (renderer == &checkbox) {
        value = checkbox.getToggleState();
    }
    else if (renderer == &combobox) {
        int selected = combobox.getSelectedId();
        // I suppose this could have the concept of nothing
        // selected, we don't support that, just auto select
        // the first one
        if (selected > 0)
          selected--;
        value = allowedValues[selected];
    }
    else if (renderer == &slider) {
        value = (int)slider.getValue();
    }
    else if (renderer == &listbox) {
        juce::StringArray selected;
        listbox.getSelectedValues(selected);
        juce::String csv = JuceUtil::ArrayToCsv(selected);
        value = csv;
    }

    return value;
}

// Convenience casters
// note that you must call getValue since the var member will be stale

int Field::getIntValue()
{
    return (int)getValue();
}

juce::String Field::getStringValue()
{
    return getValue().toString();
}

const char* Field::getCharValue()
{
    return getValue().toString().toUTF8();
}

bool Field::getBoolValue() {
    return (bool)getValue();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

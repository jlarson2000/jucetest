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
*/

#include <string>
#include <sstream>

#include <JuceHeader.h>

#include "Form.h"
#include "../util/qtrace.h"

//////////////////////////////////////////////////////////////////////
//
// Field
//
//////////////////////////////////////////////////////////////////////

// todo: should owning the name 

Field::Field(const char* argName, const char* argDisplayName, Field::Type argType)
{
    juce::Component::setName("Field");
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
 * Called by FieldGrid as fields are added
 */
void Field::render()
{
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
}

void Field::renderString()
{
    if (allowedValues.isEmpty()) {
        renderType = RenderType::Text;
        renderer = &textbox;
        
        textbox.setEditable(true);
        textbox.setColour (juce::Label::backgroundColourId, juce::Colours::lightblue);
        // from the demo
        // textbox.onTextChange = [this] { uppercaseText.setText (inputText.getText().toUpperCase(), juce::dontSendNotification); };
    }
    else if (!multi) {
        renderType = RenderType::Combo;
        renderer = &combobox;
        
        for (int i = 0 ; i < allowedValues.size() ; i++) {
            // note that item ids must be non-zero
            combobox.addItem(allowedValues[i], i + 1);
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
    }
    else {
        renderType = RenderType::List;
        renderer = &listbox;
        //listbox.setListValues(allowedValues);
        listbox.setValues(allowedValues);

        // needs much more work
        preferredHeight = 30 * 3;
    }
    
}

void Field::renderInt()
{
    if (min == 0 && max == 0) {
        renderType = RenderType::Text;
        renderer = &textbox;
        
        textbox.setEditable(true);
        // make them look different for testing
        textbox.setColour (juce::Label::backgroundColourId, juce::Colours::darkblue);
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
}

/**
 * Attached label notes
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
 */
void Field::resized()
{
    // do them all the same way for now, may want more control
    if (renderer != nullptr) {
        int height = getHeight();
        if (height == 0)
          height = 30;
        renderer->setBounds(100, 0, getWidth() - 110, height);
    }
}

void Field::paint(juce::Graphics& g)
{
    // give it an obvious background
    // need to work out  borders
    g.fillAll (juce::Colours::beige);
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

//////////////////////////////////////////////////////////////////////
//
// FieldGrid
//
//////////////////////////////////////////////////////////////////////

FieldGrid::FieldGrid()
{
    juce::Component::setName("FieldGrid");
}

FieldGrid::~FieldGrid()
{
}

void FieldGrid::setName(juce::String argName)
{
    name = argName;
}

juce::String FieldGrid::getName()
{
    return name;
}

/**
 * Add a field to the list and render it as components.
 * Don't really like deferred rendering but I want to allow a Field to
 * be constructed in phases that may influence how they are rendered.
 */
void FieldGrid::add(Field* f, int column)
{
    juce::OwnedArray<Field>* fieldColumn = nullptr;
    
    if (column >= columns.size()) {
        fieldColumn = new juce::OwnedArray<Field>();
        columns.set(column, fieldColumn);
    }
    else {
        fieldColumn = columns[column];
    }

    fieldColumn->add(f);
}

void FieldGrid::render()
{
    // I can see this happening a lot, come up with a field iterator
    for (int col = 0 ; col < columns.size() ; col++) {
        juce::OwnedArray<Field>* fieldCol = columns[col];
        if (fieldCol != nullptr) {
            for (int row = 0 ; row < fieldCol->size() ; row++) {
                Field* f = (*fieldCol)[row];
                f->render();
                addAndMakeVisible(f);
            }
        }
    }
}

/**
 * TODO: Would like each grid to auto-size the label
 * column so we don't have to hard wire it.
 */
void FieldGrid::resized()
{
    // single column for now
    juce::Rectangle<int> area = getLocalBounds();

    for (int col = 0 ; col < columns.size() ; col++) {
        juce::OwnedArray<Field>* fieldCol = columns[col];
        if (fieldCol != nullptr) {
            for (int row = 0 ; row < fieldCol->size() ; row++) {
                Field* f = (*fieldCol)[row];

                int height = f->getPreferredHeight();
                if (height == 0)
                  height = 30;
                f->setBounds(area.removeFromTop(height));
            }
        }
    }
}

void FieldGrid::paint(juce::Graphics& g)
{
    // give it an obvious background
    // need to work out  borders
        g.fillAll (juce::Colours::beige);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

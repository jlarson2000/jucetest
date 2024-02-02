/**
 * An object model for form fields that are rendered as Juce components.
 *
 * A Field is the most basic unit of input in a form. It manages the display
 * of a single value A field may be rendered using a variety of Juce components.
 * Fields will provide a preferred size which may be overridden by the container.
 *
 * Fields may be of type integer, boolean, string, or enumeration.
 * Enumeration fields will have a list of allowed values which may or not be
 * split into internal and display values.
 *
 * Fields can render an appropriate Juce component automaticaly.
 * Fields may accept an alternative renderStyle to select from among different options.
 *
 * Fields may be multi-valued.
 * 
 * Fields are given a value and will return it after editing.  They do not know
 * where the value came from.
 *
 * Fields may have a listener to respond to changes to the field.
 *
 * A FieldSet organizes multiple fields into one or more columns.
 * All fields in a field set are displayed.
 *
 * A Form contains one or more field setes.
 * If there are multiple field sets they are selcted using tabs or
 * some other set selection mechanism.  If a form contains more than one
 * field set, the field set must be named to provide the name for the tab.
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

#pragma once

#include <JuceHeader.h>
#include "SimpleListBox.h"

//////////////////////////////////////////////////////////////////////
//
// Field
//
//////////////////////////////////////////////////////////////////////

/**
 * A field defines a set of editing components for a single named paramter value.
 * Fields have a data type of boolean, integer, or string.
 * Integer fields may have allowed value ranges.
 * String fields may have a set of allowed values.
 */
class Field : public juce::Component
{
  public:

    enum class Type {Integer, Boolean, String};

    enum class RenderType {Text, Combo, Check, Slider, Rotary, List};
    
    // assume for now that these can be literal string constants
    // and we don't have to worry about lifespan
    Field(const char* name, const char* displayName, Type type);
    ~Field();

    const char* getName() {
        return name;
    }

    Type getType() {
        return type;
    }
    
    /**
     * Fields may have an alternative diplay name
     * that is used for the label.
     */
    void setDisplayName(const char* s) {
        displayName = s;
    }

    const char* getDisplayName() {
        return displayName;
    }

    const char* getDisplayableName() {
        return (displayName != nullptr) ? displayName : name;
    }

    /**
     * Set the suggested rendering type if you don't want it to default.
     */
    void setRenderType(RenderType rt) {
        renderType = rt;
    }

    void setMulti(bool b) {
        multi = b;
    }
    
    /**
     * Integer fields may have a range.
     * If a range is specified the field will be rendered
     * using a knob or slider.  If a range is not specified
     * it is rendered as a text box whose size may be influenced
     * by the size member.
     */
    void setMin(int i) {
        min = i;
    }

    int getMin() {
        return min;
    }

    void setMax(int i) {
        max = i;
    }

    int getMax() {
        return max;
    }

    /**
     * For string fields, this is the number of characcters to display.
     */
    void setSize(int i) {
        size = i;
    }

    int getSize() {
        return size;
    }

    /**
     * String values may have value limits.
     * TODO: needs a lot more work to have internal/display values
     * in a proper CC++ way.  Find a tuple class in std:: or something
     */
    juce::StringArray getAllowedValues();

    /**
     * String values with limits may want an alternative name
     * to display for the values.  The size of this array
     * must be the same as the allowed values list.
     */
    juce::StringArray getAllowedValueLabels();

    // work on how we want to consistently define these
    void setAllowedValues(const char** a);
    void setAllowedValues(juce::StringArray& src);
    
    void setAllowedValueLabels(const char** a);
    void setAllowedValueLabels(juce::StringArray& src);

    int getPreferredHeight() {
        return preferredHeight;
    }

    void setPreferredHeight(int h) {
        preferredHeight = h;
    }

    int getPreferredWidth() {
        return preferredWidth;
    }

    void setPreferredWidth(int w) {
        preferredWidth = w;
    }
    
    void setValue(juce::var value);
    
    void setValue(int i) {
        value = i;
    }

    void setValue(const char* string) {
        value = string;
    }

    void setValue(bool b) {
        value = b;
    }

    juce::var getValue() {
        return value;
    }

    int getIntValue() {
        return (int)value;
    }

    juce::String getStringValue() {
        return value.toString();
    }

    const char* getCharValue() {
        return value.toString().toUTF8();
    }

    bool getBoolValue() {
        return (bool)value;
    }

    void refreshValue();

    // build out the Juce components to display this field
    void render();

    //
    // Juce interface
    //

    void resized() override;
    
  private:
    
    void init();
    
    void renderInt();
    void renderString();
    void renderBool();
    
    const char* name = nullptr;
    const char* displayName = nullptr;
    Type type = Type::Integer;
    RenderType renderType = RenderType::Text;
    bool multi = false;
    int min = 0;
    int max = 0;
    int size = 0;
    int preferredWidth = 0;
    int preferredHeight = 0;

    juce::StringArray allowedValues = {};
    juce::StringArray allowedValueLabels = {};

    // the current value of this field
    juce::var value;

    // rendered components
    
    // readable label, for booleans, we could make use of the built-in
    // labeling of ToggleButton
    juce::Label label;
    
    // component used to render the value, dependent on Type and options
    juce::Label textbox;
    juce::ToggleButton checkbox;
    juce::ComboBox combobox;
    juce::Slider slider;
    SimpleListBox listbox;
    
    // the component we chose to render the value
    juce::Component* renderer = nullptr;
};

/**
 * A field set is a list of Fields that can be arranged in columns
 * TODO: should the column split be a property of the field or the set?
 * The field set owns the Fields which are deleted when the set is deleted.
 */
class FieldSet : public juce::Component {

  public:
    FieldSet();
    ~FieldSet();

    void setName(juce::String argName);

    void setName(const char* argName) {
        setName(juce::String(argName));
    }
    
    juce::String getName();
    
    void add(Field* field, int column = 0);

    // iterator interface
    int getColumns() {
        return columns.size();
    }

    juce::OwnedArray<Field>* getColumn(int index) {
        juce::OwnedArray<Field>* column = nullptr;
        if (index >= 0 && index < columns.size())
          column = columns[index];
        return column;
    }
    
    void render();
    
    void resized() override;
    
  private:

    juce::String name;
    juce::OwnedArray<juce::OwnedArray<Field>> columns;
};

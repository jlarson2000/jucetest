/**
 * Component model for Mobius configuration forms.
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
 *
 * TODO: Starting with const char* for names and assuming those will
 * have infinite lifetime. This is bad but works for now.
 * Revisit this to use std::string when I understand them better.
 */
class Field : public juce::Component
{
  public:

    enum class Type {Int, Bool, String};

    enum class RenderType {Text, Combo, Check, Slider, Rotary, List};
    
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
     * in a proper CC++ way
     */
    void setAllowedValues(const char** a) {
        allowedValues = a;
    }

    const char** getAllowedValues() {
        return allowedValues;
    }

    // only for FieldSet, should be a friend class?
    void render();

    //
    // Juce interface
    //

    void resized() override;
    
  private:
    
    void renderInt();
    void renderString();
    void renderBool();
    
    const char* name = nullptr;
    const char* displayName = nullptr;
    Type type = Type::Int;
    RenderType renderType = RenderType::Text;
    bool multi = false;
    int min = 0;
    int max = 0;
    int size = 0;
    const char** allowedValues = nullptr;

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

    void add(Field* field);

    void resized() override;
    
  private:

    // TODO: we could just use the child component list and downcast
    // compare with std::vector
    juce::OwnedArray<Field> fields;
};

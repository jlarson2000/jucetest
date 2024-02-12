/*
 * Extension of the ListBox that provides display of a simple list of strings
 * and allows them to be selected.  What other systems call a "multiselect".
 * Also provides for an alternate set of labels that are displayed instead
 * of the internal values and maps between them.
 */

#pragma once

#include <JuceHeader.h>

class SimpleListBox : public juce::Component, public juce::ListBoxModel
{
  public:

    class Listener {
      public:
        virtual void selectedRowsChanged(SimpleListBox*box, int lastRowSelected) = 0;
    };
    
    SimpleListBox();
    ~SimpleListBox();

    void addListener(Listener* l) {
        listener = l;
    }
    
    void setMultipleSelectionEnabled(bool b) {
        // note that turning this on also effectdively
        // disables setClickingTogglesRowSelection(true)
        // you can't have toggling rows on a single select
        // odd, I could think of uses for that?
        listBox.setMultipleSelectionEnabled(b);
    }
    
    void setValues(juce::StringArray& src);
    void setValueLabels(juce::StringArray& src);

    // if you use this you can't use value/label pairs
    // need more flexibility here
    void add(juce::String value);

    void setSelectedValues(juce::StringArray& src);
    void getSelectedValues(juce::StringArray& selected);
    void deselectAll();

    // todo: added this for single select box convenience
    // if we have more than one will need more selected
    // row iteration methods
    int getSelectedRow();
    void setSelectedRow(int index);
    juce::String getSelectedValue();
    juce::String getRowValue(int index);

    // juce::Component overrides
    
    void paint (juce::Graphics& g) override;
    void resized() override;

    // ListBoxModel

    int getNumRows() override;

    void paintListBoxItem (int rowNumber, juce::Graphics& g,
                           int width, int height, bool rowIsSelected) override;

    void selectedRowsChanged (int /*lastRowselected*/) override;

  private:
    
    Listener* listener = nullptr;
    juce::ListBox listBox;

    juce::StringArray values;
    juce::StringArray valueLabels;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleListBox)
};

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
    SimpleListBox();
    ~SimpleListBox();

    void setValues(juce::StringArray& src);
    void setValueLabels(juce::StringArray& src);

    void setSelectedValues(juce::StringArray& src);
    void getSelectedValues(juce::StringArray& selected);

    // juce::Component overrides
    
    void paint (juce::Graphics& g) override;
    void resized() override;

    // ListBoxModel

    int getNumRows() override;

    void paintListBoxItem (int rowNumber, juce::Graphics& g,
                           int width, int height, bool rowIsSelected) override;

    void selectedRowsChanged (int /*lastRowselected*/) override;

  private:
    
    juce::ListBox listBox;

    juce::StringArray values;
    juce::StringArray valueLabels;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleListBox)
};

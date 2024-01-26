/*
 * Extension of the ListBox that provides display of a simple list of strings
 * and allows them to be selected.  What other systems call a "multiselect".
 */

#pragma once

#include <JuceHeader.h>

class SimpleListBox : public juce::Component, public juce::ListBoxModel
{
  public:
    SimpleListBox();
    ~SimpleListBox();

    void setValues(const char** strings);
    
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
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleListBox)
};

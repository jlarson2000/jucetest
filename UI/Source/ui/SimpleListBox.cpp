#include <JuceHeader.h>

#include "SimpleListBox.h"
#include "../util/qtrace.h"

SimpleListBox::SimpleListBox()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    addAndMakeVisible(listBox);

    juce::Font font = juce::Font(juce::Font (16.0f, juce::Font::bold));
    listBox.setRowHeight (font.getHeight());
    listBox.setModel (this);   // Tell the listbox where to get its data model
    listBox.setColour (juce::ListBox::textColourId, juce::Colours::black);
    listBox.setColour (juce::ListBox::backgroundColourId, juce::Colours::white);

    listBox.setMultipleSelectionEnabled(true);
    listBox.setClickingTogglesRowSelection(true);
}

SimpleListBox::~SimpleListBox()
{
}

void SimpleListBox::setValues(juce::StringArray& src)
{
    // this copies
    values = src;

    // docs say "This must only be called from the main message thread"
    // not sure what that means
    listBox.updateContent();
}

void SimpleListBox::setValueLabels(juce::StringArray& src)
{
    valueLabels = src;
    listBox.updateContent();
}

/**
 * Set the initial selected rows.
 */
void SimpleListBox::setSelectedValues(juce::StringArray& selected)
{
    listBox.deselectAllRows();
    for (int i = 0 ; i < selected.size() ; i++) {
        juce::String value = selected[i];
        int index = values.indexOf(value);
        if (index >= 0) {
            listBox.selectRow(index,
                              true /* don't scroll */,
                              false /* delect others FIRST */);
        }
    }
}

/**
 * Return only the selected values.
 */
void SimpleListBox::getSelectedValues(juce::StringArray& selected)
{
    juce::SparseSet<int> rows = listBox.getSelectedRows();
    for (int i = 0 ; i < rows.size() ; i++) {
        int row = rows[i];
        selected.add(values[row]);
    }
}


void SimpleListBox::paint(juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */
	g.fillAll (juce::Colours::lightgrey);   // clear the background
    
}

void SimpleListBox::resized()
{
    // inner ListBox fills us, makes us whole
	listBox.setBounds(getBounds());
}

//
// ListBoxModel
//

int SimpleListBox::getNumRows()
{
    return values.size();
}

void SimpleListBox::paintListBoxItem (int rowNumber, juce::Graphics& g,
                                      int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
      g.fillAll (juce::Colours::lightblue);        

    g.setColour (juce::Colours::black);
    g.setFont (height * 0.7f);
   
    // g.drawText ("Row Number " + String (rowNumber + 1), 5, 0, width, height,
    // Justification::centredLeft, true);
    juce::String s;
    if (valueLabels.size() > 0)
      s = valueLabels[rowNumber];
    else
      s = values[rowNumber];

    g.drawText (s, 5, 0, width, height, juce::Justification::centredLeft, true);
}

/**
 * Don't need to overload this, the listbox will track selections
 * and we'll call getSelectedValues when we want to save them.
 */
void SimpleListBox::selectedRowsChanged (int /*lastRowselected*/)
{
    //do stuff when selection changes
}

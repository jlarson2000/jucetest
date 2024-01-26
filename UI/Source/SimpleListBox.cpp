#include <JuceHeader.h>

#include "SimpleListBox.h"
#include "Trace.h"

SimpleListBox::SimpleListBox()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    addAndMakeVisible(listBox);
    listBox.setRowHeight (30);
    listBox.setModel (this);   // Tell the listbox where to get its data model
    listBox.setColour (juce::ListBox::textColourId, juce::Colours::black);
    listBox.setColour (juce::ListBox::backgroundColourId, juce::Colours::white);
    
    // setSize (400,600);
}

SimpleListBox::~SimpleListBox()
{
}

void SimpleListBox::setValues(const char** strings) {

    // really don't understand how this works, strange that you
    // can take this in the constructor but not in add
    // values.add(strings);

    values.clear();
    if (strings != nullptr) {
        for (int i = 0 ; strings[i] != NULL ; i++) {
            values.add(strings[i]);
        }
    }
    
    listBox.updateContent();
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
    // This method is where you should set the bounds of any child
    // components that your component contains..

	listBox.setBounds(0, 0, getParentWidth(), getParentHeight());
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

    g.drawText (values[rowNumber], 5, 0, width, height, juce::Justification::centredLeft, true);
}

void SimpleListBox::selectedRowsChanged (int /*lastRowselected*/)
{
    //do stuff when selection changes
}

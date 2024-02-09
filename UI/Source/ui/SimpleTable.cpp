/*
 * Provides a basic table with column headers and various
 * content and notification options.
 */

#include <JuceHeader.h>

#include "../util/Trace.h"
#include "Panel.h"
#include "SimpleTable.h"

SimpleTable::SimpleTable()
{
    setName("SimpleTable");
    addAndMakeVisible(table);
}

SimpleTable::~SimpleTable()
{
}

void SimpleTable::render()
{
    juce::TableHeaderComponent& header = table.getHeader();

    for (int i = 0 ; i < columnTitles.size() ; i++) {
        // columnId, width, minWidth, maxWidth, propertyFlags, insertIndex
        header.addColumn(columnTitles[i], i+1, 100, 100, 100,
                         juce::TableHeaderComponent::defaultFlags);
    }

    //header.setSortColumnId (1, true);
    table.setMultipleSelectionEnabled (true);

    setSize(500, 200);
}            

void SimpleTable::resized()
{
    table.setSize(getWidth() - 10, getHeight() - 10);
}

void SimpleTable::paint (juce::Graphics& g)
{
    // temporary, give it an obvious background while we play with positioning
    g.fillAll (juce::Colours::white);
}

//
// TableListBoxModel
//

int SimpleTable::getNumRows()
{
    return 20;
}

void SimpleTable::paintRowBackground (juce::Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected)
{
    auto alternateColour = getLookAndFeel().findColour (juce::ListBox::backgroundColourId)
        .interpolatedWith (getLookAndFeel().findColour (juce::ListBox::textColourId), 0.03f);
    if (rowIsSelected)
      g.fillAll (juce::Colours::lightblue);
    else if (rowNumber % 2)
      g.fillAll (alternateColour);
}

void SimpleTable::paintCell (juce::Graphics& g, int rowNumber, int columnId,
                             int width, int height, bool rowIsSelected)
{
    g.setColour (rowIsSelected ? juce::Colours::darkblue : getLookAndFeel().findColour (juce::ListBox::textColourId));  // [5]
    g.setFont (font);

    char buf[1024];
    sprintf(buf, "%d/%d", rowNumber, columnId);
    
    g.drawText (juce::String(buf), 2, 0, width - 4, height, juce::Justification::centredLeft, true);
    g.setColour (getLookAndFeel().findColour (juce::ListBox::backgroundColourId));
    g.fillRect (width - 1, 0, 1, height);
}

void SimpleTable::cellClicked(int rowNumber, int columnId, const juce::MouseEvent& event)
{
    Trace(1, "Clicked %d/%d\n", rowNumber, columnId);
}

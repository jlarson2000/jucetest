
#include <JuceHeader.h>

#include "../util/Trace.h"

#include "JuceUtil.h"

#include "SimpleTable.h"
#include "TableTest.h"

TableTest::TableTest()
{
    setName("TableTest");

    addAndMakeVisible(table);
    
    table.setColumnTitles(juce::StringArray({"Foo", "Bar", "Baz"}));
    table.render();
    
    setSize (800, 800);
}

TableTest::~TableTest()
{
}

void TableTest::resized()
{
    table.setTopLeftPosition(10, 10);
}

void TableTest::paint (juce::Graphics& g)
{
    // temporary, give it an obvious background while we play with positioning
    g.fillAll (juce::Colours::grey);
}

void TableTest::show()
{
    setVisible(true);
    center();
    JuceUtil::dumpComponent(this);
}

/**
 * Center within the parent
 */
void TableTest::center()
{
    int pwidth = getParentWidth();
    int pheight = getParentHeight();
    
    int mywidth = getWidth();
    int myheight = getHeight();
    
    if (mywidth > pwidth) mywidth = pwidth;
    if (myheight > pheight) myheight = pheight;

    int left = (pwidth - mywidth) / 2;
    int top = (pheight - myheight) / 2;
    
    setTopLeftPosition(left, top);
}

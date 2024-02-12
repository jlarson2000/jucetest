
#include <JuceHeader.h>

#include "../util/Trace.h"

#include "JuceUtil.h"

#include "SimpleTable.h"
#include "TableTest.h"

TableTest::TableTest()
{
    setName("TableTest");

    addAndMakeVisible(table);
    
    table.setColumnTitles(juce::StringArray({"Button", "Target"}));
    table.setColumnWidth(0, 200);
    table.setColumnWidth(1, 50);
    table.setHeaderHeight(22);
    table.setRowHeight(22);
    table.addListener(this);
    
    setSize (800, 800);

    for (int i = 0 ; i < 20 ; i++) {
        char buf[1024];
        sprintf(buf, "Button %d", i+1);
        table.setCell(i, 0, juce::String(buf));
        if (i % 2)
          table.setCell(i, 1, juce::String("X"));
    }
    table.dumpCells();
}

TableTest::~TableTest()
{
}

void TableTest::resized()
{
    // total column widths is 250
    // if we need this little calculation consider
    // a getPreferredHeight(int numberOfRows) method
    table.setTopLeftPosition(10, 10);
    table.setSize(300, table.getHeaderHeight() + table.getRowHeight() * 10);
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

/**
 * SimpleTable::Listener
 */
void TableTest::tableTouched(SimpleTable* table)
{
    Trace(1, "Table touched: row %d col %d\n", table->getSelectedRow(), table->getSelectedColumn());
}


#include <JuceHeader.h>

#include "../util/Trace.h"

#include "JuceUtil.h"

#include "SimpleTabPanel.h"
#include "TabTest.h"

TabTest::TabTest()
{
    setName("TabTest");

    // addAndMakeVisible(tabs);
    tabs.addTab("Foo");
    tabs.addTab("Bar");
    tabs.setBackgroundColor(juce::Colours::black);
    
    addAndMakeVisible(targets);

    addAndMakeVisible(buttons);
    buttons.add(juce::String("Save"));
    buttons.add(juce::String("Cancel"));
    buttons.addListener(this);
    
    setSize (800, 800);
}

TabTest::~TabTest()
{
}

void TabTest::resized()
{
    // total column widths is 250
    // if we need this little calculation consider
    // a getPreferredHeight(int numberOfRows) method
    tabs.setTopLeftPosition(10, 10);
    tabs.setSize(500, 300);

    // only one of these will be added at a time
    targets.setTopLeftPosition(10, 10);
    targets.setSize(500, 300);

    juce::Rectangle<int> area = getBounds();
    buttons.setBounds(area.removeFromBottom(20));
}

void TabTest::paint (juce::Graphics& g)
{
    // temporary, give it an obvious background while we play with positioning
    g.fillAll (juce::Colours::grey);
}

void TabTest::show()
{
    setVisible(true);
    center();
    JuceUtil::dumpComponent(this);
}

/**
 * Center within the parent
 * Must be an easier way to do this...
 */
void TabTest::center()
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

void TabTest::buttonClicked(juce::String name)
{
    // trace("Button %s\n", name.toUTF8());
    setVisible(false);
}

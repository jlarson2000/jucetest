/**
 * Simple component to contain random tests.
 */

#include <JuceHeader.h>

#include "TestPanel.h"
#include "JuceUtil.h"

TestPanel::TestPanel()
{
    setName("TestPanel");

    tabs.addTab("One", juce::Colours::black, nullptr, false);
    tabs.addTab("Two", juce::Colours::black, nullptr, false);
    tabs.addTab("Three", juce::Colours::black, nullptr, false);
    //addAndMakeVisible(tabs);

    addAndMakeVisible(content);
    //content.addAndMakeVisible(tabs);
    
//    setSize (500, 500);
}

TestPanel::~TestPanel()
{
}

void TestPanel::resized()
{
    tabs.setSize(getWidth(), getHeight());
    content.setSize(getWidth(), getHeight());
}

void TestPanel::paint (juce::Graphics& g)
{
    // temporary, give it an obvious background while we play with positioning
    g.fillAll (juce::Colours::yellow);
}

/**
 * TestPanels are not at the moment resizeable, but they
 * can auto-center within the parent.
 */
void TestPanel::center()
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

TestContentPanel::TestContentPanel()
{
    setName("TestContentPanel");
}

TestContentPanel::~TestContentPanel()
{
}

void TestContentPanel::resized()
{
    // assume subclass added a single child
    Component* child = getChildComponent(0);
    if (child != nullptr)
      child->setSize(getWidth(), getHeight());
}

void TestContentPanel::paint(juce::Graphics& g)
{
    g.fillAll (juce::Colours::beige);
}



TestPanelSub::TestPanelSub()
{
    setName("TestPanelSub");

    subtabs.addTab("One", juce::Colours::black, nullptr, false);
    subtabs.addTab("Two", juce::Colours::black, nullptr, false);
    subtabs.addTab("Three", juce::Colours::black, nullptr, false);

    content.addAndMakeVisible(subtabs);
    //addAndMakeVisible(content);
    
    setSize (500, 500);
    
    JuceUtil::dumpComponent(this);

}

TestPanelSub::~TestPanelSub()
{
}


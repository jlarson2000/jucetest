/**
 * Simple component to contain random tests.
 */

#include <JuceHeader.h>

#include "../util/Trace.h"
#include "../model/Parameter.h"

#include "common/Panel.h"
#include "common/Field.h"
#include "common/FieldGrid.h"
#include "common/SimpleRadio.h"
#include "common/Form.h"
#include "config/ParameterField.h"

#include "JuceUtil.h"
#include "TestPanel.h"

TestPanel::TestPanel()
{
    setName("TestPanel");

    tabs.addTab("One", juce::Colours::black, nullptr, false);
    tabs.addTab("Two", juce::Colours::black, nullptr, false);
    tabs.addTab("Three", juce::Colours::black, nullptr, false);
    //addAndMakeVisible(tabs);
    // addAndMakeVisible(content);
    //content.addAndMakeVisible(tabs);
    
    label.setText(juce::String("Hello World!"));
    label.setBorder(juce::Colours::red);
    //addAndMakeVisible(label);

    Panel* panel1 = new Panel(Panel::Orientation::Horizontal);
    panel1->addShared(&label);
    JLabel* l = new JLabel(juce::String("xxxxxxxxxxxxxxxxxxxx!"));
    l->setBorder(juce::Colours::green);
    panel1->addOwned(l);
    panel1->autoSize();
             
    panel.addOwned(panel1);

    SimpleRadio* r = new SimpleRadio();
    r->setLabel("Radio");
    r->setButtonLabels(juce::StringArray({"1", "2", "3"}));
    r->render();
    r->setListener(this);
    panel.addOwned(r);

    Field* f = new Field("xyz", "XYZ", Field::Type::Boolean);
    f->setUnmanagedLabel(true);
    f->render();
    panel.addOwned(f);

    f = new Field("foo", "Foo", Field::Type::String);
    f->render();
    panel.addOwned(f);
    
    f = new Field("bar", "Bar", Field::Type::Integer);
    f->render();
    panel.addOwned(f);

    f = new Field("baz", "Baz", Field::Type::Integer);
    f->setMin(0);
    f->setMax(127);
    f->render();
    panel.addOwned(f);

    f = new Field("blort", "Blort", Field::Type::Integer);
    f->setMin(0);
    f->setMax(127);
    f->setRenderType(Field::RenderType::Rotary);
    f->render();
    panel.addOwned(f);

    f = new Field("sex", "Sex", Field::Type::Boolean);
    f->render();
    panel.addOwned(f);
    
    FieldGrid* grid = new FieldGrid();
    for (int col = 0 ; col < 2 ; col++) {
        for (int row = 0 ; row < 4 ; row++) {
            char buf[80];
            sprintf(buf, "%d/%d", col, row);
            f = new Field(buf, buf, Field::Type::Boolean);
            grid->add(f, col);
        }
    }
    grid->render();
    panel.addOwned(grid);

    FormPanel* fpanel = new FormPanel();
    grid = new FieldGrid();
    for (int col = 0 ; col < 2 ; col++) {
        for (int row = 0 ; row < 4 ; row++) {
            char buf[80];
            sprintf(buf, "%d/%d", col, row);
            f = new Field(buf, buf, Field::Type::Boolean);
            grid->add(f, col);
        }
    }
    fpanel->addGrid(grid);
    fpanel->render();
    panel.addOwned(fpanel);
    
    Form* form = new Form();
    form->add(new ParameterField(LoopCountParameter), "General");
    form->add(new ParameterField(SubCycleParameter), "General");
    form->add(new ParameterField(RecordThresholdParameter), "Record");
    form->add(new ParameterField(AutoRecordBarsParameter), "Record");
    form->render();
    panel.addOwned(form);

    SimpleListBox* box = new SimpleListBox();
    box->add(juce::String("One"));
    box->add(juce::String("Two"));
    box->add(juce::String("Three"));
    box->setSize(100, 80);
    box->setMultipleSelectionEnabled(false);
    panel.addOwned(box);

    // let the panel expand to fit the children
    panel.autoSize();
    addAndMakeVisible(panel);
    
    // make the test panel bigger than it needs to be, though
    // we could have it track the inner form
    setSize (500, 500);
}

TestPanel::~TestPanel()
{
}

void TestPanel::resized()
{
    tabs.setSize(getWidth(), getHeight());
    content.setSize(getWidth(), getHeight());

    // label.setTopLeftPosition(10, 10);

    panel.setTopLeftPosition(10, 10);
}

void TestPanel::paint (juce::Graphics& g)
{
    // temporary, give it an obvious background while we play with positioning
    g.fillAll (juce::Colours::grey);
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


void TestPanel::radioSelected(SimpleRadio* radio, int index)
{
    Trace(1, "Radio selected %d\n", index);
}

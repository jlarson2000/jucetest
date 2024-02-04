
#include <JuceHeader.h>

#include <string>
#include <sstream>

#include "../util/qtrace.h"

#include "SetupPanel.h"

SetupPanel::SetupPanel(ConfigEditor* argEditor) :
    ConfigPanel{argEditor, "Track Setups", ConfigPanelButton::Save | ConfigPanelButton::Cancel, true},
    tabs {juce::TabbedButtonBar::Orientation::TabsAtTop}
{
    // debugging hack
    setName("SetupPanel");

#if 0
    std::ostringstream ss;
    ss << "Creating SetupPanel\n";
    qtrace(&ss);
    
    fields.add(new Field("first", "First", Field::Type::Integer));
    fields.add(new Field("second", "Second", Field::Type::String));
    fields.add(new Field("third", "Third", Field::Type::Boolean));

    Field* combo = new Field("fourth", "Fourth", Field::Type::String);
    juce::StringArray cvalues = {"a", "b", "c"};
    combo->setAllowedValues(cvalues);
    fields.add(combo);

    Field* slider = new Field("fifth", "Fifth", Field::Type::Integer);
    slider->setMin(0);
    slider->setMax(127);
    fields.add(slider);

    Field* knob = new Field("six", "Sixth", Field::Type::Integer);
    knob->setMin(0);
    knob->setMax(127);
    knob->setRenderType(Field::RenderType::Rotary);
    fields.add(knob);
    

    Field* list = new Field("seven", "Seventh", Field::Type::String);
    list->setMulti(true);
    // const char* lvalues[] = {"one", "two", "three", nullptr};
    juce::StringArray lvalues = {"one", "two", "three"};
    list->setAllowedValues(lvalues);

    fields.add(list);

    // At this point, the content component has already been sized
    // during superclass initialization.  Adding a child does not
    // trigger resized so have to do it manually
    content.addAndMakeVisible(fields);
    fields.setBounds(content.getBounds());
#endif
    tabs.addTab("One", juce::Colours::black, nullptr, false);
    tabs.addTab("Two", juce::Colours::black, nullptr, false);
    tabs.addTab("Three", juce::Colours::black, nullptr, false);
    content.addAndMakeVisible(tabs);

    // at this point the component hierarhcy has been fully constructed
    // but not sized, until we support bottom up sizing start with
    // a fixed size, this will cascade resized() down the child hierarchy
    setSize (500, 500);
}

SetupPanel::~SetupPanel()
{
}

void SetupPanel::load()
{
}

void SetupPanel::save()
{
}

void SetupPanel::cancel()
{
}

//////////////////////////////////////////////////////////////////////
//
// ObjectSelector overloads
// 
//////////////////////////////////////////////////////////////////////

void SetupPanel::selectObject(int ordinal)
{
}

void SetupPanel::newObject()
{
}

void SetupPanel:: deleteObject()
{
}

void SetupPanel::revertObject()
{
}

void SetupPanel::renameObject(juce::String newName)
{
}

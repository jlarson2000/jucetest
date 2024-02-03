
#include <JuceHeader.h>

#include <string>
#include <sstream>

#include "GlobalPanel.h"
#include "../util/qtrace.h"

GlobalPanel::GlobalPanel(ConfigEditor* argEditor) :
    ConfigPanel{argEditor, "Global Parameters", ConfigPanelButton::Save | ConfigPanelButton::Cancel}
{
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
}

GlobalPanel::~GlobalPanel()
{
}

void GlobalPanel::show()
{
}

void GlobalPanel::save()
{
}

void GlobalPanel::revert()
{
}

void GlobalPanel::cancel()
{
}

bool GlobalPanel::isActive()
{
    return false;
}

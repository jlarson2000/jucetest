
#include <JuceHeader.h>

#include <string>
#include <sstream>

#include "SetupPanel.h"
#include "Trace.h"

SetupPanel::SetupPanel() : ConfigPanel{"Track Setups", ConfigPanelButton::Save | ConfigPanelButton::Cancel}
{
    std::ostringstream ss;
    ss << "Creating SetupPanel\n";
    Trace(&ss);
    
    fields.add(new Field("first", "First", Field::Type::Int));
    fields.add(new Field("second", "Second", Field::Type::String));
    fields.add(new Field("third", "Third", Field::Type::Bool));

    Field* combo = new Field("fourth", "Fourth", Field::Type::String);
    // does this have to be terminated?
    const char* values[] = {"one", "two", "three", nullptr};
    combo->setAllowedValues(values);
    fields.add(combo);

    Field* slider = new Field("fifth", "Fifth", Field::Type::Int);
    slider->setMin(0);
    slider->setMax(127);
    fields.add(slider);

    Field* knob = new Field("six", "Sixth", Field::Type::Int);
    knob->setMin(0);
    knob->setMax(127);
    knob->setRenderType(Field::RenderType::Rotary);
    fields.add(knob);
    

    Field* list = new Field("seven", "Seventh", Field::Type::String);
    list->setMulti(true);
    const char* lvalues[] = {"one", "two", "three", nullptr};
    list->setAllowedValues(lvalues);
    fields.add(list);

    // At this point, the content component has already been sized
    // during superclass initialization.  Adding a child does not
    // trigger resized so have to do it manually
    content.addAndMakeVisible(fields);
    fields.setBounds(content.getBounds());
}

SetupPanel::~SetupPanel()
{
    std::ostringstream ss;
    ss << "Deleting SetupPanel\n";
    Trace(&ss);
}

    

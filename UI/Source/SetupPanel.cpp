
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
    
    form.addField(new Field("First", Field::Type::Int));
    form.addField(new Field("Second", Field::Type::String));
    form.addField(new Field("Third", Field::Type::Bool));

    juce::OwnedArray<Field>& fields = form.getFields();
    for (int i = 0 ; i < fields.size() ; i++) {
        Field* f = fields[i];
        ss.str("");
        ss << "Field " << i << " is " << f->getName() << "\n";
        Trace(&ss);
    }

}

SetupPanel::~SetupPanel()
{
    std::ostringstream ss;
    ss << "Deleting SetupPanel\n";
    Trace(&ss);
}

    

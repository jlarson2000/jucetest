
#include <string>
#include <sstream>

#include <JuceHeader.h>

#include "Form.h"
#include "Trace.h"

Form::Form()
{
    std::ostringstream ss;
    ss << "Creating Form\n";
    Trace(&ss);
}

Form::~Form()
{
    std::ostringstream ss;
    ss << "Deleting Form\n";
    Trace(&ss);
}

void Form::addField(Field* f)
{
    std::ostringstream ss;
    ss << "Adding field: " << f->getName() << "\n";
    Trace(&ss);
    fields.add(f);
}

juce::OwnedArray<Field>& Form::getFields()
{
    return fields;
}

//////////////////////////////////////////////////////////////////////
//
// Field
//
//////////////////////////////////////////////////////////////////////

// todo: should owning the name 

Field::Field(const char* argName, Field::Type argType)
{
    name = argName;
    type = argType;
}

Field::~Field()
{
    std::ostringstream ss;
    ss << "Deleting field: " << name << "\n";
    Trace(&ss);
}


/**
 * A ParameterField extends the Field model to provide
 * initialization based on a Mobius Parameter definition.
 */

#pragma once

#include "JuceHeader.h"

#include "../model/Parameter.h"
#include "Field.h"

class ParameterField : public Field
{
  public:

    ParameterField(class Parameter* p);
    ~ParameterField();

    static Field::Type convertParameterType(ParameterType intype);

    void loadValue(void* sourceObject);
    void saveValue(void* targetObject);
    
  private:

    Parameter* parameter;

};
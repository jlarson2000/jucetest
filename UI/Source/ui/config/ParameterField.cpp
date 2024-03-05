/**
 * A ParameterField extends the Field model to provide
 * initialization based on a Mobius Parameter definition.
 *
 * The two models are similar and could be redisigned to share more
 * but Parameter has a lot of old code dependent on it so we
 * need to convert.
 */

#pragma once

#include "JuceHeader.h"

#include "../../util/Trace.h"
#include "../../model/UIParameter.h"
#include "../../model/ExValue.h"

#include "ParameterField.h"

ParameterField::ParameterField(UIParameter* p) :
    Field(p->getName(), p->getDisplayName(), convertParameterType(p->type))
{
    parameter = p;

    setMulti(p->multi);

    // todo: need to figure out how to handle fields with
    // confirable highs
    setMin(p->low);
    setMax(p->high);

    // enums must have allowed values, strings are optional
    if (p->type == TypeEnum || p->type == TypeString) {
        if (p->values != nullptr) {
            setAllowedValues(p->values);
        }
        if (p->valueLabels != nullptr) {
            setAllowedValueLabels(p->valueLabels);
        }
    }
}    

ParameterField::~ParameterField()
{
}

Field::Type ParameterField::convertParameterType(UIParameterType intype)
{
    Field::Type ftype = Field::Type::Integer;

    switch (intype) {
        case TypeInt: ftype = Field::Type::Integer; break;
        case TypeBool: ftype = Field::Type::Boolean; break;
        case TypeString: ftype = Field::Type::String; break;
        case TypeEnum: ftype = Field::Type::String; break;
        case TypeStructure: ftype = Field::Type::String; break;
    }

    return ftype;
}

/**
 * Set the field's value by pulling it out of a configuration object.
 */
void ParameterField::loadValue(void *obj)
{
    juce::var value;

    // newer complex parameter values use juce::var
    // should be here for all multi valued parameters
    if (parameter->juceValues) {
        parameter->getValue(obj, value);
    }
    else {
        // old-school ExValue
        ExValue ev;
        parameter->getValue(obj, &ev);

        if (parameter->multi) {
            // supposed to be using juce::var for these
            Trace(1, "ParameterField: muli-value parameter not supported without Juce accessors\n");
        }
        else {
            switch (parameter->type) {
                case TypeInt: {
                    value = ev.getInt();
                }
                    break;
                case TypeBool: {
                    value = ev.getBool();
                }
                    break;
                case TypeString: {
                    value = ev.getString();
                }
                    break;
                case TypeEnum: {
                    // don't have to do anything special here
                    // the Field will figure out what to display
                    value = ev.getString();
                }
                    break;
                case TypeStructure: {
                    value = ev.getString();
                }
                    break;
            }
        }
    }

    setValue(value);
}

void ParameterField::saveValue(void *obj)
{
    if (parameter->juceValues) {
        parameter->setValue(obj, getValue());
    }
    else {
        ExValue ev;

        if (parameter->multi) {
            // todo: will need to handle multi-valued lists properly
            Trace(1, "ParameterField: muli-value parameter not supported without Juce accessors\n");
        }
        else {
            switch (parameter->type) {
                case TypeInt: {
                    ev.setInt(getIntValue());
                }
                    break;
                case TypeBool: {
                    ev.setBool(getBoolValue());
                }
                    break;
                case TypeString: {
                    ev.setString(getCharValue());
                }
                    break;
                case TypeEnum: {
                    ev.setString(getCharValue());
                }
                    break;
                case TypeStructure: {
                    ev.setString(getCharValue());
                }
                    break;
            }
        }

        // should do this only if we had a conversion
        if (!ev.isNull())
          parameter->setValue(obj, &ev);
    }
}

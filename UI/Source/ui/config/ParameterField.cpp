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
#include "../../model/Parameter.h"
#include "../../model/ExValue.h"

#include "ParameterField.h"

ParameterField::ParameterField(Parameter* p) :
    Field(p->getName(), p->getDisplayName(), convertParameterType(p->type))
{
    parameter = p;

    setMulti(p->multi);

    // todo: need to figure out how to handle fields with
    // confirable highs
    setMin(p->low);
    setMax(p->high);

    // enums must have allowed values, strings are optional
    if (p->type == TYPE_ENUM || p->type == TYPE_STRING) {
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

Field::Type ParameterField::convertParameterType(ParameterType intype)
{
    Field::Type ftype = Field::Type::Integer;

    switch (intype) {
        case TYPE_INT: ftype = Field::Type::Integer; break;
        case TYPE_BOOLEAN: ftype = Field::Type::Boolean; break;
        case TYPE_STRING: ftype = Field::Type::String; break;
        case TYPE_ENUM: ftype = Field::Type::String; break;
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
        parameter->getJuceValue(obj, value);
    }
    else {
        // old-school ExValue
        ExValue ev;

        parameter->getConfigValue(obj, &ev);

        if (parameter->multi) {
            // supposed to be using juce::var for these
            Trace(1, "ParameterField: muli-value parameter not supported without Juce accessors\n");
        }
        else {
            switch (parameter->type) {
                case TYPE_INT: {
                    value = ev.getInt();
                }
                    break;
                case TYPE_BOOLEAN: {
                    value = ev.getBool();
                }
                    break;
                case TYPE_STRING: {
                    value = ev.getString();
                }
                    break;
                case TYPE_ENUM: {
                    // don't have to do anything special here
                    // the Field will figure out what to display
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
        parameter->setJuceValue(obj, getValue());
    }
    else {
        ExValue ev;

        if (parameter->multi) {
            // todo: will need to handle multi-valued lists properly
            Trace(1, "ParameterField: muli-value parameter not supported without Juce accessors\n");
        }
        else {
            switch (parameter->type) {
                case TYPE_INT: {
                    ev.setInt(getIntValue());
                }
                    break;
                case TYPE_BOOLEAN: {
                    ev.setBool(getBoolValue());
                }
                    break;
                case TYPE_STRING: {
                    ev.setString(getCharValue());
                }
                    break;
                case TYPE_ENUM: {
                    ev.setString(getCharValue());
                }
                    break;
            }
        }

        // should do this only if we had a conversion
        if (!ev.isNull())
          parameter->setConfigValue(obj, &ev);
    }
}
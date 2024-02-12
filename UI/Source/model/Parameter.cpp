/*
 * Static object definitions for Mobius parameters.
 *
 * There are four parameter levels:
 *
 *    Global - usually in MobiusConfig
 *    Setup  - in Setup
 *    Track  - in SetupTrack or Track
 *    Preset - in Preset
 *
 */

#include <vector>

// for juce::value
#include <JuceHeader.h>

#include "../util/Trace.h"
#include "../util/Util.h"

#include "ExValue.h"
#include "Parameter.h"

//////////////////////////////////////////////////////////////////////
//
// Global Parameter Registry
//
//////////////////////////////////////////////////////////////////////

/**
 * A registry of all parameters, created as they are constructed.
 * This is primarily for binding where we need to associate things
 * dynamically with any parameter identified by name.  Engine
 * code rarely needs these.
 *
 * The vector will be built out by the Parameter constructor.
 * Normally all Parameter objects will be static objects.
 */
std::vector<Parameter*> Parameter::Parameters;

void Parameter::dumpParameters()
{
    for (int i = 0 ; i < Parameters.size() ; i++) {
        Parameter* p = Parameters[i];
        // !! SystemConstant has getters but most of Parameter has direct member access
        // be consistent
        Trace(1, "Parameter %s %s %s\n", p->getName(), getEnumLabel(p->type), getEnumLabel(p->scope));
    }
}

/**
 * Find a Parameter by name
 * This doesn't happen often so we can do a liner search.
 */
Parameter* Parameter::getParameter(const char* name)
{
	Parameter* found = nullptr;
	
	for (int i = 0 ; i < Parameters.size() ; i++) {
		Parameter* p = Parameters[i];
		if (StringEqualNoCase(p->getName(), name)) {
            found = p;
            break;
        }
	}
	return found;
}

/**
 * Find a parameter by it's display name.
 * I believe this is used only by the Setup editor.
 */
Parameter* Parameter::getParameterWithDisplayName(const char* name)
{
	Parameter* found = nullptr;
	
	for (int i = 0 ; i < Parameters.size() ; i++) {
		Parameter* p = Parameters[i];
		if (StringEqualNoCase(p->getDisplayName(), name)) {
			found = p;
			break;	
		}
	}
	return found;
}

//////////////////////////////////////////////////////////////////////
//
// Base Parameter Definition
//
//////////////////////////////////////////////////////////////////////

Parameter::Parameter(const char* name, const char* displayName) :
    SystemConstant(name, displayName)
{
    // try to get down to one constructor so we don't need an init()
    // also move most of these to member initializers
    init();
}

// weed out keys eventually
Parameter::Parameter(const char* name, int key) :
    SystemConstant(name, key)
{
    init();
}

void Parameter::init()
{
	type = TYPE_INT;
    multi = false;
	scope = PARAM_SCOPE_NONE;
	values = nullptr;
	valueLabels = nullptr;
	low = 0;
	high = 0;
    defaultValue = 0;

    bindable = false;
    control = false;
    juceValues = false;
    zeroCenter = false;

	dynamic = false;
    deprecated = false;
    transient = false;
    resettable = false;
    scheduled = false;
    takesAction = false;

    // add to the global registry
    Parameters.push_back(this);
}

Parameter::~Parameter()
{
}

/////////////////////////////////////////////////////////////////////
//
// Value Access
//
// Methods that implement different ways of transferring a parameter
// value from one place to another.
//
/////////////////////////////////////////////////////////////////////

void Parameter::getJuceValue(void* object, juce::var& value)
{
}

void Parameter::setJuceValue(void* object, juce::var& value)
{
}

//////////////////////////////////////////////////////////////////////
//
// Value Coercion Utilities
//
//////////////////////////////////////////////////////////////////////

/**
 * Display names for booleans
 */
const char* BOOLEAN_VALUE_NAMES[] = {
	"off", "on", nullptr
};

/**
 * Convert a ParameterType into a string for display.
 * Used only in debugging.
 */
const char* Parameter::getEnumLabel(ParameterType type)
{
    const char* label = "???";
    switch (type) {
        case TYPE_INT: label = "int"; break;
        case TYPE_BOOLEAN: label = "bool"; break;
        case TYPE_ENUM: label = "enum"; break;
        case TYPE_STRING: label = "string"; break;
    }
    return label;
}

/**
 * Convert a ParameterScope into a string for display.
 * Used only in debugging.
 */
const char* Parameter::getEnumLabel(ParameterScope scope)
{
    const char* label = "???";
    switch (scope) {
        case PARAM_SCOPE_NONE: label = "none"; break;
        case PARAM_SCOPE_PRESET: label = "preset"; break;
        case PARAM_SCOPE_TRACK: label = "track"; break;
        case PARAM_SCOPE_SETUP: label = "setup"; break;
        case PARAM_SCOPE_GLOBAL: label = "global"; break;            
    }
    return label;
}

/**
 * Convert a string value to an enumeration ordinal value.
 * This is the one used by most of the code, if the name doesn't match
 * it traces a warning message and returns the first value.
 */
int Parameter::getEnum(const char *value)
{
	int ivalue = getEnumNoWarn(value);

    // if we couldn't find a match, pick the first one
    // !! instead we should leave it at the current value?
    if (ivalue < 0) {
        Trace(1, "ERROR: Invalid value for parameter %s: %s\n",
              getName(), value);
        ivalue = 0;
    }

	return ivalue;
}

/**
 * Convert a string value to an enumeration ordinal value if possible, 
 * return -1 if invalid.  This is like getEnum() but used in cases
 * where the enum is an optional script arg and we need to know
 * whether it really matched or not.
 */
int Parameter::getEnumNoWarn(const char *value)
{
	int ivalue = -1;

	if (value != nullptr) {

		for (int i = 0 ; values[i] != nullptr ; i++) {
			if (StringEqualNoCase(values[i], value)) {
				ivalue = i;
				break;
			}
		}
        
        if (ivalue < 0) {
            // Try again with prefix matching, it is convenient
            // to allow common abbreviations like "quantize" rather 
            // than "quantized" or "all" rather than "always".  It
            // might be safe to do this all the time but we'd have to 
            // carefully go through all the enums to make sure
            // there are no ambiguities.
            for (int i = 0 ; values[i] != nullptr ; i++) {
                if (StartsWithNoCase(values[i], value)) {
                    ivalue = i;
                    break;
                }
            }
        }
	}

	return ivalue;
}

/**
 * Given an ennumeration ordinal, return the corresponding name.
 * Used by XmlRenderer to convert enum values into something more
 * meaningful.
 *
 * Note that this does not return the display label.
 * Don't have a need for that yet, but might want one.
 * Compare uses of this with the getEnumLabel methods above.
 * Those don't really return display names, do we need a split
 * for those too?  
 */
const char* Parameter::getEnumName(int value)
{
    const char* label = nullptr;
    
    // TODO: Need better range checking on the high end, though
    // this usually comes directly from an (int) cast of the enum
    if (value >= 0) {
        label = values[value];
    }
    return label;
}

/**
 * Coerce an ExValue into an enumeration ordinal.
 * This must NOT scale, it is used in parameter setters
 * and must be symetrical with getOrdinalValue.
 *
 * Can we get rid of this?  Used by a lot of parameter
 * implementations to implement setObjectValue and
 * overloaded by a few.
 */
int Parameter::getEnum(ExValue *value)
{
	int ivalue = 0;

	if (value->getType() == EX_STRING) {
		// map it through the value table
        ivalue = getEnum(value->getString());
	}
	else {
		// assume it is an ordinal value, but check the range
		// clamp it between 0 and max
		int i = value->getInt();
		if (i >= 0) {
			int max = 0;
			if (values != NULL)
			  for (max = 0 ; values[max] != NULL ; max++);

			if (i < max)
			  ivalue = i;
			else
			  ivalue = max;
		}
	}
	return ivalue;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

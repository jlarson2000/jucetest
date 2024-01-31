/*
 * Copyright (c) 2010 Jeffrey S. Larson  <jeff@circularlabs.com>
 * All rights reserved.
 * See the LICENSE file for the full copyright and license declaration.
 * 
 * ---------------------------------------------------------------------
 * 
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

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <ctype.h>

#include <vector>

#include "../util/Util.h"
#include "../util/Trace.h"
//#include "List.h"
#include "../util/MessageCatalog.h"

//#include "Action.h"
//#include "Audio.h"
//#include "Export.h"
//#include "Function.h"
#include "Messages.h"
//#include "Mobius.h"
//#include "MobiusConfig.h"
//#include "Mode.h"
//#include "Project.h"
//#include "Recorder.h"
//#include "Setup.h"
//#include "Track.h"
//#include "Script.h"
//#include "Synchronizer.h"

#include "ExValue.h"
#include "Parameter.h"

// more work to do before we can bring these in
// they're not really part of the configuration model anyway
#define HIDE_TRACK


//////////////////////////////////////////////////////////////////////
//
// Global Parameter Registry
//
// A registry of all parameters, created as they are constructed.
// This is primarily for binding where we need to associate things
// dynamically with any parameter identified by name.
//
// Not liking the global namespace for the static objects.
//
//////////////////////////////////////////////////////////////////////

/**
 * Global registry of parameters.
 * Since these are all created statically don't need to worry
 * about lifespan of the objects.
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

//
// Utilities to render enumerations for debugging
//

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

    // if not a name match, try aliases
    // todo: do we really need aliases any more?  backward compatibility
    // is much less important now
	if (found == nullptr) {
		for (int i = 0 ; i < Parameters.size() ; i++) {
			Parameter* p = Parameters[i];
			for (int j = 0 ; 
				 j < MAX_PARAMETER_ALIAS && p->aliases[j] != nullptr ; 
				 j++) {
				if (StringEqualNoCase(p->aliases[j], name)) {
					found = p;
					break;
				}
			}
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

// Shared text for boolean values

const char* BOOLEAN_VALUE_NAMES[] = {
	"off", "on", nullptr
};

int BOOLEAN_VALUE_KEYS[] = {
	MSG_VALUE_BOOLEAN_FALSE, MSG_VALUE_BOOLEAN_TRUE, 0
};

const char* BOOLEAN_VALUE_LABELS[] = {
	nullptr, nullptr, nullptr
};

/**
 * Refresh the cached display names from the message catalog.
 */
void Parameter::localizeAll(MessageCatalog* cat)
{
	for (int i = 0 ; i < Parameters.size() ; i++) {
        Parameters[i]->localize(cat);
    }
    
	// these are shared by all
	for (int i = 0 ; BOOLEAN_VALUE_NAMES[i] != nullptr; i++) {
		const char* msg = cat->get(BOOLEAN_VALUE_KEYS[i]);
		if (msg == nullptr)
		  msg = BOOLEAN_VALUE_NAMES[i];
		BOOLEAN_VALUE_LABELS[i] = msg;
	}

    // a good point to run diagnostics
    checkAmbiguousNames();
}

void Parameter::checkAmbiguousNames()
{
	for (int i = 0 ; i < Parameters.size() ; i++) {
        Parameter* p = Parameters[i];
        const char** values = p->values;
        if (values != nullptr) {
            for (int j = 0 ; values[j] != nullptr ; j++) {
                Parameter* other = getParameter(values[j]);
                if (other != nullptr) {
                    printf("WARNING: Ambiguous parameter name/value %s\n", values[j]);
					fflush(stdout);
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////
//
// Parameter
//
//////////////////////////////////////////////////////////////////////

// do we really need a no-arg constructor?
/*
Parameter::Parameter()
{
    init();
    Trace(1, "Parameter::Parameter\n");
}
*/

Parameter::Parameter(const char* name, int key) :
    SystemConstant(name, key)
{
    // if we decide we only need one constructor
    // don't need an init() method
    // modern way for this is to have this be in the class member initializers
    // anyway
    init();
}

void Parameter::init()
{
	bindable = false;
	dynamic = false;
    deprecated = false;
    transient = false;
    resettable = false;
    scheduled = false;
    takesAction = false;
    control = false;

	type = TYPE_INT;
	scope = PARAM_SCOPE_NONE;
	low = 0;
	high = 0;
    zeroCenter = false;
    mDefault = 0;

	values = nullptr;
	valueKeys = nullptr;
	valueLabels = nullptr;
    xmlAlias = nullptr;

	for (int i = 0 ; i < MAX_PARAMETER_ALIAS ; i++)
	  aliases[i] = nullptr;

    // add to the global registry
    Parameters.push_back(this);
}

Parameter::~Parameter()
{
}

void Parameter::addAlias(const char* alias) 
{
    bool added = false;

	for (int i = 0 ; i < MAX_PARAMETER_ALIAS ; i++) {
        if (aliases[i] == nullptr) {
            aliases[i] = alias;
            added = true;
            break;
        }
    }

    if (!added)
      Trace(1, "Alias overflow: %s\n", alias);
}

/**
 * Must be overloaded in the subclass.
 */
void Parameter::getObjectValue(void* object, class ExValue* value)
{
    Trace(1, "Parameter %s: getObjectValue not overloaded!\n",
          getName());
}

/**
 * Must be overloaded in the subclass.
 */
void Parameter::setObjectValue(void* object, class ExValue* value)
{
    Trace(1, "Parameter %s: setObjectValue not overloaded!\n",
          getName());
}

// defer migration of Export and Action
#if 0
void Parameter::getValue(Export* exp, ExValue* value)
{
    Trace(1, "Parameter %s: getValue not overloaded!\n",
          getName());
	value->setString("");
}

int Parameter::getOrdinalValue(Export* exp)
{
    Trace(1, "Parameter %s: getOrdinalValue not overloaded! \n",
          getName());
    return -1;
}

void Parameter::setValue(Action* action)
{
    Trace(1, "Parameter %s: setValue not overloaded!\n",
          getName());
}
#endif

/**
 * Refresh the cached display names from the message catalog.
 * This overloads the one inherited from SystemConstant so we
 * can avoid warning about hidden and deprecated parameters.
 * Push that down to SysetmConstant?
 *
 * We also handle the localization of the values.
 *
 * !! This needs work
 * We've got static objects now that auto destruct, but this
 * won't handle names pulled out of the catalog.  Need to teach
 * ~Parameter about this or just stop using the message catalog
 * and refine if and how we do localication
 */
void Parameter::localize(MessageCatalog* cat)
{
    int key = getKey();

	if (key == 0) {
		if (bindable)
		  Trace(1, "No catalog key for parameter %s\n", getName());
		setDisplayName(getName());
	}
	else {
		const char* msg = cat->get(key);
		if (msg != nullptr)
		  setDisplayName(msg);
		else {
			Trace(1, "No localization for parameter %s\n", getName());
			setDisplayName(getName());
		}
	}

	if (valueKeys != nullptr) {
		// note that these will leak if we don't have something to flush them
		if (valueLabels == nullptr) {
			int count = 0;
			while (valueKeys[count] != 0) count++;
			valueLabels = allocLabelArray(count);
		}
		for (int i = 0 ; valueKeys[i] != 0 ; i++) {
			const char* msg = cat->get(valueKeys[i]);
			if (msg != nullptr)
			  valueLabels[i] = msg;
			else {
				Trace(1, "No localization for parameter %s value %s\n", 
					  getName(), values[i]);
				if (valueLabels[i] == nullptr)
				  valueLabels[i] = values[i];
			}
		}
	}
}

/**
 * Allocate a label array and fill it with nulls.
 */
const char** Parameter::allocLabelArray(int size)
{
	int fullsize = size + 1; // leave a null terminator
	const char** labels = new const char*[fullsize];
	for (int i = 0 ; i < fullsize ; i++)
	  labels[i] = nullptr;

	return labels;
}

//////////////////////////////////////////////////////////////////////
//
// Default Ordinal mapping for the UI
// A few classes overload these if they don't have a fixed enumeration.
//
//////////////////////////////////////////////////////////////////////

int Parameter::getLow()
{
    return low;
}

// this shit is only necessary for group count which we get from
// a global config but go through MobiusInterface to get it
// make this more direct
/*
int Parameter::getHigh(MobiusInterface* m)
{
    int max = high;

    if (type == TYPE_BOOLEAN) {
        max = 1;
    }
    else if (valueLabels != nullptr) {
        for ( ; valueLabels[max] != nullptr ; max++);
        max--;
    }

    return max;
}

int Parameter::getBindingHigh(MobiusInterface* m)
{
    int max = getHigh(m);

    // if an int doesn't have a max, give it something so we can
    // have a reasonable upper bound for CC scaling
    if (type == TYPE_INT && max == 0)
      max = 127;

    return max;
}
*/

/**
 * Given an ordinal, map it into a display label.
 */
// find a way to avoid MobiusInterface here
/*
void Parameter::getOrdinalLabel(MobiusInterface* m, 
                                       int i, ExValue* value)
{
	if (valueLabels != nullptr) {
		value->setString(valueLabels[i]);
	}
	else if (type == TYPE_INT) {
		value->setInt(i);
	}
    else if (type == TYPE_BOOLEAN) {
		value->setString(BOOLEAN_VALUE_LABELS[i]);
	}
    else 
	  value->setInt(i);
}

void Parameter::getDisplayValue(MobiusInterface* m, ExValue* value)
{
    // weird function used in just a few places by
    // things that overload getOrdinalLabel
    value->setNull();
}
*/

//////////////////////////////////////////////////////////////////////
//
// coersion utilities
//
//////////////////////////////////////////////////////////////////////

/**
 * Convert a string value to an enumeration ordinal value.
 * This is the one used by most of the code, if the name doesn't match
 * it traces a warning message and returns the first value.
 */
int Parameter::getEnum(const char *value)
{
	int ivalue = getEnumValue(value);

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
int Parameter::getEnumValue(const char *value)
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
 * Check for an enumeration value that has been changed and convert
 * the old name from the XML or script into the new name.
 */
void Parameter::fixEnum(ExValue* value, const char* oldName, 
                               const char* newName)
{
	if (value->getType() == EX_STRING) {
        const char* current = value->getString();
        if (StringEqualNoCase(oldName, current))
          value->setString(newName);
    }
}

/**
 * Convert a Continuous Controller number in the range of 0-127
 * to an enumerated value.
 * !! this isn't used any more, if we're going to do scaling
 * it needs to be done in a way appropriate for the binding.
 */
int Parameter::getControllerEnum(int value)
{
	int ivalue = 0;

	if (value >= 0 && value < 128) {
		int max = 0;
		for (max = 0 ; values[max] != nullptr ; max++);

		int unit = 128 / max;
		ivalue = value / unit;
	}

	return ivalue;
}

/**
 * Coerce an ExValue into an enumeration ordinal.
 * This must NOT scale, it is used in parameter setters
 * and must be symetrical with getOrdinalValue.
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
			if (values != nullptr)
			  for (max = 0 ; values[max] != nullptr ; max++);

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

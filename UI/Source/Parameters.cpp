/**
 * Parameter definitions
 */

#include <vector>

#include "Parameters.h"

Parameter::Parameter()
{
    name = "unknown";
    displayName = nullptr;
    type = TypeInt;
    scope = ScopeNone;
    low = 0;
    high = 127;
    value = 0;
    defaultValue = 0;
    
    bindable = false;
    dynamic = false;
    deprecated = false;
    transient = false;
    resettable = false;
    scheduled = false;
    takesAction = false;
    control = false;
    zeroCenter = false;

    // add to the global registry
    Parameters.push_back(this);
}

Parameter::Parameter(const char* argName, const char* argDisplayName)
{
    name = argName;
    displayName = argDisplayName;
}

Parameter::~Parameter()
{
}

/**
 * Global registry of parameters.
 * Since these are all created statically don't need to worry
 * about lifespan of the objects.
 */
std::vector<Parameter*> Parameter::Parameters;

//////////////////////////////////////////////////////////////////////
//
// Preset Parameters
//
//////////////////////////////////////////////////////////////////////

class PresetParameter : public Parameter 
{
  public:
    PresetParameter(const char* name, const char* displayName) :
        Parameter(name, displayName) {
        scope = ScopePreset;
    }
};

    
class SubCycleParameterType : public PresetParameter 
{
  public:
	SubCycleParameterType() :
        PresetParameter("subcycles", "Sub Cycles")
    {
        bindable = true;
        type = TypeInt;
        low = 1;
        high = 128;
        // addAlias("8thsPerCycle");
    }
};

SubCycleParameterType SubCycleParamter;

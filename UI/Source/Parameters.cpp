/**
 * Parameter definitions
 */

#include "Parameters.h"

Parameter::Parameter()
{
    name = nullptr;
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
}

Parameter::~Parameter()
{
}

//////////////////////////////////////////////////////////////////////
//
// Preset Parameters
//
//////////////////////////////////////////////////////////////////////

class SubCycleParameterType : public Parameter
{

  public:
    
	SubCycleParameterType() {
        name = "subcycles";
        displayName = "Sub Cyles";
        type = TypeInt;
        low = 1;
        high = 128;
        // addAlias("8thsPerCycle");
    }
    
};

SubCycleParameterType SubCycleParameter;


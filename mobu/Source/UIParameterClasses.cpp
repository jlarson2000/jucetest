/*
 * Extensions of UIParameter containing specific parameter code.
 * This file is generated and must not be modified.
 */

#include <JuceHeader.h>

#include "ExValue.h"
#include "Preset.h"

#include "UIParameter.h"

// Example
//
// This is the original syntax for parameter definition
// and should be improved with modern sensibilities
//
// Old code used these naming conventions and we must not confict:
//
//   class Parameter : public SystemConstant
//   class PresetParameter : public Parameter
//   class SubCycleParameterType : public PresetParameter
//   Parameter* SubCycleParameter = new SubCycleParameterType();
//   Parameter* Parameters[MAX_STATIC_PARAMETERS];
//   extern Parameter* AltFeedbackEnableParameter;
//
// To prevent memory leaks we're either going to have to use
// the static object/pointer convention, or have a shutdown()
// hook that deletes everything 

class UIParameterSubcyclesClass : public UIParameter
{
  public:
	UIParameterSubcyclesClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};

// note could avoid having to pass this if SystemCostant
// were smart about capitalization conventions
// but it matters less for generated code

UIParameterSubcyclesClass::UIParameterSubcyclesClass()
{
    name = "subcycles";
    displayName = "Subcycles";
    scope = ScopePreset;
    type = TypeInt;
    low = 1;
    high = 128;
}

void UIParameterSubcyclesClass::getValue(void* obj, ExValue* value)
{
	value->setInt(((Preset*)obj)->getSubcycles());
}

void UIParameterSubcyclesClass::setValue(void* obj, ExValue* value)
{
	((Preset*)obj)->setSubcycles(value->getInt());
}

UIParameterSubcyclesClass UIParameterSubcyclesObj;
UIParameter* UIParameterSubcycles = &UIParameterSubcyclesObj;

// !GENERATE

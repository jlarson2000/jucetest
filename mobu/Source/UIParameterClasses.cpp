/*
 * Extensions of UIParameter containing specific parameter code.
 * This file is generated below the GENERATED token and must not be
 * modified.
 * 
 * This is the original syntax for parameter definition
 * and should be improved with modern sensibilities
 * Old code used these naming conventions and we must not confict:
 * 
 *   class Parameter : public SystemConstant
 *   class PresetParameter : public Parameter
 *   class SubCycleParameterType : public PresetParameter
 *   Parameter* SubCycleParameter = new SubCycleParameterType();
 *   Parameter* Parameters[MAX_STATIC_PARAMETERS];
 *   extern Parameter* AltFeedbackEnableParameter;
 *
 * To prevent memory leaks we're either going to have to use
 * the static object/pointer convention, or have a shutdown()
 * hook that deletes everything 
 */

#include <JuceHeader.h>

#include "ExValue.h"
#include "Preset.h"

#include "UIParameter.h"

// !GENERATE

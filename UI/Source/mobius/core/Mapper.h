
// stubs for various object mapping utilities

#pragma once

#include "../../model/MobiusState.h"

extern class UIEventType* MapEventType(class EventType* src);

extern class FunctionDefinition* MapFunction(class Function* src);

extern class Parameter* MapParameter(class UIParameter* src);

//
// config objects don't keep a cached pointer to these
// any more
//

extern class Preset* GetCurrentPreset(class MobiusConfig* src);
extern class Setup* GetCurrentSetup(class MobiusConfig* src);

extern class Preset* GetPreset(class MobiusConfig* src, int number);
extern class Preset* GetPreset(class MobiusConfig* src, const char* name);

extern class Setup* GetSetup(class MobiusConfig* src, int number);
extern class Setup* GetSetup(class MobiusConfig* src, const char* name);

extern int GetSetupCount(class MobiusConfig* src);

extern class ModeDefinition* MapMode(class MobiusMode* mode);

// replacement for Parameter::getBindingHigh(MobiusInterface)
extern int GetBindingHigh(class Parameter* p, class Mobius* m);
// replacement for MobiusInterface
extern void GetOrdinalLabel(class Parameter* p, class Mobius* m, int ordinal, class ExValue* value);
// MobiusInterface
extern int GetHigh(class Parameter* p, class Mobius* m);

extern void SetActiveSetup(class MobiusConfig* config, int number);

extern void SleepMillis(int);
    

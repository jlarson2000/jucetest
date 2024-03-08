/**
 * Temporary model mapping functions.
 */

#pragma once

extern void SleepMillis(int);
extern bool IsPlugin(class Mobius* m);

extern class UIEventType* MapEventType(class EventType* src);
extern class FunctionDefinition* MapFunction(class Function* src);
extern class UIParameter* MapParameter(class Parameter* src);
extern class ModeDefinition* MapMode(class MobiusMode* mode);

extern void WriteAudio(class Audio* a, const char* path);
extern void WriteFile(const char* path, const char* content);

//
// config objects don't keep a cached pointer to these any more
//

extern class Preset* GetCurrentPreset(class MobiusConfig* src);
extern class Setup* GetCurrentSetup(class MobiusConfig* src);
extern void SetCurrentSetup(class MobiusConfig* config, int number);

//
// Object searchers moved to Structure
//

extern class Preset* GetPreset(class MobiusConfig* src, int number);
extern class Preset* GetPreset(class MobiusConfig* src, const char* name);
extern int GetPresetCount(class MobiusConfig* src);

extern class Setup* GetSetup(class MobiusConfig* src, int number);
extern class Setup* GetSetup(class MobiusConfig* src, const char* name);
extern int GetSetupCount(class MobiusConfig* src);

/**
 * Experiment with "intrinsic" functions that are like Functions
 * but don't have a concrete Function model underneath.
 *
 * They at minimum have a name, and optionally can have an ordinal.
 * Once they get an ordinal then we've got a model really but at least
 * it's simpler than Function.
 *
 */

#pragma once

// Pick a base ordinal that is high enough to not be usable within
// the Function table.  Not really necessary but reinforces that this
// is special in the debugger.

typedef enum {

    /**
     * Passes a MobiusConfiguration object.
     * Ownership of the object passes to the receiver.
     */
    IntrinsicBase = 1000,
    IntrinsicLoadScripts,
    IntrinsicLoadSamples

} IntrinsicFunctions;

/**
 * Need a home for this method, so this may a well become
 * a definitional model too.
 */
class Intrinsic
{
  public:

    // need names for these if they can be bound in the UI which
    // means we're one more away from a concrete model

    inline static const char* LoadScriptsName = "Load Scripts";
    inline static const char* LoadSamplesName = "Load Samples";

    // add the intrinsic functions to the list of DynamicActions
    // being returned to the UI
    static void addIntrinsics(class DynamicConfig* config);

};


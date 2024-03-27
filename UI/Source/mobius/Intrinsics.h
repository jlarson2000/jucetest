/**
 * Experiment with "intrinsic" functions that are like Functions
 * but don't have a concrete Function model underneath.
 *
 * This is starting to flesh out some concepts I want to start using
 * for all system constant objects that have complex definitions we don't
 * want to always have defined as static objects.  This gives the
 * option to use a code generator to generate implementation classes or
 * "handler" classes that implement several things in one class.
 *
 * Mostly this applies to Functions and Parameters, but can extend this
 * to other currently static objects like Mode, and others.
 *
 * Fundamentaly a system constant must have two things that must be static
 * and can be resolved at compile time.
 *
 *       id
 *       name
 *
 * The id is a small contiguous range of integers suitable for use
 * in an optimized switch statement.  It is the foundation of a "jump table"
 * used for fast association between a reference to something and the
 * implementation of that something.  The reference needs to be availalbe
 * at compile time for parts of the system, notably the UI, that need access
 * to something but must know how it is implemented.
 *
 * The name is a short symbolic name that can be used to reference the thing
 * in places where remembering numeric ids is error prone.  For us this is
 * action bindings in the UI (e.g. MIDI note 54 calls the "Record" function)
 * and in scripts (e.g. "set subcycles 4")
 *
 * Everything else about the abstract thing can be created dynamically at
 * runtime.
 *
 * This concept is particulary useful for Parameters and Functions since
 * have many of them, and defining a concrete implementation class for all
 * of them is tedious and a maintenance headache.  Intrinsics are a step
 * toward a new world where all compiled code needs is an ID number to
 * cause something to happen, and all scripts need to know is the name.
 *
 * More on this in design notes as it evolves...
 *
 */

#pragma once

#include <JuceHeader.h>

/**
 * Intrinsic objects have an ID defined in an enum which are
 * widely supported by optimizing compilers to produce a fast
 * jump table.
 *
 * I'm giving this a base number to prevent collisions with
 * ordinals of the old static objects still in use.  This may
 * confuse some compilers, unclear.
 *
 * Could put this in the Intrinsic class for namespacing but
 * I'm liking keeping this as simple as possible.
 */
typedef enum {

    IntrinsicBase = 1000,
    IntrinsicInvalid,
    IntrinsicLoadScripts,
    IntrinsicLoadSamples,
    IntrinsicAnalyzeDiff,

} IntrinsicId;

/**
 * Mapping between intrinsic IDs and symbolic names.
 * Again the simplest possible data structure is used to avoid
 * code clutter.  You don't need defined constant name literals.
 * Consider a std::vector here
 */
// ugh, wanted to have this here so the enums and the names could be
// together but then we have multiple definition errors, work on this...
/*
const char* IntrinsicFunctionNames[] = {
    "loadScripts",
    "loadSamples",
    "testDiff",
    nullptr
};
*/

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
    inline static const char* AnalyzeDiffName = "Analyze Diff";

    static void init();

    static IntrinsicId Intrinsic::getId(juce::String name);

    // add the intrinsic functions to the list of DynamicActions
    // being returned to the UI
    static void addIntrinsics(class DynamicConfig* config);

  private:

    static juce::OwnedArray<class DynamicAction> actions;

};

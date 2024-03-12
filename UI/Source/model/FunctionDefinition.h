// comments need revisiting
// need to split out "constant" vs. "dynamic" functions so
// some can be opaque and run with an invoke() method
// and other have global constant pointers so they can be
// tested without string comparison

/* 
 * Model for function definitions.
 *
 * Functions are commands that can be sent to the engine.
 * They differ from Parameters in that they do not have values
 * and cannot be configured.
 *
 * TODO: Check VST3 and see if plugins have a similar concept that
 * could be used these days.
 *
 * Unlike Parameter, the model has been split between BaseFunction
 * which has only the information necessary for the UI (bindings and actions)
 * and the original model used by the engine.
 *
 * The engine constants will subclass the base definitions.
 * In cases where we need to map between implementation objects can
 * use ordinals.
 *
 * Unlike Parameter I don't think we need global pointers to
 * implementation objects, just keep them in a vector for UI iteration
 * and resolve them by name at runtime where necessary.
 */

#pragma once

#include <vector>

#include "SystemConstant.h"

//////////////////////////////////////////////////////////////////////
//
// FunctionDefinition
//
//////////////////////////////////////////////////////////////////////

class FunctionDefinition : public SystemConstant {

  public:
    
	FunctionDefinition(const char* name);
    virtual ~FunctionDefinition();

    // used by UIAction.cpp
    // decide if we need this
    bool isSpread() {
        return false;
    }

    //////////////////////////////////////////////////////////////////////
    // Global Function Registry
    //////////////////////////////////////////////////////////////////////

    static std::vector<FunctionDefinition*> Instances;
    static void trace();
	static FunctionDefinition* find(const char* name);

  private:

};

//////////////////////////////////////////////////////////////////////
//
// Constant Functions
//
//////////////////////////////////////////////////////////////////////

extern FunctionDefinition* SamplePlay;

//////////////////////////////////////////////////////////////////////
//
// Replicated Function
//
// Extenion used by functions that support a numeric multiplier.
// Some functions have both a set of relative and absolute functions
// so we multiply only when the replicated flag is on.
//
// This is not used yet, but may still be relevant.
// It was used by a handful of functions that can target things that
// have a configurable size.  Exammples were:
//
//    Select Track
//    Select Loop
//    Play Sample
//    Run Script
//
// For track selection, since there are a variable number of tracks
// there is no one function that says "select track 2".  We could (and did)
// hard code a default set, but doesn't work for samples and scripts which
// are completely random.  These are handled now with a single function
// that takes an argument.  Eventually might work out a way to bind them
// without the user needing to specify an argument.  But since this is really
// just syntactic sugar in the UI it doesn't need to be in the engine.
//
// 
//////////////////////////////////////////////////////////////////////

#if 0
class ReplicatedFunction : public FunctionDefinition {
  public:
	ReplicatedFunction();
  protected:
	bool replicated;
	char fullName[32];
};

/**
 * Constant for RunScriptFunction.
 */
#define MAX_SCRIPT_NAME 1024

/**
 * This is the only specific function class that we define globally
 * because Function needs it to create Function wrappers for loaded
 * scripts.
 */
class RunScriptFunction : public Function {
  public:
	RunScriptFunction(class Script* s);
	void invoke(class Action* action, class Mobius* m);
	bool isMatch(const char* xname);
  private:
	// we have to maintain copies of these since the strings the
	// Script return can be reclained after an autoload
	char mScriptName[MAX_SCRIPT_NAME];
};
#endif

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

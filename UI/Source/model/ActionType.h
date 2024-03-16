/**
 * A collection of static object that define the types of actions
 * that can be taken on the system core from the user interface.
 * This is part of the Binding and Action models but factored out
 * so they can be used at various levels without needing to know
 * where they came from.
 * 
 * They self initialize during static initialization and will
 * self destruct.
 *
 */

#pragma once

#include <vector>

#include "SystemConstant.h"

//////////////////////////////////////////////////////////////////////
//
// Types
//
//////////////////////////////////////////////////////////////////////

/**
 * Defines the type of action, or which object within
 * the system will carry out that action.
 */
class ActionType : public SystemConstant {
  public:

    static std::vector<ActionType*> Instances;
	static ActionType* find(const char* name);

	ActionType(const char* name, const char* display);
};

extern ActionType* ActionFunction;
extern ActionType* ActionParameter;
extern ActionType* ActionActivation;

// this was a weird one, used to send down notification
// of the completion of a ThreadEvent (now KernelEvent)
// we don't do this using Actions any more so this can be removed
// unless you come up with some other reason to be able to send down
// actions that target running ScriptInterpreters
extern ActionType* ActionScript;

// until we can refactor all the old uses of TargetPreset
// and decide on the right concrete model, define these
// here just so we have a place to store the names
// they aren't really ActionTypes

extern ActionType* ActionSetup;
extern ActionType* ActionPreset;
extern ActionType* ActionBindings;

//////////////////////////////////////////////////////////////////////
//
// Operators
//
//////////////////////////////////////////////////////////////////////

/**
 * Constants that describe operations that produce a relative change to
 * a control or parameter.
 */
class ActionOperator : public SystemConstant {
  public:
    
    static std::vector<ActionOperator*> Instances;
    static ActionOperator* find(const char* name);

    ActionOperator(const char* name, const char* display);
};

extern ActionOperator* OperatorMin;
extern ActionOperator* OperatorMax;
extern ActionOperator* OperatorCenter;
extern ActionOperator* OperatorUp;
extern ActionOperator* OperatorDown;
extern ActionOperator* OperatorSet;
extern ActionOperator* OperatorPermanent;

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

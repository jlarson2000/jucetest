/*
 * A model for representing actions to be taken within the Mobius
 * engine.  These are created in response to triggers.
 */

//#include <stdio.h>
//#include <string.h>
//#include <memory.h>
//#include <stdlib.h>
//#include <ctype.h>

#include "../util/Util.h"
#include "../util/Trace.h"
#include "../util/List.h"

#include "ExValue.h"

//#include "MidiByte.h"

#include "Binding.h"
#include "FunctionDef.h"
//#include "Event.h"
//#include "Script.h"
#include "Parameter.h"

#include "UIAction.h"

/****************************************************************************
 *                                                                          *
 *                              ACTION OPERATOR                             *
 *                                                                          *
 ****************************************************************************/

std::vector<ActionOperator*> ActionOperator::Operators;

ActionOperator::ActionOperator(const char* name, const char* display) :
    SystemConstant(name, display)
{
    ordinal = Operators.size();
    Operators.push_back(this);
}

/**
 * Find an Operator by name
 * This doesn't happen often so we can do a liner search.
 */
ActionOperator* ActionOperator::getOperator(const char* name)
{
	ActionOperator* found = nullptr;
	
    // todo: need to support display names?
	for (int i = 0 ; i < Operators.size() ; i++) {
		ActionOperator* op = Operators[i];
		if (StringEqualNoCase(op->getName(), name)) {
            found = op;
            break;
        }
	}
	return found;
}

TriggerMode TriggerModeContinuousObj{"continuous", "Continuous"};
TriggerMode* TriggerModeContinuous = &TriggerModeContinuousObj;

ActionOperator OperatorMinObj{"min", "Minimum"};
ActionOperator* OperatorMin = &OperatorMinObj;

ActionOperator OperatorMaxObj{"max", "Maximum"};
ActionOperator* OperatorMax = &OperatorMaxObj;

ActionOperator OperatorCenterObj{"center", "Center"};
ActionOperator* OperatorCenter = &OperatorCenterObj;

ActionOperator OperatorUpObj{"up", "Up"};
ActionOperator* OperatorUp = &OperatorUpObj;

ActionOperator OperatorDownObj{"down", "Down"};
ActionOperator* OperatorDown = &OperatorDownObj;

ActionOperator OperatorSetObj{"set", "Set"};
ActionOperator* OperatorSet = &OperatorSetObj;

ActionOperator OperatorPermanentObj{"permanent", "Permanent"};
ActionOperator* OperatorPermanent = &OperatorPermanentObj;

//////////////////////////////////////////////////////////////////////
//
// Target Cache
//
//////////////////////////////////////////////////////////////////////

/**
 * Union of possible target pointers.
 * This is set during the resolution of the symbolic references
 * in the Binding (or UIButton) objects to the concrete objects
 * that are named.
 *
 * This is all that remains of the old ResolvedTarget concept.
 *
 * While the code deals with these as a void* and casts 
 * when necessary, it is nice in the debugger to have these
 * in a union so we can see what they are.
 *
 * Formerly had Bindable in here but I didn't like ugly cache
 * invalidation when the configuration objects are replaced.
 * Until it seems necessary, just remember the name and look them
 * up at runtime.  
 */
typedef union {

    void* object;
    class FunctionDefinition* function;
    class Parameter* parameter;
    //class Bindable* bindable;
    class UIControl* uicontrol;

} TargetPointer;

/****************************************************************************
 *                                                                          *
 *                                   ACTION                                 *
 *                                                                          *
 ****************************************************************************/

void UIAction::init()
{
    // Trigger
    id = 0;
    trigger = nullptr;
    triggerMode = nullptr;
    passOscArg = false;
    triggerValue = 0;
    triggerOffset = 0;
    down = false;
    repeat = false;
    longPress = false;

    // Target, Scope
    mInternedTarget = nullptr;
    //mResolvedTrack = nullptr;
    mLongFunction = nullptr;

    // Time
    escapeQuantization = false;
    noLatency = false;
    noSynchronization = false;

    // Arguments
    bindingArgs[0] = 0;
    actionOperator = nullptr;
    arg.setNull();
    scriptArgs = nullptr;

    // Status
    //rescheduling = nullptr;
    //reschedulingReason = nullptr;
    //mobius = nullptr;
    
    inInterrupt = false;
    noGroup = false;
    noTrace = false;
    millisecond = 0;
    streamTime = 0.0f;

    // private

    mNext = nullptr;
    mPooled = false;
    mRegistered = false;

    //mEvent = nullptr;
    //mThreadEvent = nullptr;

    mOverlay = 0;
    mName = nullptr;
}

UIAction::UIAction()
{
    init();
}

UIAction::UIAction(UIAction* src)
{
    init();
    if (src != nullptr)
      clone(src);
}

UIAction::UIAction(ResolvedTarget* t)
{
    init();
    mInternedTarget = t;
}

/**
 * We own nothing except the chain pointer.
 * scrptArgs is transient and owned by the script interpreter that
 * built the action
 */
UIAction::~UIAction()
{
    if (mRegistered)
      Trace(1, "Atttempt to delete registered action!\n");

    // scriptArgs is dynamically allocated and must be freed
    delete scriptArgs;
    delete mName;

	UIAction *el, *next;
	for (el = mNext ; el != nullptr ; el = next) {
		next = el->getNext();
		el->setNext(nullptr);
		delete el;
	}
}

/**
 * Return an action to it's pool.
 */
void UIAction::free()
{   
    if (mPool != nullptr)
      mPool->freeUIAction(this);
    else {
        // let this be okay
        //Trace(1, "UIAction::free with no pool!\n");
        delete this;
    }
}

/**
 * Reset clears a previously initialized action.
 * The difference here is that we have to release the
 * scriptArgs.
 */
void UIAction::reset()
{
    delete scriptArgs;
    init();
}

/**
 * New method to format a concise description of the action.
 * Used during initial testing.  String must not be freed.
 * Use an internal buffer for now as this evolves.
 * Note that this is different than getName() which is something
 * related to OSC, should change the method name to make this clear.
 */
const char* UIAction::getDescription()
{
    // need more...
    strcpy(mDescription, "Action of destiny!");
    return &mDescription;
}

const char* UIAction::getName()
{
    return mName;
}

void UIAction::setName(const char* name)
{
    delete mName;
    mName = CopyString(name);
}

/**
 * Note that this is called instead of reset() when returning
 * something from the pool for cloning so we must initialize
 * every field!
 */
void UIAction::clone(UIAction* src)
{
    //mobius = src->mobius;

    // assume names don't need to convey

    // Trigger
    id = src->id;
    trigger = src->trigger;
    triggerMode = src->triggerMode;
    passOscArg = src->passOscArg;
    triggerValue = src->triggerValue;
    triggerOffset = src->triggerOffset;
    down = src->down;
    repeat = src->repeat;
    longPress = src->longPress;

    // Target, Scope
    // take the private target if we have one
    mInternedTarget = src->mInternedTarget;
    mPrivateTarget.clone(&(src->mPrivateTarget));
    mLongFunction = src->mLongFunction;

    // Should we clone these?  They're supposed to be transient!
    //mResolvedTrack = src->mResolvedTrack;
    noGroup = src->noGroup;
    noTrace = src->noTrace;

    // Time
    escapeQuantization = src->escapeQuantization;
    noLatency = src->noLatency;
    noSynchronization = src->noSynchronization;

    // Arguments
    strcpy(bindingArgs, src->bindingArgs);
    actionOperator = src->actionOperator;
    arg.set(&(src->arg));

    // Long script args are NOT cloned.  Since script actions 
    // are created on the fly we do not need to clone an interned
    // UIAction.  We could but we would have to copy the ExValueList
    // which is a pain.
    //scriptArgs = someKindOfCopy(src->scriptArgs);
    if (src->scriptArgs != nullptr)
      Trace(1, "Cloning action with script arguments!\n");
    delete scriptArgs;
    scriptArgs = nullptr;

    // relevant runtime status
    //inInterrupt = src->inInterrupt;

    // are these really necessary?
    //millisecond = src->millisecond;
    //streamTime = src->streamTime;

    // absolutely not these
    //mEvent = nullptr;
    //mThreadEvent = nullptr;

    // !! not sure, probably should reset
    //rescheduling = src->rescheduling;
    //reschedulingReason = src->reschedulingReason;

    // mNext and mPooled maintained by the pool functions

    // mRegistered and mOverlay are not cloned, they are only used
    // by BindingResolver for actions we do clone
    mRegistered = false;
    mOverlay = 0;
}
 
bool UIAction::isSustainable()
{
    return (triggerMode == TriggerModeMomentary ||
            triggerMode == TriggerModeToggle);
}

void UIAction::setPooled(bool b)
{
    mPooled = b;
}

bool UIAction::isPooled()
{
    return mPooled;
}

void UIAction::setPool(UIActionPool* p)
{
    mPool = p;
}

UIAction* UIAction::getNext() 
{
    return mNext;
}

void UIAction::setNext(UIAction* a)
{
    mNext = a;
}

bool UIAction::isRegistered() 
{
    return mRegistered;
}

void UIAction::setRegistered(bool b)
{
    mRegistered = b;
}

int UIAction::getOverlay()
{
    return mOverlay;
}

void UIAction::setOverlay(int i)
{
    mOverlay = i;
}

bool UIAction::isResolved()
{
    return (getTargetObject() != nullptr);
}

ResolvedTarget* UIAction::getResolvedTarget()
{
    ResolvedTarget* t = mInternedTarget;
    if (t == nullptr)
      t = &mPrivateTarget;
    return t;
}

Target* UIAction::getTarget()
{
    ResolvedTarget* rt = getResolvedTarget();
    return (rt != nullptr) ? rt->getTarget() : nullptr;
}

void* UIAction::getTargetObject()
{
    ResolvedTarget* rt = getResolvedTarget();
    return (rt != nullptr) ? rt->getObject() : nullptr;
}

int UIAction::getTargetTrack()
{
    ResolvedTarget* rt = getResolvedTarget();
    return (rt != nullptr) ? rt->getTrack() : 0;
}

int UIAction::getTargetGroup()
{
    ResolvedTarget* rt = getResolvedTarget();
    return (rt != nullptr) ? rt->getGroup() : 0;
}

/**
 * If the action has bindingArgs, parse them into an
 * ActionOperator and argument value.
 */
void UIAction::parseBindingArgs()
{
    if (strlen(bindingArgs) > 0) {
        actionOperator = nullptr;
        
        char* psn = bindingArgs;

        // skip leading
        psn = advance(psn, false);

        // find end of token
        char* end = advance(psn, true);

        // terminate it to search for operators
        char save = *end;
        *end = 0;

        actionOperator = ActionOperator::getOperator(psn);
        if (actionOperator != nullptr) {
            // skip to the operand
            *end = save;
            psn = end;
            psn = advance(psn, false);
            end = advance(psn, true);
            *end = 0;
        }

        if (strlen(psn) > 0) {
            if (IsInteger(psn))
              arg.setInt(atoi(psn));
            else
              arg.setString(psn);
        }

        // leave this empty so we don't do it again
        bindingArgs[0] = 0;
    }
}

/**
 * Helper for parseBindingArgs
 */
char* UIAction::advance(char* start, bool stopAtSpace)
{
    while (*start) {
        char ch = *start;
        if ((isspace(ch) != 0) == stopAtSpace)
          break;
        else
          start++;
    }

    return start;
}

/**
 * Return true if our target is the same as another.
 * The action must be resolved by now.
 * Used by BindingResolver to filter redundant bindings.
 */
bool UIAction::isTargetEqual(UIAction* other)
{
    // ugh, names and indirection suck
    return (getTarget() == other->getTarget() &&
            getTargetObject() == other->getTargetObject() &&
            getTargetTrack() == other->getTargetTrack() &&
            getTargetGroup() == other->getTargetGroup());
}

/**
 * Dynamically set a target.
 * This should only be used for a small number of internally
 * constructed actions.
 */
void UIAction::setTarget(Target* t)
{
    setTarget(t, nullptr);
}

void UIAction::setTarget(Target* t, void* object)
{
    // we may have started with an interned target, switch
    mInternedTarget = nullptr;
    mPrivateTarget.setTarget(t);
    mPrivateTarget.setObject(object);
}

/**
 * Dynamically set a target function.
 * This is used when building UIActions on the fly rather than from Bindings.
 * This can only be used with static functions, you can't use this
 * for scripts, those are only accessible through ResolvedTargets.
 */
void UIAction::setFunction(FunctionDefinition* f)
{
    setTarget(TargetFunction, f);
}

FunctionDefinition* UIAction::getFunction()
{
    FunctionDefinition* f = nullptr;
    if (getTarget() == TargetFunction)
      f = (FunctionDefinition*)getTargetObject();
    return f;
}

void UIAction::setLongFunction(FunctionDefinition* f)
{
    mLongFunction = f;
}

FunctionDefinition* UIAction::getLongFunction()
{
    return mLongFunction;
}

/**
 * Dynamically set a target parameter.
 * This is used when building UIActions on the fly rather than from Bindings.
 */
void UIAction::setParameter(Parameter* p)
{
    setTarget(TargetParameter, p);
}

/**
 * Convenience method for things that create UIActions on the fly with 
 * function targets.
 *
 * Note that the track argument is 1 based like a Binding.
 * This does not switch from mInternedTarget to mPrivate target,
 * you need to call setTarget() first.
 */
void UIAction::setTargetTrack(int track)
{
    mPrivateTarget.setTrack(track);
}

void UIAction::setTargetGroup(int group)
{
    mPrivateTarget.setGroup(group);
}

/**
 * When actions are processed internally we use this to
 * force it to a certain track.
 */
#if 0
void UIAction::setResolvedTrack(Track* t)
{
    mResolvedTrack = t;
}

Track* UIAction::getResolvedTrack()
{
    return mResolvedTrack;
}

Event* UIAction::getEvent()
{
    return mEvent;
}

ThreadEvent* UIAction::getThreadEvent()
{
    return mThreadEvent;
}

void UIAction::setThreadEvent(ThreadEvent* te)
{
    mThreadEvent = te;
}
#endif

/****************************************************************************
 *                                                                          *
 *                                 UTILITIES                                *
 *                                                                          *
 ****************************************************************************/

/**
 * Get the MIDI status code from the action id.
 * Format: ((status | channel) << 8) | key
 *
 * We expect these to be MS_ constaints so zero out the channel.
 */
int UIAction::getMidiStatus()
{
    return ((id >> 8) & 0xF0);
}

/**
 * Get the MIDI status code from the action id.
 * Format: ((status | channel) << 8) | key
 *
 * We expect the argument to be an MS_ constaints so it is
 * already 8 bits with a zero channel.
 */
void UIAction::setMidiStatus(int i)
{
    id = ((i << 8) | (id & 0xFFF));
}

/**
 * Get the MIDI channel from the action id.
 * Format: ((status | channel) << 8) | key
 */
int UIAction::getMidiChannel()
{
    return ((id >> 8) & 0xF);
}

void UIAction::setMidiChannel(int i)
{
    id = ((i << 8) | (id & 0xF0FF));
}

/**
 * Get the MIDI key, program, or CC number from the action id.
 * Format: ((status | channel) << 8) | key
 */
int UIAction::getMidiKey()
{
    return (id & 0xFF);
}

void UIAction::setMidiKey(int i)
{
    id = (i | (id & 0xFF00));
}

/**
 * Return true if this action is bound to a function or script that
 * supports spreading.
 */
bool UIAction::isSpread() 
{
	bool spread = false;
    if (getTarget() == TargetFunction) {
        FunctionDefinition* f = (FunctionDefinition*)getTargetObject();
        if (f != nullptr)
          spread = f->isSpread();
    }
	return spread;
}

/**
 * Calculate a display name for this action.
 * Used in the KeyHelp dialog, possibly others.
 */
void UIAction::getDisplayName(char* buffer, int max)
{
    // TODO: add a trigger prefix!
    buffer[0] = 0;

    if (mInternedTarget != nullptr) {
        mInternedTarget->getFullName(buffer, max);

        if (strlen(bindingArgs) > 0) {
            // unparsed, unusual
            AppendString(" ", buffer, max);
            AppendString(bindingArgs, buffer, max);
        }
        else {
            // already parsed
            if (actionOperator != nullptr && 
                actionOperator != OperatorSet) {
                AppendString(" ", buffer, max);
                AppendString(actionOperator->getName(), buffer, max);
            }

            if (!arg.isNull()) {
                AppendString(" ", buffer, max);
                int start = strlen(buffer);
                arg.getString(&buffer[start], max - start);
            }
        }
    }
}

#if 0
/**
 * Set the event that owns this action, checking for error conditions.
 * Check a bunch of "not supposed to happen" integrity constraints to
 * find bugs.
 */
void UIAction::setEvent(Event* e)
{
    if (e != nullptr) {
        if (mEvent != nullptr) {
            if (mEvent != e) {
                Trace(1, "UIAction already owned by another event!\n");
                // steal it?
            }
            else {
                Trace(1, "UIAction already owned by this event!\n");
                if (e->getUIAction() != this) {
                    Trace(1, "UIAction/Event reference not circular!\n");
                    e->setUIAction(this);
                }
            }
        }
        else if (e->getUIAction() != nullptr) {
            if (e->getUIAction() != this) {
                Trace(1, "Event already owns another action!\n");
                // steal it?
            }
            else if (e->getUIAction() == this) {
                Trace(1, "Event already owns this action!\n");
                if (mEvent != e) {
                    Trace(1, "Event/UIAction reference not circular!\n");
                    mEvent = e;
                }
            }
        }
        else {
            // we hope to be here
            e->setUIAction(this);
            mEvent = e;
        }
    }
}

/**
 * Move ownership of the UIAction from one event to another.
 * Again we're being really careful about detecting structure errors,
 * since event/action relationships gets fuzzy in complex cases like
 * Multiply/Insert modes and loop switch.
 */
void UIAction::changeEvent(Event* e)
{
    detachEvent(mEvent);
    setEvent(e);
}

/**
 * Remove the relationship between an action and event.
 */
void UIAction::detachEvent(Event* e)
{
    if (e != nullptr && mEvent != e)
      Trace(1, "detachEvent: expected event not attached!\n");
    
    if (mEvent != nullptr) {
        if (mEvent->getUIAction() != this)
          Trace(1, "detachEvent: Current event doesn't own this action!\n");
        mEvent->setUIAction(nullptr);
        mEvent = nullptr;
    }
}

void UIAction::detachEvent()
{
    detachEvent(mEvent);
}
#endif

/****************************************************************************
 *                                                                          *
 *                                ACTION POOL                               *
 *                                                                          *
 ****************************************************************************/

UIActionPool::UIActionPool()
{
    mUIActions = nullptr;
    mAllocated = 0;
}

UIActionPool::~UIActionPool()
{
    delete mUIActions;
}

/**
 * Allocate a new action, using the pool if possible.
 * Note that this is not managed a csect, it should only be called
 * by Mobius which uses maintains a single app/interrupt coordination
 * csect.
 */
UIAction* UIActionPool::newUIAction()
{
    return allocUIAction(nullptr);
}

UIAction* UIActionPool::newUIAction(UIAction* src)
{
    return allocUIAction(src);
}

UIAction* UIActionPool::allocUIAction(UIAction* src)
{
    UIAction* action = mUIActions;

    if (action == nullptr) {
        action = new UIAction(src);
        action->setPool(this);
        mAllocated++;
    }
    else {
        mUIActions = action->getNext();
        action->setNext(nullptr);
        action->setPooled(false);
        if (src != nullptr)
          action->clone(src);
        else
          action->reset();
    }

    return action;
}

void UIActionPool::freeUIAction(UIAction* action)
{
    if (action != nullptr) {
        if (action->isPooled())
          Trace(1, "Ignoring attempt to free pooled action\n");
        else {
            action->setNext(mUIActions);
            mUIActions = action;
            action->setPooled(true);

            // Release script args now or wait till it is brought
            // out of the pool?  Might as well do them now
            delete action->scriptArgs;
            action->scriptArgs = nullptr;
            // this is transient
            //action->setTargetTrack(nullptr);
        }
    }
}

void UIActionPool::dump()
{
    int count = 0;

    for (UIAction* a = mUIActions ; a != nullptr ; a = a->getNext())
      count++;

    printf("UIActionPool: %d allocated, %d in the pool, %d in use\n", 
           mAllocated, count, mAllocated - count);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

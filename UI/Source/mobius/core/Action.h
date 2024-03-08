/*
 * An internal core model for UIAction that includes additional
 * engine specific state.  One of these will be created when
 * receiving a UIAction from the outside, and can also be created
 * on the fly within the engine, especially in scripts.
 *
 * Actions can live for an indefinite period after they have requested
 * if they are scheduled and associated with events or scripts.
 *
 * They are allocated from a pool since they need to be created randomly
 * within the engine.
 *
 * See UIAction.h for more on the general structure of the action members.
 * The terminology changed during the Juce port so some of the legacy
 * access methods are maintained until we can evolve the old code that
 * uses actions.
 *
 * One important difference with UIAction is the notion of "results"
 *
 *   Results
 *
 *    When an action is being processed, several result properties may
 *    be set to let the caller how it was processed.  This is relevant
 *    only for the script interpreter.
 *
 * Old comments:
 * 
 * Actions may be created from scratch at runtime but it is more common
 * to create them once and "register" them so that they may be reused.
 * Using registered actions avoids the overhead of searching for the
 * system objects that define the target, Functions, Parameters, Bindables
 * etc.  Instead, when the action is registered, the target is resolved
 * and saved in the Action.
 *
 * Before you execute a registered action you must make a copy of it.
 * Actions submitted to Mobius are assumed to be autonomous objects
 * that will become owned by Mobius and deleted when the action is complete.
 *
 * New Notes:
 *
 * Mobius has the notion of interning or registering an action for reuse by
 * the old UI.  Retain this for awhile but I think we can get rid of it.
 * 
 */

#pragma once

#include "../../model/SystemConstant.h"
#include "../../model/UIAction.h"

// sigh, need this until we can figure out what to do with ExValue
#include "Expr.h"

//////////////////////////////////////////////////////////////////////
//
// TargetPointer
//
//////////////////////////////////////////////////////////////////////

/**
 * This corresponds to UIAction::ActionImplementation
 * It has the same meaning except that it points to the internal
 * model for Parameter and Function.
 * 
 * Direct references to what used to be called "Bindable"s and what
 * are now Structures (Preset, Setup, BindingSet) have been removed
 * and those are now referenced with an ordinal number.  Keeping a cached
 * pointer to those causes lots of complications since unlike Function and
 * Parameter the model for those can change which would invalidate
 * the cache we would keep here.  Since actions on structures is rare
 * and there aren't many of them, we just save the name and look
 * it up in the current model by name when necessary.
 *
 * There are still cache invalidation problems for Scripts which are
 * referenced here as dynamically allocated Functions.  Need to rethink
 * how that works, it's by far easiest just to require that scripts
 * can only be changed in a state of Global Reset.
 */
typedef union {

    void* object;
    class Function* function;
    class Parameter* parameter;
    int ordinal;
    
} TargetPointer;

/****************************************************************************
 *                                                                          *
 *                                   ACTION                                 *
 *                                                                          *
 ****************************************************************************/

/**
 * An object containing information about an action that is to 
 * take place within the engine.
 * 
 * These are created in response to trigger events then passed to Mobius
 * for processing.
 */
class Action {

    // actions are normally associated with a pool
    friend class ActionPool;

  public:

	Action();
    Action(class UIAction* src);

    // clone the action, who uses this?
	Action(Action* src);

    // must be a shorthand for a common dynamically created Function action
    Action(class Function* func);

    // destructor should normally be called only by the pool
	~Action();

    // application code normally frees it which returns it to the pool
    void free();

    // pool state
    Action* getNext();
    void setNext(Action* a);
    bool isRegistered();
    void setRegistered(bool b);

    // called by Mobius::doParameter to convert an argument string
    // from a binding model into the actionOperator and numeric argument
    // variables.  This is optional, the caller could have set those
    // directly if known
    void parseBindingArgs();

    // these are things to unpack MIDI message pieces that are held
    // in the "id" value
    // this shoudl NOT be necessary in core code, all we should care about
    // is that the id be unique for sustain and longPress detection
    // keep around temporarily but revisit this and if we really need them
    // UIAction can provide the utilities
    
    int getMidiStatus();
    void setMidiStatus(int i);

    int getMidiChannel();
    void setMidiChannel(int i);

    int getMidiKey();
    void setMidiKey(int i);

    // pretty sure this is only related to how bindings are processed
    // and does not need to be replicated down here
    bool isSpread();

    // for trace logging, and probably the old UI, get a readable
    // representation of what this action does
    void getDisplayName(char* buffer, int max);
    const char* getDisplayName();
    const char* getTypeDisplayName();
    void getGroupName(char* buf);
    void getFullName(char* buffer, int max);

    //////////////////////////////////////////////////////////////////////
    // Trigger
    //////////////////////////////////////////////////////////////////////

    // things that are direct copies from UIAction
    //
    // !! For script triggers, this will be the address of the ScriptInterpreter.
    // "This is only used for some special handling of the GlobalReset function."
    // this has the 64-bit pointer problem, need another way to do this
	long id;

	class Trigger* trigger;
    class TriggerMode* triggerMode;
    int triggerValue;
	int triggerOffset;
	bool down;
	bool longPress;

    // still used by Mobius but shouldn't be necessary
    bool repeat;

    // UIAction has this but it should have been filtered out long ago
    // bool repeat;

    // UIAction had this but it shouldn't be necessary in core
    //bool passOscArg;

    //////////////////////////////////////////////////////////////////////
    // ActionType (the artist formerly known as Target)
    //////////////////////////////////////////////////////////////////////
    
    ActionType* type;
    char actionName[MAX_TARGET_NAME];

    // These were inside a ResolvedTarget in old code

    // Corresponds to UIAction::actionImplementation
    TargetPointer implementation;

    //////////////////////////////////////////////////////////////////////
    // Scope
    //////////////////////////////////////////////////////////////////////

    // track number we still want to pass down, it will
    // be resolved to a Track poitner
    int scopeTrack;

    // groups should be handled in the UI, but keep it around
    // until we can remove the group tooling in the engine
    int scopeGroup;

    //////////////////////////////////////////////////////////////////////
    // Time
    //////////////////////////////////////////////////////////////////////

	/**
	 * True if quantization is to be disabled.
	 * Used only when rescheduling quantized functions whose
	 * quantization has been "escaped".
	 */
	bool escapeQuantization;

	/**
	 * True if input latency compensation is disabled.
	 * Used when invoking functions from scripts after we've
	 * entered "system time".
	 */
	bool noLatency;

    /**
     * True if the event should not be subject to synchronization
     * as it normally might.
     */
    bool noSynchronization;

    //////////////////////////////////////////////////////////////////////
    // Arguments
    //////////////////////////////////////////////////////////////////////

    char bindingArgs[MAX_ARG_LENGTH];
    class ActionOperator* actionOperator;
    ExValue arg;
    class ExValueList* scriptArgs;

    //////////////////////////////////////////////////////////////////////
    //
    // Runtime
    //
    // Various transient things maintained while the action is 
    // being processed.
    //
    //////////////////////////////////////////////////////////////////////

	/**
	 * True if we're rescheduling this after a previously scheduled
	 * function event has completed.
	 */
	class Event* rescheduling;

    /**
     * When reschedulign is true, this should have the event that 
     * we just finished that caused the rescheduling.
     */
    class Event* reschedulingReason;

    // can we get by without this?
    class Mobius* mobius;

    /**
     * Trasnsient flag set true if this action is being evaluated inside
     * the interrupt.    
     */ 
    bool inInterrupt;

    /**
     * Transient flag to disable focus lock and groups.  Used only
     * for some error handling in scripts.
     */
    bool noGroup;

    /**
     * Don't trace invocation of this function.  
     * A kludge for Speed shift parameters that convert themselves to 
     * many function invocations.   
     */
    bool noTrace;

    // temporary for debugging trigger timing
    long millisecond;
    double streamTime;

    //////////////////////////////////////////////////////////////////////
    // Legacy Accessors
    //////////////////////////////////////////////////////////////////////

    bool isResolved();
    bool isSustainable();
    
    class ActionType* getTarget() {return type;}
    void* getTargetObject() {return implementation.object;}
    class Function* getFunction();
    int getTargetTrack() {return scopeTrack;}
    int getTargetGroup() {return scopeGroup;}

    // probably used with "interning" still need this?
    bool isTargetEqual(Action* other);

    void setTarget(class ActionType* t);
    void setTarget(ActionType* t, void* object);
    void setFunction(class Function* f);
    void setParameter(class Parameter* p);
    void setTargetTrack(int track);
    void setTargetGroup(int group);

    // kludge for long press, make this cleaner
    void setLongFunction(class Function*f);
    Function* getLongFunction();

    // internal use only, not for the UI
    void setResolvedTrack(class Track* t);
    Track* getResolvedTrack();

    class ThreadEvent* getThreadEvent();
    void setThreadEvent(class ThreadEvent* te);

    class Event* getEvent();

    void setEvent(class Event* e);
    void changeEvent(class Event* e);
    void detachEvent(class Event* e);
    void detachEvent();

    // name is weird, this seems to be something left over from OSC
    // it is not the same as actionName
    const char* getName();
    void setName(const char* name);

    //////////////////////////////////////////////////////////////////////
    //
    // Protected
    //
    //////////////////////////////////////////////////////////////////////

  protected:

    void setPooled(bool b);
    bool isPooled();
    void setPool(class ActionPool* p);

    //////////////////////////////////////////////////////////////////////
    //
    // Private
    //
    //////////////////////////////////////////////////////////////////////

  private:

	void init();
    void reset();
    void clone(Action* src);
    char* advance(char* start, bool stopAtSpace);

    Action* mNext;
    bool mPooled;
    bool mRegistered;

    /**
     * The pool we came from.
     */
    class ActionPool* mPool;

	/**
	 * Set as a side effect of function scheduling to the event
	 * that represents the end of processing for this function.
	 * There may be play jump child events and other similar things
	 * that happen first.
	 */
	class Event* mEvent;

	/**
	 * Set as a side effect of function scheduling a Mobius
	 * thread event scheduled to process this function outside
	 * the interrupt handler.
	 * !! have to be careful with this one, it could in theory be
	 * processed before we have a chance to deal with it in the
	 * interpreter.
	 */
	class ThreadEvent* mThreadEvent;

    /**
     * Set during internal processing to the resolved Track
     * in which this action will run.  Overrides whatever is specified
     * in the target.
     */
    class Track* mResolvedTrack;

    /**
     * Allow the client to specify a name, convenient for
     * OSC debugging.
     */
    char* mName;

    /**
     * Alternate function to have the up transition after
     * a long press.  
     * !! get rid of this, we should either just replace Function
     * at invocation time, or have Function test the flags and figure it out.
     */
    Function* mLongFunction;

};

/****************************************************************************
 *                                                                          *
 *                                ACTION POOL                               *
 *                                                                          *
 ****************************************************************************/

class ActionPool {

  public:

    ActionPool();
    ~ActionPool();

    Action* newAction();
    Action* newAction(Action* src);
    void freeAction(Action* a);

    void dump();

  private:

    Action* allocAction(Action* src);

    Action* mActions;
    int mAllocated;


};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

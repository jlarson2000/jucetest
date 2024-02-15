/*
 * Model for sending commands or "actions" from the UI to the Mobius engine.
 *
 * This is a simplified version of the original Action to remove
 * engine dependencies and move to a cleaner model for "resolution"
 * of the target and the lifecycle of the action objects.
 *
 * The old model had the notion of a ResolvedTarget which was an interned
 * object managed by Mobius.cpp that had cached pointers directly to the various
 * objects that were targeted.  This included internal structure like Track
 * and configuration objects like Preset and Setup.  This resulted in awkward
 * cache management since config objects can be deleted as they are edited
 * and some of the targets like UIParameter didn't belong down there anyway.
 *
 * Since Bindings are no longer managed below the MobiusInterface we can move a
 * lot of that complexity up here.  The exception being targets that resolve to
 * Tracks.  Currently we will still reference them by number since it's a simple
 * array dereference to get to them.  Potentially slower are references to
 * config objects like Preset which will now do a linear search rather than worrying
 * about invalidating cached pointers.  Can revisit this if necessary.
 *
 * UIActions are now owned and operated by the UI layer, Mobius no longer takes
 * ownership of them when they are passed to doAction.  We'll still maintain the old
 * Action model internally for awhile, conversion will be made at the interface.
 * 
 * todo: need work on script targets, currently wants to test
 * FunctionDef->event->RunScriptEvent
 * Scripts are going to be sort of like dynamic FunctionDefs
 * that get added at runtime.  Maybe should have a new Target type for
 * that but that would complicated the engine.
 * hmm, why can't we just make the single RunScriptFunction subclass
 * and add it as RunScriptFunction to the Functions array?
 * 
 * UIActions are usually built from a Binding definition, but now UIButton
 * serves as a binding source, which is more like it was a long time ago,
 * rethink this, maybe it was better to make everything be a Binding
 *
 * ---
 * Old Comments
 * 
 * Once the Mobius engine is initialized, it is controlled primarily
 * by the posting of Actions.  An Action object is created and given
 * to Mobius with the doAction command.  The Action is carried out 
 * synchronously if possible, otherwise it is placed in an action
 * queue and processed at the beginning of the next audio interrupt.
 *
 * The Action model contains the following things, described here using
 * the classic W's model.
 *
 *   Trigger (Who)
 *
 *    Information about the trigger that is causing this action to
 *    be performed including the trigger type (midi, key, osc, script),
 *    trigger values (midi note number, velocity), and trigger
 *    behavior (sustainable, up, down).
 *
 *   Target (What)
 *
 *    Defines what is to be done.  Execute a function, change a control,
 *    set a parameter, select a configuration object.
 *
 *   Scope (Where)
 *
 *    Where the target is to be modified: global, track, or group.
 *
 *   Time (When)
 *
 *    When the target is to be modified: immediate, after latency delay,
 *    at a scheduled time, etc.
 *
 *   Arguments (How)
 *
 *    Additional information that may effect the processing of the action.
 *    Arguments may be specified in the binding or may be passed from
 *    scripts.
 *
 *   Results
 *
 *    When an action is being processed, several result properties may
 *    be set to let the caller how it was processed.  This is relevant
 *    only for the script interpreter.
 *
 * Actions may be created from scratch at runtime but it is more common
 * to create them once and "register" them so that they may be reused.
 * Using registered actions avoids the overhead of searching for the
 * system objects that define the target, Functions, Parameters, Bindables
 * etc.  Instead, when the action is registered, the target is resolved
 * and saved in the Action.
 *
 * NOTE: This is where we may need to swap out a UIAction subclass instance
 * or use the Pimpl pattern.
 *
 * Before you execute a registered action you must make a copy of it.
 * Actions submitted to Mobius are assumed to be autonomous objects
 * that will become owned by Mobius and deleted when the action is complete.
 * 
 * NOTE: I might want to change this, have the engine do the copying, especially
 * if it needs to map it anyway?  I guess that makes it harder to use for
 * the Results since the caller won't know where to look.
 *
 */

#pragma once

#include "SystemConstant.h"
#include "Binding.h"

// sigh, need this until we can figure out what to do with ExValue
#include "ExValue.h"

/****************************************************************************
 *                                                                          *
 *                                  OPERATOR                                *
 *                                                                          *
 ****************************************************************************/

/**
 * Constants that describe operations that produce a relative change to
 * a control or parameter binding.
 */
class ActionOperator : public SystemConstant {
  public:
    static ActionOperator* get(const char* name);

    ActionOperator(const char* name, const char* display);

    static std::vector<ActionOperator*> Operators;
	static ActionOperator* getOperator(const char* name);

    int ordinal;
    
};

extern ActionOperator* OperatorMin;
extern ActionOperator* OperatorMax;
extern ActionOperator* OperatorCenter;
extern ActionOperator* OperatorUp;
extern ActionOperator* OperatorDown;
extern ActionOperator* OperatorSet;
extern ActionOperator* OperatorPermanent;

//////////////////////////////////////////////////////////////////////
//
// TargetPointer
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

/**
 * Maximum length of a string argument in an Action.
 * There are four of these so make them relatively short.
 *
 * UPDATE: In theory Function aruments could be arbitrary.  The only one
 * that comes to mind is RunScript which will have the name of the
 * Script to run.
 */
#define MAX_ARG_LENGTH 128

/**
 * Maximum length of an internal buffer used to format a readable
 * description of the action for debugging.
 */
#define MAX_DESCRIPTION 1024

/**
 * An object containing information about an action that is to 
 * take place within the engine.
 * 
 * These are created in response to trigger events then passed to Mobius
 * for processing.
 */
class UIAction {

  public:

	UIAction();
	~UIAction();

    //////////////////////////////////////////////////////////////////////
    //
    // Trigger (Who)
    //
    //////////////////////////////////////////////////////////////////////

	/**
	 * A unique identifier for the action.
	 * This is used when matching the down and up transitions of
	 * sustainable triggers with script threads.
	 * The combination of the Trigger and this id must be unique.
	 * It is also exposed as a variable for scripts so we need to
     * lock down the meaning.  
     *
     * For MIDI triggers it will be the first byte containing both
     * the status and channel plus the second byte containing the note
     * number.  The format is:
     *
     *      ((status | channel) << 8) | key
     *
     * For Key triggers it will be the key code.
     *
     * For Host triggers it will be the host parameter id, but this
     * is not currently used since we handle parameter bindings at a higher
     * level in MobiusPlugin.
     *
     * For script triggers, this will be the address of the ScriptInterpreter.
     * This is only used for some special handling of the GlobalReset function.
     * !! This is one of the 64-bit modifications.  The value must be large enough
     * to accomodate a pointer and long isn't enough.  Should be an enum with void*
     * and integer values.
	 */
	long id;

	/**
	 * The trigger that was detected.
	 */
	Trigger* trigger;

    /**
     * The behavior of this trigger if ambiguous.
     */
    TriggerMode* triggerMode;

    /**
     * True if we will be passing the OSC message argument
     * along as a function argument or using it as the parameter value.
     * This effects the triggerMode.
     */
    bool passOscArg;

    // From here down are dynamic values that change for every
    // invocation of the action.

    /**
     * A secondary value for the the trigger.
     * This is only used for TriggerMidi and contains the key velocity
     * for notes and the controller value for CCs.  It is used only
     * by the LoopSwitch function to set output levels.
     */
    int triggerValue;

	/**
	 * For ranged triggers, this is the relative location within the range.
     * Negative if less than center, positive if greater than center.
	 */
	int triggerOffset;

	/**
	 * True if the trigger was is logically down.  If the trigger
     * is not sustainable this should always be true.
	 */
	bool down;

    /**
     * true if the trigger is in "auto repeat" mode.
     * This is relevant only for TriggerKey;
     */
    bool repeat;

	/**
	 * True if this is the up transition after a long press.
	 * Also true for invocations that are done at the moment
	 * the long-press time elapses.  
	 */
	bool longPress;

    //////////////////////////////////////////////////////////////////////
    //
    // Target (What)
    // Scope (Where)
    //
    // These are more complicated and must be accessed with methods.
    // If the Action was created from a Binding we'll have a ResolvedTarget.
    // IF the Action is created dynamically we'll have a private set
    // of properties that define the target.  We don't have interfaces
    // for the dynamic construction all possible targets only Functions.  
    //
    //////////////////////////////////////////////////////////////////////

    bool isResolved();
    bool isSustainable();
    Target* getTarget();
    void* getTargetObject();
    class FunctionDefinition* getFunction();
    int getTargetTrack();
    int getTargetGroup();

    bool isTargetEqual(UIAction* other);

    void setTarget(Target* t);
    void setTarget(Target* t, void* object);
    void setFunction(class FunctionDefinition* f);
    void setParameter(class Parameter* p);
    void setTargetTrack(int track);
    void setTargetGroup(int group);

    // kludge for long press, make this cleaner
    void setLongFunction(class FunctionDefinition*f);
    FunctionDefinition* getLongFunction();

    // internal use only, not for the UI
    //void setResolvedTrack(class Track* t);
    //Track* getResolvedTrack();

    ResolvedTarget* getResolvedTarget();

    //class ThreadEvent* getThreadEvent();
    //void setThreadEvent(class ThreadEvent* te);

    //class Event* getEvent();

    //void setEvent(class Event* e);
    //void changeEvent(class Event* e);
    //void detachEvent(class Event* e);
    //void detachEvent();

    // something to do with OSC debugging
    const char* getName();
    void setName(const char* name);

    //////////////////////////////////////////////////////////////////////
    //
    // Time (When)
    //
    // TODO: Might be interesting for OSC to schedule things in the future.  
    // 
    // Might be a good place to control latency adjustments.
    //
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
     * todo: should this be internal?
	 */
	bool noLatency;

    /**
     * True if the event should not be subject to synchronization
     * as it normally might.
     */
    bool noSynchronization;

    //////////////////////////////////////////////////////////////////////
    //
    // Arguments (How)
    //
    // TODO: Figure out a way to install new configration objects
    // through this interface so we don't have to have
    // Mobius methods setConfiguration, etc.  
    //
    //////////////////////////////////////////////////////////////////////

    /**
     * Optional binding arguments.
     * These are taken from Binding.arguments and are processed differently for
     * each target.  For numeric parameters they may be:
     *
     *    center 
     *      calculate the center value
     *    up X
     *      raise the value by X
     *    down X
     *      lower the value by X
     *    set X
     *      set the value to X
     *
     * Other parameter types and functions support different arguments.
     * If the args include both an operator and a numeric operand, the
     * operand is left in the "arg".
     */
    char bindingArgs[MAX_ARG_LENGTH];

    /**
     * Operator to apply to the current value of a parameter or control.
     * Normally this is parsed from bindingArgs.
     * sigh, "operator" is a reserved word
     */
    ActionOperator* actionOperator;

    /**
     * The primary argument of the action.
     * This must be set for Parameter targets.  For function targets
     * there are a few that require an argument: SelectLoop, SelectTrack,
     * InstantMultiply etc.
     *
     * For Host bindings, this will be the scaled value of the
     * host parameter value.
     *
     * For Actions created by the UI it will be an integer.  For
     * controls knobs it will be 0-127, for replicated functions
     * like LoopN and TrackN it will be the object index.
     * 
     * For Key bindigns, this will start 0 but may be adjusted
     * by bindingArguments.
     *
     * For MIDI note bindings, this will start 0 but may be adjusted
     * for bindingArguments.
     *
     * For MIDI CC bindings, this will be scaled CC value.
     * 
     * For OSC bindings, this will be the scaled first message argument.
     *
     * In all cases bindingArgs may contain commands to recalculate
     * the value relative to the current value.
     */
    ExValue arg;

    /** 
	 * Optional arguments, only valid in scripts.
     * This is used for a small number of script functions that
     * take more than one argument.  The list is of variable length,
     * dynamically allocated, and must be freed.
     *
     * NOTE: may be able to get rid of this, or move it to the
     * internal Action managed by the engine
	 */
    class ExValueList* scriptArgs;

    //////////////////////////////////////////////////////////////////////
    //
    // Runtime
    //
    // Various transient things maintained while the action is 
    // being processed.
    //
    // I think this is only relevant for the engine's internal Action
    //
    //////////////////////////////////////////////////////////////////////

	/**
	 * True if we're rescheduling this after a previously scheduled
	 * function event has completed.
	 */
	//class Event* rescheduling;

    /**
     * When reschedulign is true, this should have the event that 
     * we just finished that caused the rescheduling.
     */
    //class Event* reschedulingReason;

    // can we get by without this?
    //class Mobius* mobius;

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
    //
    // Functions
    //
    //////////////////////////////////////////////////////////////////////

    // new method for debugging action handling
    const char* getDescription();

    int getOverlay();
    void setOverlay(int i);

    void parseBindingArgs();

    int getMidiStatus();
    void setMidiStatus(int i);

    int getMidiChannel();
    void setMidiChannel(int i);

    int getMidiKey();
    void setMidiKey(int i);

    bool isSpread();
    void getDisplayName(char* buffer, int max);

    //////////////////////////////////////////////////////////////////////
    //
    // Protected
    //
    //////////////////////////////////////////////////////////////////////

  protected:

    // can we remove pooling above the engine?
    void setPooled(bool b);
    bool isPooled();
    void setPool(class UIActionPool* p);
    
    //////////////////////////////////////////////////////////////////////
    //
    // Private
    //
    //////////////////////////////////////////////////////////////////////

  private:

	void init();
    void reset();
    void clone(UIAction* src);
    // helper for arg parsing
    char* advance(char* start, bool stopAtSpace);

    UIAction* mNext;
    bool mPooled;
    bool mRegistered;

    /**
     * The pool we came from.
     */
    class UIActionPool* mPool;

	/**
	 * Set as a side effect of function scheduling to the event
	 * that represents the end of processing for this function.
	 * There may be play jump child events and other similar things
	 * that happen first.
	 */
	//class Event* mEvent;
    
	/**
	 * Set as a side effect of function scheduling a Mobius
	 * thread event scheduled to process this function outside
	 * the interrupt handler.
	 * !! have to be careful with this one, it could in theory be
	 * processed before we have a chance to deal with it in the
	 * interpreter.
	 */
	//class ThreadEvent* mThreadEvent;

    /**
     * Reference to an interned target when the action is created from
     * a Binding.
     */
    ResolvedTarget* mInternedTarget;
    
    /**
     * Private target properties for actions that are not associated 
     * with bindings.  These are typically created on the fly by the UI.
     */
    ResolvedTarget mPrivateTarget;

    /**
     * Set during internal processing to the resolved Track
     * in which this action will run.  Overrides whatever is specified
     * in the target.
     */
    //class Track* mResolvedTrack;

    /**
     * Internal field set by BindingResolver to indicate which
     * BindingConfig overlay this action came from.
     */
    int mOverlay;

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
    FunctionDefinition* mLongFunction;

    char mDescription[MAX_DESCRIPTION];

};

/****************************************************************************
 *                                                                          *
 *                                ACTION POOL                               *
 *                                                                          *
 ****************************************************************************/

class UIActionPool {

  public:

    UIActionPool();
    ~UIActionPool();

    UIAction* newUIAction();
    UIAction* newUIAction(UIAction* src);
    void freeUIAction(UIAction* a);

    void dump();

  private:

    UIAction* allocUIAction(UIAction* src);

    UIAction* mUIActions;
    int mAllocated;


};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

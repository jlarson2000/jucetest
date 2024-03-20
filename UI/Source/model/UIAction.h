/**
 * A model for actions that can be sent from the UI into the engine.
 * This was derived from the old Action model used by the engine and heavily
 * simplified.
 *
 * It uses the same Trigger and ActionType constant objects as Binding
 * and is usually built from a Binding but can exist independently.
 *
 */

#pragma once

#include "SystemConstant.h"
#include "Trigger.h"
#include "ActionType.h"
#include "Binding.h"

// sigh, need this until we can figure out what to do with ExValue
#include "ExValue.h"

//////////////////////////////////////////////////////////////////////
//
// Implementation
//
//////////////////////////////////////////////////////////////////////

/**
 * Union of possible action implementation pointers.
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
 * Formerly had Bindable (now Structure) in here but I didn't like ugly cache
 * invalidation when the configuration objects are replaced.
 * Until it seems necessary, just remember the name and look them
 * up at runtime.
 *
 * Ordinal may be set when available for faster lookup of Structures
 * and Scripts.
 */
typedef union {

    void* object;
    class FunctionDefinition* function;
    class UIParameter* parameter;
    int ordinal;
    //class Bindable* bindable;

} ActionImplementation;

//////////////////////////////////////////////////////////////////////
//
// UIAction
//
//////////////////////////////////////////////////////////////////////

/**
 * Maximum length of a target name
 *
 * For most actions, this is relevant only until the target reference
 * is resolved to a pointer to a system constant object.
 *
 * Configurable objects like Presets and Scripts the name identifies
 * the non-constant object and an "ordinal" may be calculated for faster
 * lookup.
 */
#define MAX_TARGET_NAME 128

/**
 * Maximum length of a string argument in an Action.
 * There are four of these so make them relatively short.
 *
 * UPDATE: In theory Function aruments could be arbitrary.  The only one
 * that comes to mind is RunScript which will have the name of the
 * Script to run.
 *
 * todo: think about the difference between targetName and targetArg for
 * configurable objects.  I think in all cases now we have a Function
 * to represent the object (SelectPreset, RunScript) and the argument
 * can be the object name.  In those cases targetName would be "SelectPreset".
 * Currently targetName is the object name.
 */
#define MAX_ARG_LENGTH 128

/**
 * A random string we used to call "name".
 * I think this is only used in OSC bindings, might be the path.
 * Figure out what this is and give it a better name.
 */
#define MAX_EXTENSION 1024

/**
 * Maximum length of an internal buffer used to format a readable
 * description of the action for debugging.
 */
#define MAX_DESCRIPTION 1024

class UIAction {

  public:

	UIAction();
    UIAction(UIAction* src);
	~UIAction();

    void init(class Binding* b);
    void reset();
    void resolve();

    // in a few (one?) places actions need to be queued
    // in particular MobiusKernel needs to do this but juce::Array
    // and std::vector are problematic because they can dynamically grow
    // without annoying preparation and checking their capacity before use
    // since we're not supposed to be allocating memory in the audio thread
    // use a good old fashioned linked list
    // note that unlike other older objects with a chain pointer we do not
    // cascade delete these
    UIAction* next;
    
    //////////////////////////////////////////////////////////////////////
    // Trigger
    //////////////////////////////////////////////////////////////////////

	/**
	 * The type of trigger that was detected.
	 */
	Trigger* trigger;

    /**
     * The behavior of this trigger if ambiguous.
     */
    TriggerMode* triggerMode;

	/**
	 * A unique identifier for the trigger.
     * Used for the detection of down/up transitions and long presses.
     * See design/trigger-ids
	 */
	long triggerId;

    /**
     * A secondary value for the the trigger.
     * This is only used for TriggerMidi and contains the key velocity
     * for notes and the controller value for CCs.  It is used only
     * by the LoopSwitch function to set output levels.
     *
     * todo: the meaning of id and triggerValue is not obvious
     * change this to triggerNumber and triggerExtra or something
     * might end up with duplications of triggerNumber and id so we
     * can keep the meaning of id cleaner
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
     *
     * todo: HATE this, since up transitions are the special case
     * this should be reversed to triggerUp
	 */
	bool down;

    /**
     * true if the trigger is in "auto repeat" mode.
     * This is relevant only for TriggerKey
     * !! there is no need for this complexity, and it will never be set
     * now that the key tracker suppresses auto-repeat.  If it becomes interesting
     * handle it entirely in the UI with multiple actions, it does not need to
     * be sent down.
     */
    //bool repeat;

	/**
	 * True if this is the up transition after a long press.
	 * Also true for invocations that are done at the moment
	 * the long-press time elapses.  
	 */
	bool longPress;

    /**
     * True if we will be passing the OSC message argument
     * along as a function argument or using it as the parameter value.
     * This effects the triggerMode.
     * todo: obscure, don't like it, nosir
     */
    bool passOscArg;

    /**
     * Special case for actions generated by a script.
     *
     * Behavior of an action from within a script may be different
     * depending on which script the action came from.  In particular
     * the GlobalReset function normally cancels all running scripts,
     * but if GlobalReset is done from within a script, you don't want
     * to cancel the script that is doing it.
     *
     * Old code used the triggerId for this but it broke with
     * 64-bit pointers and was obscure anyway.
     */
    void* triggerOwner;

    //////////////////////////////////////////////////////////////////////
    // Action
    //////////////////////////////////////////////////////////////////////

    /**
     * The type of action to perform (Function, Parameter, or Activate)
     */
    ActionType* type;

    /**
     * The name of the internal object that defines what the action does.
     */
    char actionName[MAX_TARGET_NAME];

    /**
     * A resolved pointer to a system constant object like Function
     * or Parameter that implements the action.  For Structures
     * that can be activated, it will be ordinal of the object.
     * 
     */
    ActionImplementation implementation;
    
    /**
     * Alternate function to have the up transition after a long press.  
     * !! get rid of this, we should either just replace Function
     * at invocation time, or have Function test the flags and figure it out.
     * todo: probably relevant only for the engine
     */
    class FunctionDefinition* longFunction;

    //////////////////////////////////////////////////////////////////////
    // Scope
    //////////////////////////////////////////////////////////////////////

    /**
     * Non-zero if the action is targeted to a specific track rather than
     * globally, or for the selected track.
     * todo: dislike not being consistent about track numbers, they
     * start from zero in some places with -1 being active and 1 in
     * some places with 0 being active. Here they are 1 based.
     */
    int scopeTrack;

    /**
     * Non-zero if the action is targeted to a track group.
     */
    int scopeGroup;

    //////////////////////////////////////////////////////////////////////
    // Extensions
    //////////////////////////////////////////////////////////////////////

    // this has something todo with OSC debugging, is it the path?
    char extension[MAX_EXTENSION];

    /**
     * Internal field set by BindingResolver to indicate which
     * BindingSet overlay this action came from.
     * todo: do we need this any more?
     */
    int bindingOverlay;

    //////////////////////////////////////////////////////////////////////
    //
    // Time
    //
    // TODO: Might be interesting for OSC to schedule things in the future.  
    // Might be a good place to control latency adjustments.
    // todo: I think most if not all of these are only used by the
    // engine in it's internal Action model
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
    // Arguments
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

    // todo: hate using ExValue for the primary value since
    // they can now almost always be integers or ordinals

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
    // Utilities
    //////////////////////////////////////////////////////////////////////

    // try to hide ExValue
    
    int getValueInt() {
        return arg.getInt();
    }

    void setValue(int i) {
        arg.setInt(i);
    }

    bool isSustainable();
    bool isTargetEqual(UIAction* other);

    // these are necessary for the binding table builder
    // and some old code related to Pitch and Time stretch

    int getMidiStatus();
    void setMidiStatus(int i);

    int getMidiChannel();
    void setMidiChannel(int i);

    int getMidiKey();
    void setMidiKey(int i);

    bool isSpread();

    // Residual nice naming calculators, weed this 
    const char* getDescription();
    void getDisplayName(char* buffer, int max);

    //////////////////////////////////////////////////////////////////////
    // Results
    //
    // These can be set synchronously by the engine after calling doAction
    //
    //////////////////////////////////////////////////////////////////////

    // zero if the action was performed immediately
    // non-zero if the action was queued;
    int actionId;

    //////////////////////////////////////////////////////////////////////
    //
    // Private
    //
    //////////////////////////////////////////////////////////////////////

  private:

	void init();
    
    void parseBindingArgs();
    char* advance(char* start, bool stopAtSpace);

    char mDescription[MAX_DESCRIPTION];
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

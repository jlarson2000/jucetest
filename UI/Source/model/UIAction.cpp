/*
 * A model for representing actions to be taken within the Mobius
 * engine.  These are created in response to triggers.
 */

#include "../util/Util.h"
#include "../util/Trace.h"
#include "../util/List.h"

#include "ExValue.h"

#include "Trigger.h"
#include "ActionType.h"
#include "FunctionDefinition.h"
#include "UIParameter.h"
#include "Binding.h"

#include "UIAction.h"

//////////////////////////////////////////////////////////////////////
//
// Action
//
// Most of the properties are directly accessible so we can dispense
// with getter/seter functions
//
//////////////////////////////////////////////////////////////////////

UIAction::UIAction()
{
    init();
}

UIAction::~UIAction()
{
    // old model has some dynamic string here,
    // using char buffers for now but could convert to juce::String
    delete scriptArgs;
}

/**
 * Needed when actions pass from the UI to the kernel tiers.
 * todo: This is simple enough we really need to be looking
 * at starting to use copy constructors and letting the compiler
 * handle this.
 */
UIAction::UIAction(UIAction* src)
{
    // this never conveys
    next = nullptr;
    
    // Trigger
    id = src->id;
    trigger = src->trigger;
    triggerMode = src->triggerMode;
    passOscArg = src->passOscArg;
    triggerValue = src->triggerValue;
    triggerOffset = src->triggerOffset;
    down = src->down;
    //repeat = src->repeat;
    longPress = src->longPress;

    // Action
    type = src->type;
    strcpy(actionName, src->actionName);
    implementation.object = src->implementation.object;
    longFunction = src->longFunction;

    // Scope
    scopeTrack = src->scopeTrack;
    scopeGroup = src->scopeGroup;

    // Extension
    strcpy(extension, src->extension);
    bindingOverlay = src->bindingOverlay;
    
    // Time
    escapeQuantization = src->escapeQuantization;
    noLatency = src->noLatency;
    noSynchronization = src->noSynchronization;
    
    // Arguments
    strcpy(bindingArgs, src->bindingArgs);
    actionOperator = src->actionOperator;
    arg.set(&(src->arg));

    // todo: I don't think this is used in the UI
    // if so hide it under functions
    // it's an ExValueList and I don't want to mess with copying it yet
    scriptArgs = nullptr;
}


void UIAction::init()
{
    next = nullptr;
    // Trigger
    id = 0;
    trigger = nullptr;
    triggerMode = nullptr;
    passOscArg = false;
    triggerValue = 0;
    triggerOffset = 0;
    down = false;
    //repeat = false;
    longPress = false;

    // Operation
    type = nullptr;
    strcpy(actionName, "");
    implementation.object = nullptr;
    longFunction = nullptr;

    // Scope
    scopeTrack = 0;
    scopeGroup = 0;

    // Extension
    strcpy(extension, "");
    bindingOverlay = 0;
    
    // Time
    escapeQuantization = false;
    noLatency = false;
    noSynchronization = false;
    
    // Arguments
    strcpy(bindingArgs, "");
    actionOperator = nullptr;
    arg.setNull();

    // todo: I don't think this is used in the UI
    // if so hide it under functions
    scriptArgs = nullptr;
}

void UIAction::init(Binding* b)
{
    init();

    trigger = b->trigger;
    triggerMode = b->triggerMode;
    setMidiStatus(b->triggerValue);
    setMidiChannel(b->midiChannel);

    type = b->action;
    CopyString(b->getActionName(), actionName, sizeof(actionName));
    CopyString(b->getArguments(), bindingArgs, sizeof(bindingArgs));

    // initially at least, all binding argument strings will be numbers
    // and code to handle actions expects that in the ExValue arg
    // need to be smarter about this and possibly make Operator do the parsing
    if (strlen(bindingArgs) > 0)
      arg.setInt(ToInt(bindingArgs));

    // more as we bring on Binders
}

/**
 * Reset clears a previously initialized action.
 * The difference here is that we have to release the
 * scriptArgs.
 *
 * todo: where would this be used?
 * probably when we pooled them or copied them
 */
void UIAction::reset()
{
    delete scriptArgs;
    init();
}

/**
 * Resolve the operation implementation pointer if we haven't already.
 */
void UIAction::resolve()
{
    if (implementation.object == nullptr) {
        if (type == ActionFunction) {
            implementation.function = FunctionDefinition::find(actionName);
        }
    }

    if (implementation.object == nullptr) {
        trace("Unresolved action: %s\n", actionName);
    }
}

//////////////////////////////////////////////////////////////////////
//
// Descriptions
//
// This is a mess, figure out who needs these and simplify
//
//////////////////////////////////////////////////////////////////////

/**
 * New method to format a concise description of the action.
 * Used during initial testing.  String must not be freed.
 * Use an internal buffer for now as this evolves.
 */
const char* UIAction::getDescription()
{
    // need more...
    getDisplayName(mDescription, sizeof(mDescription));
    return (const char*)&mDescription;
}
 
/**
 * Calculate a display name for this action.
 * Used in the KeyHelp dialog, possibly others.
 *
 * todo: this is old but looks useful
 */
void UIAction::getDisplayName(char* buffer, int max)
{
    // TODO: add a trigger prefix!
    buffer[0] = 0;

    AppendString(actionName, buffer, max);

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

//////////////////////////////////////////////////////////////////////
//
// Public Utilities
//
//////////////////////////////////////////////////////////////////////

/**
 * Return true if the TriggerMode supports sustainability
 */
bool UIAction::isSustainable()
{
    return (triggerMode == TriggerModeMomentary ||
            triggerMode == TriggerModeToggle);
}

/**
 * Return true if our operation is the same as another.
 * The action must be resolved by now.
 * Used by BindingResolver to filter redundant bindings.
 *
 * !! This does more than just the target, it's a combo
 * of operation and scope
 */
bool UIAction::isTargetEqual(UIAction* other)
{
    return (type == other->type &&
            implementation.object == other->implementation.object &&
            scopeTrack == other->scopeTrack &&
            scopeGroup == other->scopeGroup);
}

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
    if (type == ActionFunction) {
        FunctionDefinition* f = implementation.function;
        if (f != nullptr)
          spread = f->isSpread();
    }
	return spread;
}

//////////////////////////////////////////////////////////////////////
//
// Private Utilities
//
//////////////////////////////////////////////////////////////////////

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

        actionOperator = ActionOperator::find(psn);
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


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

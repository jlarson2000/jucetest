/*
 * A model for representing actions to be taken within the Mobius
 * engine.  These are created in response to triggers.
 */

#include "../util/Util.h"
#include "../util/Trace.h"
#include "../util/List.h"

#include "ExValue.h"

#include "Binding.h"
#include "FunctionDefinition.h"
#include "Parameter.h"

#include "UIAction.h"

//////////////////////////////////////////////////////////////////////
//
// ActionOperator
//
//////////////////////////////////////////////////////////////////////

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

// todo: this sounds dangerous, what did it do?
ActionOperator OperatorPermanentObj{"permanent", "Permanent"};
ActionOperator* OperatorPermanent = &OperatorPermanentObj;

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

    // Target
    target = nullptr;
    strcpy(targetName, "");
    targetPointer.object = nullptr;
    targetOrdinal = 0;
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

    trigger = b->getTrigger();
    triggerMode = b->getTriggerMode();
    setMidiStatus(b->getValue());
    setMidiChannel(b->getChannel());

    target = b->getTarget();
    CopyString(b->getName(), targetName, sizeof(targetName));
    CopyString(b->getArgs(), bindingArgs, sizeof(bindingArgs));

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

    AppendString(targetName, buffer, max);

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
 * Return true if our target is the same as another.
 * The action must be resolved by now.
 * Used by BindingResolver to filter redundant bindings.
 */
bool UIAction::isTargetEqual(UIAction* other)
{
    return (target == other->target &&
            targetPointer.object == other->targetPointer.object &&
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
    if (target == TargetFunction) {
        FunctionDefinition* f = targetPointer.function;
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


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

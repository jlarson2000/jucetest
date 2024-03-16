/**
 * Code related to the processing of UIActions sent to Mobius.
 * This is mostly old code factored out of Mobius to reduce the size.
 *
 * Mapping between the new UIAction model and old Action is done here.
 *
 * This is basically the same as an old class ActionDispatcher, but that
 * appeared to have never been used, so I'm starting it fresh with a code
 * base that I know worked.
 *
 */

// for juce::Array
#include <JuceHeader.h>

#include "../../util/Trace.h"
#include "../../model/UIAction.h"
#include "../../model/UIParameter.h"
#include "../../model/FunctionDefinition.h"

#include "Mapper.h"

#include "Action.h"
#include "Export.h"
#include "Function.h"
#include "Parameter.h"
#include "TriggerState.h"
#include "Track.h"
#include "Loop.h"
#include "Script.h"
#include "Mobius.h"

#include "Actionator.h"

Actionator::Actionator(Mobius* m)
{
    mMobius = m;
    mActionPool = new ActionPool();
    mTriggerState = new TriggerState();
    mFunctionMap = nullptr;
    mParameterMap = nullptr;

    // safe to do this in the constructor?
    // we need to do it during structure initialization because
    // it isn't supposed to grow in the audio thread
    initFunctionMap();
    initParameterMap();
}

Actionator::~Actionator()
{
    delete mTriggerState;

    mActionPool->dump();
    delete mActionPool;
}

void Actionator::dump()
{
    mActionPool->dump();
}

/**
 * Initialize the table for mapping UI tier FunctionDefinition
 * objects to their core Function counterparts.
 * This can only be done during initialization in the UI thread.
 *
 * Handling only static functions right now, will need to do something
 * extra for scripts.
 *
 * Jesus vector is a fucking fight, use juce::Array, it's obvious.
 */
void Actionator::initFunctionMap()
{
    mFunctionMap.clear();
    for (int i = 0 ; i < FunctionDefinition::Instances.size() ; i++) {
        FunctionDefinition* f = FunctionDefinition::Instances[i];
        Function* coreFunction = Function::getStaticFunction(f->getName());
        if (coreFunction != nullptr)
          mFunctionMap.add(coreFunction);
        else {
            mFunctionMap.add(nullptr);
            Trace(1, "Actionator::initFunctionMap Function %s not found\n", f->getName());
        }
    }
}

/**
 * Initialize the table for mapping UIParameters to core Parameter
 */
void Actionator::initParameterMap()
{
    mParameterMap.clear();
    for (int i = 0 ; i < UIParameter::Instances.size() ; i++) {
        UIParameter* p = UIParameter::Instances[i];

        // in a few cases I wanted to use a different name
        const char* pname = p->coreName;
        if (pname == nullptr)
          pname = p->getName();
        
        Parameter* coreParameter = Parameter::getParameter(pname);
        if (coreParameter != nullptr) {
            mParameterMap.add(coreParameter);
        }
        else {
            mParameterMap.add(nullptr);
            Trace(1, "Actionator::initParameterMap Parameter %s not found\n", pname);
        }
    }
}

/**
 * Map a UI to Core parameter, and trace a warning if we can't find one.
 */
Parameter* Actionator::mapParameter(UIParameter* uip)
{
    Parameter* cp = nullptr;

    if (uip != nullptr) {
        cp = mParameterMap[uip->ordinal];
        if (cp == nullptr) {
            Trace(1, "Actionator: Unable to map Parameter %s\n",
                  uip->getName());
        }
    }
    return cp;
}

/**
 * Called by Mobius at the beginning of each audio interrupt.
 */
void Actionator::advanceTriggerState(int frames)
{
    mTriggerState->advance(this, frames);
}

/**
 * THINK: This was brought down from Mobius with the rest of the
 * action related code because it needed it, but so do lots of other
 * things in Mobius.  Where should this live?
 *
 * Determine the destination Track for an Action.
 * Return NULL if the action does not specify a destination track.
 * This can be called by a few function handlers that declare
 * themselves global but may want to target the current track.
 */
Track* Actionator::resolveTrack(Action* action)
{
    Track* track = NULL;

    if (action != NULL) {

        // This trumps all, it should only be set after the
        // action has been partially processed and replicated
        // for focus lock or groups.
        track = action->getResolvedTrack();
        
        if (track == NULL) {

            // note that the track number in an action is 1 based
            // zero means "current"
            int tnum = action->getTargetTrack();
            if (tnum > 0) {
                track = mMobius->getTrack(tnum - 1);
                if (track == NULL) {
                    Trace(1, "Track index out of range");
                    // could either return NULL or force it to the lowest
                    // or higest
                    track = mMobius->getTrack();
                }
            }

            // Force a track change if this function says it must run in the 
            // active track.  This will usually be the same, but when calling
            // some of the track management functions from scripts, they may
            // be different.
            Function* f = action->getFunction();
            if (f != NULL && f->activeTrack) {
                Track* active = mMobius->getTrack();
                if (track != active) {
                    if (track != NULL)
                      Trace(mMobius, 2, "Mobius: Adjusting target track for activeTrack function %s\n", f->getName());
                    track = active;
                }
            }
        }
    }

    return track;
}

//////////////////////////////////////////////////////////////////////
//
// Action Lifecycle
//
//////////////////////////////////////////////////////////////////////

/**
 * Allocate an action. 
 * The caller is expected to fill this out and execute it with doAction.
 * If the caller doesn't want it they must call freeAction.
 * These are maintained in a pool that both the application threads
 * and the interrupt threads can access so have to use a Csect.
 */
Action* Actionator::newAction()
{
    Action* action = NULL;

    //mCsect->enter("newAction");
    action = mActionPool->newAction();
    //mCsect->leave("newAction");

    // always need this
    action->mobius = mMobius;

    return action;
}

void Actionator::freeAction(Action* a)
{
    //mCsect->enter("newAction");
    mActionPool->freeAction(a);
    //mCsect->leave("newAction");
}

Action* Actionator::cloneAction(Action* src)
{
    Action* action = NULL;

    //mCsect->enter("cloneAction");
    action = mActionPool->newAction(src);
    //mCsect->leave("cloneAction");

    // not always set if allocated outside
    action->mobius = mMobius;

    return action;
}

/**
 * Called when the action has finished processing.
 * Notify the listener if there is one.
 */
void Actionator::completeAction(Action* a)
{
    // TODO: listener

    // if an event is still set we're owned by the event
    // threadEvents don't imply ownership
    if (a->getEvent() == NULL)
      freeAction(a);
}

//////////////////////////////////////////////////////////////////////
//
// Action Execution
//
//////////////////////////////////////////////////////////////////////

/**
 * Processes the action list queued for the next audio block.
 * This is the way most actions are performed.
 *
 * The doAction() method below is called by a small number of internal
 * components that manufacture actions as a side effect of something other
 * than a trigger.
 */
void Actionator::doInterruptActions(UIAction* actions)
{
    // we do not delete these, they are converted to Action
    // and may have results in them, but the caller owns them
    UIAction* action = actions;
    while (action != nullptr) {
        doCoreAction(action);
        action = action->next;
    }
}

/**
 * We're bypassing some of the old logic with Mobius::doAction
 * that validated various things, and decided whether or not
 * to handle it in UI thread or defer it to the interrupt.
 * Instead we call doActionNow which is what the audio interrupt
 * handler did.  This loses the call to completeAction()
 * which we have to do out here to return the action allocated
 * with Mobius::newAction to the pool or else it will leak.
 *
 * Once this gets retooled to avoid the above/below interrupt
 * distinction, should just call the one doAction and let it
 * handle the pooling nuance.  In particular you can't just
 * call Mobius::freeAction because ownership may have transferreed
 * to an Event.
 */
void Actionator::doCoreAction(UIAction* action)
{
    Action* coreAction = nullptr;
    
    if (action->type == ActionFunction) {
        FunctionDefinition* f = action->implementation.function;
        if (f != nullptr) {
            Function* cf = mFunctionMap[f->ordinal];
            if (cf == nullptr) {
                Trace(1, "Actionator::doCoreAction Unable to send action, function not found %s\n",
                      f->getName());
            }
            else {
                coreAction = newAction();
                coreAction->assimilate(action);
                coreAction->implementation.function = cf;
            }
        }
    }
    else if (action->type == ActionParameter) {
        UIParameter* p = action->implementation.parameter;
        Parameter* cp = mapParameter(p);
        if (cp != nullptr) {
            coreAction = newAction();
            coreAction->assimilate(action);
            coreAction->implementation.parameter = cp;
        }
    }
    else if (action->type == ActionActivation) {
        // select a setup or preset
        // should have ordinals now...
        Trace(1, "Actionator::doCoreAction Activation action not implemented\n");
    }
    else {
        Trace(1, "Actionator::doCoreAction Unknown action type %s\n", action->type->getName());
    }

    if (coreAction != nullptr) {
        doActionNow(coreAction);
        completeAction(coreAction);
    }

}

/**
 * Process one action within the interrupt.  
 * This is also called directly by ScriptInterpreter and a few other
 * places.
 *
 * Old comments:
 * 
 * The Action is both an input and an output to this function.
 * It will not be freed but it may be returned with either the
 * mEvent or mKernelEvent fields set.  This is used by the 
 * script interpreter to schedule "Wait last" and "Wait thread" 
 * events.
 *
 * If an Action comes back with mEvent set, then the Action is
 * now owned by the Event and must not be freed by the caller.
 * It will be freed when the event is handled.  If mEvent is null
 * then the caller of doActionNow must return it to the pool.
 *
 * If the action is returned with mKernelEvent set it is NOT
 * owned and must be returned to the pool.  A ScriptInterpreter
 * may still wait on the event, but it will be notified in a different way.
 * 
 * This will replicate actions that use group scope or 
 * must obey focus lock.  If the action is replicated only the first
 * one is returned, the others are freed.  This is okay for scripts
 * since we'll never do replication if we're called from a script.
 *
 * TODO: Consider doing the replication outside the interrupt and
 * leave multiple Actions on the list.
 *
 * Internally the Action may be cloned if a function decides to 
 * schedule more than one event.  The Action object passed to 
 * Function::invoke must be returned with the "primary" event.
 */
void Actionator::doActionNow(Action* a)
{
    ActionType* t = a->getTarget();

    // not always set if comming from the outside
    a->mobius = mMobius;

    if (t == NULL) {
        Trace(1, "Action with no target!\n");
    }
    // brought over from the original doAction when we
    // merged that and doActionNow, hate the need to be testing
    // trigger state down here
    else if (a->isSustainable() && !a->down && t != ActionFunction) {
        // Currently functions and UIControls are the only things that support 
        // up transitions.  UIControls are messy, generalize this to 
        // be more like a parameter with trigger properties.
        Trace(2, "Ignoring up transition action\n");
    }
    else if (t == ActionFunction) {
        doFunction(a);
    }
    else if (t == ActionParameter) {
        doParameter(a);
    }
    else if (t == ActionPreset) {
        doPreset(a);
    }
    else if (t == ActionSetup) {
        doSetup(a);
    }
    else {
        Trace(1, "Actionator: Invalid action target %s\n", t->getName());
    }
}

/**
 * Handle a TargetPreset action.
 * Like the other config targets this is a bit messy because the
 * Action will have a resolved target pointing to a preset in the
 * external config, but we need to set one from the interrupt config.
 * Would be cleaner if we just referenced these by number.
 *
 * Prior to 2.0 we did not support focus on preset changes but since
 * we can bind them like any other target I think it makes sense now.
 * This may be a surprise for some users, consider a global parameter
 * similar to FocusLockFunctions to disable this?
 */
void Actionator::doPreset(Action* a)
{
    MobiusConfig* config = mMobius->getConfiguration();
    Preset* p = (Preset*)a->getTargetObject();
    if (p == NULL) {    
        // may be a dynamic action
        // support string args here too?
        int number = a->arg.getInt();
        if (number < 0)
          Trace(1, "Missing action Preset\n");
        else {
            p = GetPreset(config, number);
            if (p == NULL) 
              Trace(1, "Invalid preset number: %ld\n", (long)number);
        }
    }

    if (p != NULL) {
        int number = p->ordinal;

        Trace(2, "Preset action: %ld\n", (long)number);

        // determine the target track(s) and schedule events
        Track* track = resolveTrack(a);

        if (track != NULL) {
            track->setPreset(number);
        }
        else if (a->noGroup) {
            // selected track only
            track = mMobius->getTrack();
            track->setPreset(number);
        }
        else {
            // Apply to the current track, all focused tracks
            // and all tracks in the Action scope.
            int targetGroup = a->getTargetGroup();

            // might want a global param for this?
            bool allowPresetFocus = true;

            if (targetGroup > 0) {
                // only tracks in this group
                for (int i = 0 ; i < mMobius->getTrackCount() ; i++) {
                    Track* t = mMobius->getTrack(i);
                    if (targetGroup == t->getGroup())
                      t->setPreset(number);
                }
            }
            else if (allowPresetFocus) {
                for (int i = 0 ; i < mMobius->getTrackCount() ; i++) {
                    Track* t = mMobius->getTrack(i);
                    if (mMobius->isFocused(t))
                      t->setPreset(number);
                }
            }
        }
    }
}

/**
 * Process a TargetSetup action.
 * We have to change the setup in both the external and interrupt config,
 * the first so it can be seen and the second so it can be used.
 */
void Actionator::doSetup(Action* a)
{
    MobiusConfig* config = mMobius->getConfiguration();

    // If we're here from a Binding should have resolved
    Setup* s = (Setup*)a->getTargetObject();
    if (s == NULL) {
        // may be a dynamic action
        int number = a->arg.getInt();
        if (number < 0)
          Trace(1, "Missing action Setup\n");
        else {
            s = GetSetup(config, number);
            if (s == NULL) 
              Trace(1, "Invalid setup number: %ld\n", (long)number);
        }
    }

    if (s != NULL) {
        int number = s->ordinal;
        Trace(2, "Setup action: %ld\n", (long)number);

        // This is messy, the resolved target will
        // point to an object from the external config but we have 
        // to set one from the interrupt config by number
        SetCurrentSetup(config, number);
        mMobius->setSetupInternal(number);

        // special operator just for setups to cause it to be saved
        // UPDATE: This is old and questionable, it isn't used
        // anywhere in core code or scripts.  It was used in UI.cpp in response
        // to selecting a Setup from the main menu.  I don't think this should
        // necessarily mean to make a permanent change, though that would be convenient
        // rather than editing the full MobiusConfig.  But if you want that, just
        // have the UI do that it's own damn self rather than sending it all the way
        // down here, only to have a KernelEvent back
        // can remove this EventType
#if 0        
        if (a->actionOperator == OperatorPermanent) {
            ThreadEvent* te = new ThreadEvent(TE_SAVE_CONFIG);
            mMobius->addEvent(te);
        }
#endif
        
    }
}

/**
 * Process a function action.
 *
 * We will replicate the action if it needs to be sent to more than
 * one track due to group scope or focus lock.
 *
 * If a->down and a->longPress are both true, we're being called
 * after long-press detection.
 *
 */
void Actionator::doFunction(Action* a)
{
    // Client's won't set down in some trigger modes, but there is a lot
    // of code from here on down that looks at it
    if (a->triggerMode != TriggerModeMomentary)
      a->down = true;

    // Only functions track long-presses, though we could
    // in theory do this for other targets.  This may set a->longPress
    // on up transitions
    mTriggerState->assimilate(a);

    Function* f = (Function*)a->getTargetObject();
    if (f == NULL) {
        // should have caught this in doAction
        Trace(1, "Missing action Function\n");
    }
    else if (f->global) {
        // These are normally not track-specific and don't schedule events.
        // The one exception is RunScriptFunction which can be both
        // global and track-specififc.  If this is a script we'll
        // end up in runScript()
        if (!a->longPress)
          f->invoke(a, mMobius);
        else {
            // Most global functions don't handle long presses but
            // TrackGroup does.  Since we'll get longpress actions regardless
            // have to be sure not to call the normal invoke() method
            // ?? what about scripts
            f->invokeLong(a, mMobius);
        }
    }
    else {
        // determine the target track(s) and schedule events
        Track* track = resolveTrack(a);

        if (track != NULL) {
            doFunction(a, f, track);
        }
        else if (a->noGroup) {
            // selected track only
            doFunction(a, f, mMobius->getTrack());
        }
        else {
            // Apply to tracks in a group or focused
            Action* ta = a;
            int nactions = 0;
            int targetGroup = a->getTargetGroup();
            Track* active = mMobius->getTrack();
            
            for (int i = 0 ; i < mMobius->getTrackCount() ; i++) {
                Track* t = mMobius->getTrack(i);

                if ((targetGroup > 0 && targetGroup == t->getGroup()) ||
                    (targetGroup <= 0 &&
                     (t == active || (f->isFocusable() && mMobius->isFocused(t))))) {

                    // if we have more than one, have to clone the
                    // action so it can have independent life
                    if (nactions > 0)
                      ta = cloneAction(a);

                    doFunction(ta, f, t);

                    // since we only "return" the first one free the 
                    // replicants
                    if (nactions > 0)
                      completeAction(ta);

                    nactions++;
                }
            }
        }
    }
}

/**
 * Do a function action within a resolved track.
 *
 * We've got this weird legacy EDP feature where the behavior of the up
 * transition can be different if it was sustained long.  This is mostly
 * used to convret non-sustained functions into sustained functions, 
 * for example Long-Overdub becomes SUSOverdub and stops as soon as the
 * trigger is released.  I don't really like this 
 */
void Actionator::doFunction(Action* action, Function* f, Track* t)
{
    // set this so if we need to reschedule it will always go back
    // here and not try to do group/focus lock replication
    action->setResolvedTrack(t);

    if (action->down) { 
        if (action->longPress) {
            // Here via TriggerState when we detect a long-press,
            // call a different invocation method.
            // TODO: Think about just having Funcion::invoke check for the
            // longPress flag so we don't need two methods...
            // 
            // We're here if the Function said it supported long-press
            // but because of the Sustain Functions preset parameter,
            // there may be a track-specific override.  If the function
            // is sustainable (e.g. Record becomes SUSRecord) then this
            // disables long-press behavoir.

            Preset* p = t->getPreset();
            if (f->isSustain(p)) {
                // In this track, function is sustainable
                Trace(t, 2, "Ignoring long-press action for function that has become sustainable\n");
            }
            else {
                f->invokeLong(action, t->getLoop());
            }
        }
        else {
            // normal down invocation
            f->invoke(action, t->getLoop());

            // notify the script interpreter on each new invoke
            // !! sort out whether we wait for invokes or events
            // !! Script could want the entire Action
            // TODO: some (most?) manual functions should cancel
            // a script in progress?
            mMobius->resumeScript(t, f);
        }
    }
    else if (!action->isSustainable() || !f->isSustainable()) {
        // Up transition with a non-sustainable trigger or function, 
        // ignore the action.  Should have filtered these eariler?
        Trace(3, "Actionator::doFunction not a sustainable action\n");
    }
    else {
        // he's up!
        // let the function change how it ends
        if (action->longPress) {
            Function* alt = f->getLongPressFunction(action);
            if (alt != NULL && alt != f) {
                Trace(2, "Actionator::doFunction Long-press %s converts to %s\n",
                      f->getDisplayName(),
                      alt->getDisplayName());
            
                f = alt;
                // I guess put it back here just in case?
                // Not sure, this will lose the ResolvedTarget but 
                // that should be okay, the only thing we would lose is the
                // ability to know what the real target function was.
                //action->setFunction(alt);
            }
        }

        f->invoke(action, t->getLoop());
    }
}

/**
 * Process a parameter action.
 *
 * These are always processed synchronously, we may be inside or
 * outside the interrupt.  These don't schedule Events so the caller
 * is responsible for freeing the action.
 *
 * Also since these don't schedule Events, we can reuse the same
 * action if it needs to be replicated due to group scope or focus lock.
 */
void Actionator::doParameter(Action* a)
{
    Parameter* p = (Parameter*)a->getTargetObject();
    if (p == NULL) {
        Trace(1, "Missing action Parameter\n");
    }
    else if (p->scope == PARAM_SCOPE_GLOBAL) {
        // Action scope doesn't matter, there is only one
        doParameter(a, p, NULL);
    }
    else if (a->getTargetTrack() > 0) {
        // track specific binding
        Track* t = mMobius->getTrack(a->getTargetTrack() - 1);
        if (t != NULL)
          doParameter(a, p, t);
    }
    else if (a->getTargetGroup() > 0) {
        // group specific binding
        // !! We used to have some special handling for 
        // OutputLevel where it would remember relative positions
        // among the group.
        Action* ta = a;
        int nactions = 0;
        int group = a->getTargetGroup();
        for (int i = 0 ; i < mMobius->getTrackCount() ; i++) {
            Track* t = mMobius->getTrack(i);
            if (t->getGroup() == group) {
                if (p->scheduled && nactions > 0)
                  ta = cloneAction(a);
                  
                doParameter(ta, p, t);

                if (p->scheduled && nactions > 0)
                  completeAction(ta);
                nactions++;
            }
        }
    }
    else {
        // current track and focused
        // !! Only track parameters have historically obeyed focus lock
        // Preset parameters could be useful but I'm scared about   
        // changing this now
        if (p->scope == PARAM_SCOPE_PRESET) {
            doParameter(a, p, mMobius->getTrack());
        }
        else {
            Action* ta = a;
            int nactions = 0;
            for (int i = 0 ; i < mMobius->getTrackCount() ; i++) {
                Track* t = mMobius->getTrack(i);
                if (mMobius->isFocused(t)) {
                    if (p->scheduled && nactions > 0)
                      ta = cloneAction(a);

                    doParameter(ta, p, t);

                    if (p->scheduled && nactions > 0)
                      completeAction(ta);
                    nactions++;
                }
            }
        }
    }
}

/**
 * Process a parameter action once we've determined the target track.
 *
 * MIDI bindings pass the CC value or note velocity unscaled.
 * 
 * Key bindings will always have a zero value but may have bindingArgs
 * for relative operators.
 *
 * OSC bindings convert the float to an int scaled from 0 to 127.
 * !! If we let the float value come through we could do scaling
 * with a larger range which would be useful in few cases like
 * min/max tempo.
 *
 * Host bindings convert the float to an int scaled from 0 to 127.
 * 
 * When we pass the Action to the Parameter, the value in the
 * Action must have been properly scaled.  The value will be in
 * bindingArgs for strings and action.value for ints and bools.
 *
 */
void Actionator::doParameter(Action* a, Parameter* p, Track* t)
{
    ParameterType type = p->type;

    // set this so if we need to reschedule it will always go back
    // here and not try to do group/focus lock replication
    a->setResolvedTrack(t);

    if (type == TYPE_STRING) {
        // bindingArgs must be set
        // I suppose we could allow action.value be coerced to 
        // a string?
        p->setValue(a);
    }
    else { 
        int min = p->getLow();
        int max = p->getHigh(mMobius);
       
        if (min == 0 && max == 0) {
            // not a ranged type
            Trace(1, "Invalid parameter range\n");
        }
        else {
            // numeric parameters support binding args for relative changes
            a->parseBindingArgs();
            
            ActionOperator* op = a->actionOperator;
            if (op != NULL) {
                // apply relative commands
                Export exp(a);
                int current = p->getOrdinalValue(&exp);
                int neu = a->arg.getInt();

                if (op == OperatorMin) {
                    neu = min;
                }
                else if (op == OperatorMax) {
                    neu = max;
                }
                else if (op == OperatorCenter) {
                    neu = ((max - min) + 1) / 2;
                }
                else if (op == OperatorUp) {
                    int amount = neu;
                    if (amount == 0) amount = 1;
                    neu = current + amount;
                }
                else if (op == OperatorDown) {
                    int amount = neu;
                    if (amount == 0) amount = 1;
                    neu = current - amount;
                }
                // don't need to handle OperatorSet, just use the arg

                if (neu > max) neu = max;
                if (neu < min) neu = min;
                a->arg.setInt(neu);
            }

            p->setValue(a);
        }
    }
}

//////////////////////////////////////////////////////////////////////
//
// Paramaeters Access
//
// This doesn't really belong here since retrieving a value isn't
// an Action, but we're doing UI/Core function mapping here and
// this seems as good a place as any to have that for parameters.
//
//////////////////////////////////////////////////////////////////////

/**
 * The interface for this is evolving to use UIQuery but for now
 * the getParameter method will be called by the UI on MobiusShell
 * and shell will pass it directly to us without going through KernelMessages.
 *
 * It is expected to be UI thread safe and synchronous.
 *
 * This isn't used very often, only for the "control" parameters like output
 * level and feedback.  And for the "instant parameter" UI component that allows
 * ad-hoc parameter changes without activating an entire Preset.
 *
 * trackNumber follows the convention of UIAction with a value of zero
 * meaning the active track, and specific track numbers starting from 1.
 *
 * The values returned are expected to be "ordinals" in the new model.
 * 
 */
int Actionator::getParameter(UIParameter* p, int trackNumber)
{
    int value = 0;
    
    Parameter* cp = mapParameter(p);
    if (cp != nullptr) {
        value = getParameter(cp, trackNumber);
        if (strcmp(p->getName(), "output")) {
            //Trace(1, "Output track %d %d\n", trackNumber, value);
        }
    }

    return value;
}

/**
 * Parameter accessor after the UIParameter conversion.
 */
int Actionator::getParameter(Parameter* p, int trackNumber)
{
    int value = 0;
    
    Track* track = nullptr;
    if (trackNumber == 0) {
        // active track
        track = mMobius->getTrack();
    }
    else {
        track = mMobius->getTrack(trackNumber - 1);
        if (track == nullptr) {
            Trace(1, "Mobius::getParameter track number out of range %d\n", trackNumber);
        }
    }

    if (track != nullptr) {
        
        Export exp(mMobius);
        exp.setTarget(p, track);
        
        value = exp.getOrdinalValue();

        if (value < 0) {
            // this convention was followed for invalid Export configuration
            // not sure the new UI is prepared for this?
            Trace(1, "Mobius::getParameter Export unable to determine ordinal\n");
            value = 0;
        }
    }

    return value;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

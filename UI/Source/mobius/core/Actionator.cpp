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

#include "../../util/Trace.h"
#include "../../model/UIAction.h"
#include "../../model/UIParameter.h"

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

Actionator::Actionator()
{
    mTriggerState = new TriggerState();
    mActionPool = new ActionPool();
}

Actionator::~Actionator()
{
}

/**
 * New interface for actions.
 * Convert the UIAction model into an internal Action
 * which may have an indefinite lifeespan.
 */
void Actionator::doAction(UIAction* src)
{
    Action* internal = newAction();
    internal->assimilate(src);

    doActionNow(internal);
}

/****************************************************************************
 *                                                                          *
 *                                  ACTIONS                                 *
 *                                                                          *
 ****************************************************************************/

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
    // you normally don't do this, just delete them
    if (a->isRegistered())
      Trace(1, "Freeing a registered action!\n");

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

    // make sure this is off
    action->setRegistered(false);

    return action;
}

/****************************************************************************
 *                                                                          *
 *                              ACTION EXECUTION                            *
 *                                                                          *
 ****************************************************************************/

/**
 * Perform an action, either synchronously or scheduled for the next 
 * interrupt.  We assume ownership of the Action object and will free
 * it (or return it to the pool) when we're finished.
 *
 * This is the interface that must be called from anything "outside"
 * Mobius, which is any trigger that isn't the script interpreter.
 * Besides performing the Action, this is where we track down/up 
 * transitions and long presses.
 *
 * It may also be used by code "inside" the audio interrupt in which
 * case action->inInterrupt or TriggerEvent will be set.  
 *
 * If we're not in the interrupt, we usually defer all actions to the
 * beginning of the next interrupt.  The exceptions are small number
 * of global functions that have the "outsideInterrupt" option on.
 *
 * UI targets are always done synchronously since they don't effect 
 * the Mobius engine.
 * 
 * Originally we let TriggerHost run synchronously but that was wrong,
 * PluginParameter will track the last set value.
 *
 * Note that long press tracking is only done inside the interrupt
 * which means that the few functions that set outsideInterrupt and
 * the UI controls can't respond to long presses.  Seems fine.
 */
void Actionator::doAction(Action* a)
{
    bool ignore = false;
    bool defer = false;

    // catch auto-repeat on key triggers early
    // we can let these set controls and maybe parameters
    // but

    ActionType* target = a->getTarget();

    if (a->isRegistered()) {
        // have to clone these to do them...error in the UI
        Trace(1, "Attempt to execute a registered action!\n");
        ignore = true;
    }
    else if (a->repeat && a->triggerMode != TriggerModeContinuous) {
        Trace(3, "Ignoring auto-repeat action\n");
        ignore = true;
    }
    else if (a->isSustainable() && !a->down && 
             target != ActionFunction) {
        // Currently functions and UIControls are the only things that support 
        // up transitions.  UIControls are messy, generalize this to 
        // be more like a parameter with trigger properties.
        Trace(2, "Ignoring up transition action\n");
        ignore = true;
    }
    else if (a->down && a->longPress) {
        // this is the convention used by TriggerState to tell
        // us when a long-press has been reached on a previous trigger
        // we are in the interrupt and must immediately forward to the tracks
        // ?? would be better to do this as a new trigger type, 
        // like TriggerLong?  Not as easy to screw up but then we lose the 
        // original trigger type which might be interesting in scripts.
        // !! if we just use action->inInterrupt consistently we wouldn't
        // need to test this
        doActionNow(a);
    }
    else if (a->trigger == TriggerScript ||
             a->trigger == TriggerEvent ||
             // !! can't we use this reliably and not worry about trigger?
             a->inInterrupt) {
        // Script and Event triggers are in the interrupt
        // The UI targets don't have restrictions on when they can change.
        // Bindings are used outside the interrupt.

        doActionNow(a);
    }
    else if (target == ActionFunction) {

        Function* f = (Function*)a->getTargetObject();
        if (f == NULL) {
            Trace(1, "Missing action Function\n");
        }
        else if (f->global && f->outsideInterrupt) {
            // can do these immediately
            f->invoke(a, mMobius);
        }
        else if (mMobius->getInterrupts() == 0) {
            // audio stream isn't running, suppress most functions
            // !! this is really dangerous, revisit this
            if (f->runsWithoutAudio) {
                // Have to be very careful here, current functions are:
                // FocusLock, TrackGroup, TrackSelect.
                // Maybe it would be better to ignore these and popup
                // a message? If these are sustainable or long-pressable
                // the time won't advance
                Trace(2, "Audio stream not running, executing %s\n", 
                      f->getName());
                doActionNow(a);
            }
            else {
                Trace(2, "Audio stream not running, ignoring %s",
                      f->getName());
            }
        }
        else
          defer = true;
    }
    else if (target == ActionParameter) {
        // TODO: Many parameters are safe to set outside
        // defrering may cause UI flicker if the change
        // doesn't happen right away and we immediately do a refresh
        // that puts it back to the previous value
        defer = true;
    }
    else {
        // controls are going away, Setup has to be inside, 
        // not sure about Preset
        defer = true;
    }
    
    if (!ignore && defer) {
        // pre 2.0 we used a ring buffer in Track for this that
        // didn't require a csect, consider resurecting that?
        // !! should have a maximum on this list?
        //mCsect->enter("doAction");
        if (mLastAction == NULL)
          mActions = a;
        else 
          mLastAction->setNext(a);
        mLastAction = a;
        //mCsect->leave("doAction");
    }
    else if (!a->isRegistered()) {
        completeAction(a);
    }

}

/**
 * Process the action list when we're inside the interrupt.
 */
void Actionator::doInterruptActions()
{
    Action* actions = NULL;
    Action* next = NULL;

    //mCsect->enter("doAction");
    actions = mActions;
    mActions = NULL;
    mLastAction = NULL;
    //mCsect->leave("doAction");

    for (Action* action = actions ; action != NULL ; action = next) {
        next = action->getNext();

        action->setNext(NULL);
        action->inInterrupt = true;

        doActionNow(action);

        completeAction(action);
    }
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
    if (!a->isRegistered() && a->getEvent() == NULL)
      freeAction(a);
}

/**
 * Process one action within the interrupt.  
 * This is also called directly by ScriptInterpreter.
 *
 * The Action is both an input and an output to this function.
 * It will not be freed but it may be returned with either the
 * mEvent or mThreadEvent fields set.  This is used by the 
 * script interpreter to schedule "Wait last" and "Wait thread" 
 * events.
 *
 * If an Action comes back with mEvent set, then the Action is
 * now owned by the Event and must not be freed by the caller.
 * It will be freed when the event is handled.  If mEvent is null
 * then the caller of doActionNow must return it to the pool.
 *
 * If the action is returned with mThreadEvent set it is NOT
 * owned and must be returned to the pool.
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
    else if (t == ActionFunction) {
        doFunction(a);
    }
    else if (t == ActionParameter) {
        doParameter(a);
    }
    else if (t == ActionScript) {
        doScriptNotification(a);
    }
    else if (t == ActionPreset) {
        doPreset(a);
    }
    else if (t == ActionSetup) {
        doSetup(a);
    }
    else if (t == ActionBindings) {
        doBindings(a);
    }
    //else if (t == ActionUIConfig) {
    //// not supported yet, there is only one UIConfig
    //Trace(1, "UIConfig action not supported\n");
    //}
    else {
        Trace(1, "Invalid action target\n");
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
        if (a->actionOperator == OperatorPermanent) {
            // save it too, control flow is convoluted,
            // we could have done this when the Action
            // was recevied outside the interrupt
            ThreadEvent* te = new ThreadEvent(TE_SAVE_CONFIG);
            mMobius->addEvent(te);
        }
    }
}

/**
 * Process a TargetBindings action.
 * We can be outside the interrupt here.  All this does is
 * set the current overlay binding in mConfig which, we don't have
 * to phase it in, it will just be used on the next trigger.
 */
void Actionator::doBindings(Action* a)
{
#if 0    
    // If we're here from a Binding should have resolved
    BindingConfig* bc = (BindingConfig*)a->getTargetObject();
    if (bc == NULL) {
        // may be a dynamic action
        int number = a->arg.getInt();
        if (number < 0)
          Trace(1, "Missing action BindingConfig\n");
        else {
            bc = mConfig->getBindingConfig(number);
            if (bc == NULL) 
              Trace(1, "Invalid binding overlay number: %ld\n", (long)number);
        }
    }

    if (bc != NULL) {
        int number = bc->getNumber();
        Trace(2, "Bindings action: %ld\n", (long)number);
        mConfig->setOverlayBindingConfig(bc);

        // sigh, since getState doesn't export 

    }
#endif    
}

/**
 * Special internal target used to notify running scripts when
 * something interesting happens on the outside.
 * 
 * Currently there is only one of these, from MobiusTread when
 * it finishes processing a ThreadEvent that a script might be waiting on.
 *
 * Note that this has to be done by probing the active scripts rather than
 * remembering the invoking ScriptInterpreter in the event, because
 * ScriptInterpreters can die before the events they launch are finished.
 */
void Actionator::doScriptNotification(Action* a)
{
    if (a->trigger != TriggerThread)
      Trace(1, "Unexpected script notification trigger!\n");

    // unusual way of passing this in, but target object didn't seem
    // to make sense
    ThreadEvent* te = a->getThreadEvent();
    if (te == NULL)
      Trace(1, "Script notification action without ThreadEvent!\n");
    else {
        for (ScriptInterpreter* si = mMobius->getScripts() ; si != NULL ; 
             si = si->getNext()) {

            // this won't advance the script, it just prunes the reference
            si->finishEvent(te);
        }

        // The ThreadEvent is officially over, we get to reclaim it
        a->setThreadEvent(NULL);
        delete te;
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

/**
 * Do a function action within a resolved track.
 *
 * We've got this weird legacy EDP feature where the behavior of the up
 * transition can be different if it was sustained long.  This is mostly
 * used to convret non-sustained functions into sustained functions, 
 * for example Long-Overdub becomes SUSOverdub and stops as soon as the
 * trigger is released.  I don't really like this 
 *
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

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

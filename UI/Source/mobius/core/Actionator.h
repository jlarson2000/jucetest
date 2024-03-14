/**
 * Code related to the processing of UIActions sent to Mobius.
 * This is mostly old code factored out of Mobius to reduce the size.
 *
 * Mapping between the new UIAction model and old Action is done here.
 *
 */

#pragma once

class Actionator
{
  public:

    Actionator();
    ~Actionator();

    void doAction(UIAction* src);

    class Action* newAction();
    void freeAction(class Action* a);
    class Action* cloneAction(class Action* src);
    void doAction(class Action* a);
    void doInterruptActions();
    void completeAction(class Action* a);
    void doActionNow(class Action* a);
    void doPreset(class Action* a);
    void doSetup(class Action* a);
    void doBindings(class Action* a);
    void doScriptNotification(class Action* a);
    void doFunction(class Action* a);
    Track* resolveTrack(class Action* action);
    void doFunction(class Action* action, class Function* f, class Track* t);
    void doParameter(class Action* a);
    void doParameter(class Action* a, class Parameter* p, class Track* t);

  private:

    class Mobius* mMobius;
    class ActionPool* mActionPool;
    class Action* mRegisteredActions;
    class Action *mActions;
    class Action *mLastAction;
    class TriggerState* mTriggerState;

    
};


/**
 * Code related to the processing of UIActions sent to Mobius.
 * This is mostly old code factored out of Mobius to reduce the size.
 *
 * Mapping between the new UIAction model and old Action is done here.
 *
 */

#pragma once

#include <JuceHeader.h>


class Actionator
{
  public:

    Actionator(class Mobius* m);
    ~Actionator();
    
    void dump();
    int getParameter(UIParameter* p, int trackNumber);

    void doAction(UIAction* src);
    void doCoreAction(UIAction* action);

    // still necessary?
    void doAction(class Action* a);
    
    // not necessary any more
    void doActionNow(class Action* a);
    void doInterruptActions();

    class Action* newAction();
    void freeAction(class Action* a);
    class Action* cloneAction(class Action* src);
    void completeAction(class Action* a);
    Track* resolveTrack(class Action* action);

    // called by Mobius in beginAudioInterrupt
    void advanceTriggerState(int frames);

  private:

    class Mobius* mMobius;
    class ActionPool* mActionPool;
    class TriggerState* mTriggerState;
    class Action *mActions;
    class Action *mLastAction;

    // UI to core Function mapping
    // std::vector<class Function*> functionMap;
    juce::Array<class Function*> mFunctionMap;
    juce::Array<class Parameter*> mParameterMap;
    
    // needs to be done after core initialization because
    // some of the tables aren't set up until after
    void initFunctionMap();
    void initParameterMap();
    Parameter* mapParameter(UIParameter* uip);
    
    int getParameter(Parameter* p, int trackNumber);

    void doPreset(class Action* a);
    void doSetup(class Action* a);
    void doBindings(class Action* a);
    void doScriptNotification(class Action* a);
    void doFunction(class Action* a);
    void doFunction(class Action* action, class Function* f, class Track* t);
    void doParameter(class Action* a);
    void doParameter(class Action* a, class Parameter* p, class Track* t);

};


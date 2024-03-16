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
    
    // Parameter value access is in here too since
    // it has to do similar UI/core mapping and is small
    int getParameter(UIParameter* p, int trackNumber);

    // this used to be in Mobius but it was moved down
    // here with the rest of the action code, where should this live?
    Track* resolveTrack(class Action* action);

    // action object management
    class Action* newAction();
    void freeAction(class Action* a);
    class Action* cloneAction(class Action* src);
    void completeAction(class Action* a);

    // called by Mobius in beginAudioInterrupt
    void advanceTriggerState(int frames);

    // perform actions queued for the next interrupt
    void doInterruptActions(class UIAction* actions);

    // do an internally generated action
    void doActionNow(class Action* a);

  private:

    class Mobius* mMobius;
    class ActionPool* mActionPool;
    class TriggerState* mTriggerState;

    // UI to core Function mapping
    // std::vector<class Function*> functionMap;
    // !! has the dynamic growth problem, not so bad
    // since this doesn't change after initialization
    // but need to be clearer about when that happens
    class juce::Array<class Function*> mFunctionMap;
    class juce::Array<class Parameter*> mParameterMap;
    
    // needs to be done after core initialization because
    // some of the tables aren't set up until after
    void initFunctionMap();
    void initParameterMap();
    
    void doCoreAction(UIAction* action);

    void doPreset(class Action* a);
    void doSetup(class Action* a);
    void doFunction(class Action* a);
    void doFunction(class Action* action, class Function* f, class Track* t);
    void doParameter(class Action* a);
    void doParameter(class Action* a, class Parameter* p, class Track* t);

    Parameter* mapParameter(UIParameter* uip);
    int getParameter(Parameter* p, int trackNumber);

};


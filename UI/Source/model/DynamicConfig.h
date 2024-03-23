/**
 * Model for actions and parameters that can vary at runtime
 * and not defined with static objects or from the XML files.
 *
 * One of these is obtained by calling MobiusInterface::getDynamicConfig
 * after loading a MobiusConfig or being notified of an internal change
 * through the MobiusListener.
 *
 * This emerged to solve two problems related to scripts.
 * 
 *    - the need to know the names of callable scripts to show in
 *      the binding configuration panels
 *
 *    - the desire to have some scripts automatically install themselves
 *      as UI action buttons without manual button configuration
 *
 * I have lots of thoughs on how this could be expanded into something
 * far more flexible in the future, but for now we're focusing on just
 * getting scripts called with buttons for the unit tests.
 *
 * Need to work on the names:
 *   DynamicAction, ActionDefinition, distinct model for Functions vs Parameters...
 *
 * Inching toward Juce in the core by using Array rather than the
 * old-school linked lists.
 *
 * Added an error message list here to convey any parsing errors or other
 * problems encountered loading the configuration.  Not sure if this is where
 * it should go but it's marginally more convenient than having a bunch
 * of MobiusListener calls every time we see one.
 *
 */

#pragma once

#include <JuceHeader.h>

#include "ActionType.h"


class DynamicConfig
{
  public:

    DynamicConfig();
    DynamicConfig(DynamicConfig* src);
    ~DynamicConfig();

    juce::OwnedArray<class DynamicAction>* getActions();
    juce::StringArray* getErrors();
    
    void clearActions();
    void addAction(DynamicAction* action);

    void clearErrors();
    void addError(juce::String error);
    
  private:

    juce::OwnedArray<class DynamicAction> actions;
    juce::StringArray errors;
    
};

class DynamicAction
{
  public:

    DynamicAction();
    DynamicAction(DynamicAction* src);
    ~DynamicAction();

    ActionType* type = ActionScript;

    juce::String name;
    juce::String displayName;
    int ordinal = 0;

    // true if this action would like to be a UI button
    // might want to evolve this to a more general way
    // to create any binding type
    bool button = false;

  private:
};

        

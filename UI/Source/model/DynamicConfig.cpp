/**
 * Evolving model for actions derived from scripts.
 */

#include <JuceHeader.h>

#include "DynamicConfig.h"


//////////////////////////////////////////////////////////////////////
//
// DynamicConfig
//
//////////////////////////////////////////////////////////////////////

DynamicConfig::DynamicConfig()
{
}

// learn how to use copy constructors you moron
DynamicConfig::DynamicConfig(DynamicConfig* src)
{
    if (src != nullptr) {
        juce::OwnedArray<DynamicAction>* srcActions = src->getActions();
        for (int i = 0 ; i < srcActions->size() ; i++) {
            DynamicAction* a = (*srcActions)[i];
            actions.add(new DynamicAction(a));
        }
        // this has to copy easier, right?
        juce::StringArray* srcErrors = src->getErrors();
        for (int i = 0 ; i < srcErrors->size() ; i++) {
            errors.add((*srcErrors)[i]);
        }
    }
}

DynamicConfig::~DynamicConfig()
{
}

juce::OwnedArray<DynamicAction>* DynamicConfig::getActions()
{
    return &actions;
}

juce::StringArray* DynamicConfig::getErrors()
{
    return &errors;
}

void DynamicConfig::clearActions()
{
    actions.clear();
}

void DynamicConfig::addAction(DynamicAction* action)
{
    actions.add(action);
}

void DynamicConfig::clearErrors()
{
    errors.clear();
}

void DynamicConfig::addError(juce::String error)
{
    errors.add(error);
}

//////////////////////////////////////////////////////////////////////
//
// DynamicAction
//
//////////////////////////////////////////////////////////////////////

DynamicAction::DynamicAction()
{
}

// this one should be dead easy with a copy constructor
DynamicAction::DynamicAction(DynamicAction* src)
{
    if (src != nullptr) {
        type = src->type;
        name = src->name;
        displayName = src->displayName;
        ordinal = src->ordinal;
        button = src->button;
    }
}

DynamicAction::~DynamicAction()
{
}


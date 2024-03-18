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

DynamicAction::~DynamicAction()
{
}


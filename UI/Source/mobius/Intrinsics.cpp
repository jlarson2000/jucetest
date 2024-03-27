/**
 * An experiment with a light weight way to define, bind, and implement
 * action targets that behave like Functions but aren't full blown
 * Function objects.
 */

#include "../model/DynamicConfig.h"

#include "Intrinsics.h"

/**
 * Definition for static declaration.
 */
juce::OwnedArray<DynamicAction> Intrinsic::actions;

/**
 * Initialize the list of intrinsics.
 * Normally called once by MobiusShell.
 */
void Intrinsic::init()
{
    if (actions.size() == 0) {
        DynamicAction* a = new DynamicAction();
        a->type = ActionIntrinsic;
        a->name = Intrinsic::LoadScriptsName;
        a->ordinal = IntrinsicLoadScripts;
        actions.add(a);
    

        a = new DynamicAction();
        a->type = ActionIntrinsic;
        a->name = Intrinsic::LoadSamplesName;
        a->ordinal = IntrinsicLoadSamples;
        actions.add(a);
    
        a = new DynamicAction();
        a->type = ActionIntrinsic;
        a->name = Intrinsic::AnalyzeDiffName;
        a->ordinal = IntrinsicAnalyzeDiff;
        actions.add(a);
    }
}

/**
 * Lookup an intrinsic ordinal by name.
 */
IntrinsicId Intrinsic::getId(juce::String name)
{
    IntrinsicId id = IntrinsicInvalid;
    for (int i = 0 ; i < actions.size() ; i++) {
        DynamicAction* a = actions[i];
        if (a->name == name) {
            id = (IntrinsicId)a->ordinal;
            break;
        }
    }
    return id;
}

/**
 * Add the intrinsic functions to a DynamicConfig that
 * was expected to start clean, though it might have
 * scripts in it by now.
 *
 * The definitions are copied so the caller (UI) can own
 * the config for an indefinite period of time.
 */
void Intrinsic::addIntrinsics(DynamicConfig* config)
{
    for (int i = 0 ; i < actions.size() ; i++) {
        DynamicAction* src = actions[i];
        DynamicAction* copy = new DynamicAction(src);
        config->addAction(copy);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

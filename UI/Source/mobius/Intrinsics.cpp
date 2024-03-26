/**
 * An experiment with a light weight way to define, bind, and implement
 * action targets that behave like Functions but aren't full blown
 * Function objects.
 */

#include "../model/DynamicConfig.h"

#include "Intrinsics.h"

/**
 * Add the intrinsic functions to a DynamicConfig that
 * was expected to start clean, though it might have
 * scripts in it by now.
 */
void Intrinsic::addIntrinsics(DynamicConfig* config)
{
    DynamicAction* a = new DynamicAction();
    a->type = ActionIntrinsic;
    a->name = Intrinsic::LoadScriptsName;
    a->ordinal = IntrinsicLoadScripts;
    config->addAction(a);
    

    a = new DynamicAction();
    a->type = ActionIntrinsic;
    a->name = Intrinsic::LoadSamplesName;
    a->ordinal = IntrinsicLoadSamples;
    config->addAction(a);
    
    a = new DynamicAction();
    a->type = ActionIntrinsic;
    a->name = Intrinsic::TestDiffName;
    a->ordinal = IntrinsicTestDiff;
    config->addAction(a);
}


/**
 * Function necessary for scripts to trigger samples.
 * Only used internally by test scripts, in the new world
 * SampleTrigger is haneled by the kernel above the core.
 *
 * This is equivalent to the UI FunctionDefinition SamplePlay
 * but only used internally and we don't map them.
 * 
 */

#include "../../MobiusKernel.h"

#include "../Function.h"
#include "../Action.h"
#include "../Mobius.h"

class SampleTriggerFunction {
  public:
	SampleFunction(int index);
	void invoke(Action* action, Mobius* m);
};

SampleTriggerFunction SampleTriggerObj;
Function* SampleTrigger = &SampleTriggerObj;

PUBLIC SampleTriggerFunction::SampleTriggerFunction()
{
	global = true;
	noFocusLock = true;

    setName("Sample");
    scriptOnly = true;
}

void SampleTriggerFunction::invoke(Action* action, Mobius* m)
{
	if (action->down) {
		trace(action, m);

        int sampleIndex = action->arg.getInt();

        // args are 1 based, convert
        sampleIndex--;

        if (sampleIndex >= 0) {
            MobiusKernel* k = m->getKernel();
            k->sampleTrigger(sampleIndex);
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/**
 * Function necessary for scripts to send Alerts.
 */

#include "../../MobiusKernel.h"

#include "../Function.h"
#include "../Action.h"
#include "../Mobius.h"

class AlertFunction : public Function {
  public:
	AlertFunction();
	void invoke(Action* action, Mobius* m);
};

AlertFunction AlertObj;
Function* Alert = &AlertObj;

AlertFunction::AlertFunction()
{
    setName("Alert");
	global = true;
	noFocusLock = true;
    scriptOnly = true;
}

void AlertFunction::invoke(Action* action, Mobius* m)
{
	if (action->down) {
		trace(action, m);

        // smarter about overflow !!
        const char* msg = action->arg.getString();
        if (msg != nullptr) {
            KernelEvent* e = m->newKernelEvent();
            e->type = EventAlert;
            strcpy(e->arg1, msg);
            m->sendKernelEvent(e);
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/**
 * A simulator of the Mobius engine for UI testing.
 */

#include "../util/Trace.h"

#include "../model/MobiusConfig.h"
#include "../model/MobiusState.h"
#include "../model/UIAction.h"
#include "../model/XmlREnderer.h"

#include "MobiusSimulator.h"

MobiusSimulator::MobiusSimulator()
{
    // this is given to us later
    configuration = nullptr;
}

MobiusSimulator::~MobiusSimulator()
{
    delete configuration;
}

//////////////////////////////////////////////////////////////////////
//
// Configuration
//
//////////////////////////////////////////////////////////////////////

void MobiusSimulator::configure(MobiusConfig* config)
{
    // clone it so we can make internal modifications
    XmlRenderer xr;
    configuration = xr.clone(config);
}

//////////////////////////////////////////////////////////////////////
//
// State
//
//////////////////////////////////////////////////////////////////////

MobiusState* MobiusSimulator::getState()
{
    return &state;
}

//////////////////////////////////////////////////////////////////////
//
// Actions
//
//////////////////////////////////////////////////////////////////////

/**
 * Perform an action.
 * In the old interface, ownership of the Action is taken
 * so we can resolve and intern it.
 * Think more about this.  UI wants to hold copies of partially
 * resolved Actions
 */
void MobiusSimulator::doAction(UIAction* action)
{
    trace("Recieved action: %s\n", action->getDescription());
}

//////////////////////////////////////////////////////////////////////
//
// Tests
//
// Just a hook to force the engine to do something that isn't
// defined by any Actions
//
// Used early on and can delete eventually
//
//////////////////////////////////////////////////////////////////////

void MobiusSimulator::test()
{
    trace("Running a test...\n");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


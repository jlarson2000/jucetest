//
// left over from when Samples were handled in core, now they're
// up in kernel
//
// what's left here could still be interesting if we wanted
// to schedule a SmapleTriggerEvent for quantized playback
// of a Sample.  I don't think we ever did that, and if we
// want that someday, work it into the more general event timeline
// that isn't soo wrapped up in the legacy model
//

/*
 * Copyright (c) 2010 Jeffrey S. Larson  <jeff@circularlabs.com>
 * All rights reserved.
 * See the LICENSE file for the full copyright and license declaration.
 * 
 * ---------------------------------------------------------------------
 * 
 * Functions related to Sample triggering.
 *
 * !! This is declared global but I can't see how this will
 * be reliable as SampleTrack::trigger has dependencies on the
 * state of the interrupt.  Fucking with this is sensitive though
 * since so many of the unit tests depend on it.
 *
 */

#include <stdio.h>
#include <string.h>
#include <memory.h>

#include "../Action.h"
#include "../Event.h"
#include "../Function.h"
#include "../Loop.h"
#include "../Messages.h"
#include "../Mobius.h"

//////////////////////////////////////////////////////////////////////
//
// SampleTriggerEvent
//
// We don't currently schedule events for sample triggers, though
// I suppose a quantized trigger could be an interesting effect?
//
//////////////////////////////////////////////////////////////////////

class SampleTriggerEventType : public EventType {
  public:
	SampleTriggerEventType();
};

SampleTriggerEventType::SampleTriggerEventType()
{
	name = "SampleTrigger";
}

SampleTriggerEventType SampleTriggerEventObj;
EventType* SampleTriggerEvent = &SampleTriggerEventObj;

//////////////////////////////////////////////////////////////////////
//
// SampleTriggerFunction
//
//////////////////////////////////////////////////////////////////////

class SampleTriggerFunction : public ReplicatedFunction {
  public:
	SampleTriggerFunction(int index);
	void invoke(Action* action, Mobius* m);
    void doEvent(Loop* loop, Event* event);
};

// TODO: need a way to define these on the fly

Function* SampleN = new SampleTriggerFunction(0);
Function* Sample1 = new SampleTriggerFunction(1);
Function* Sample2 = new SampleTriggerFunction(2);
Function* Sample3 = new SampleTriggerFunction(3);
Function* Sample4 = new SampleTriggerFunction(4);
Function* Sample5 = new SampleTriggerFunction(5);
Function* Sample6 = new SampleTriggerFunction(6);
Function* Sample7 = new SampleTriggerFunction(7);
Function* Sample8 = new SampleTriggerFunction(8);

SampleTriggerFunction::SampleTriggerFunction(int i)
{
	eventType = SampleTriggerEvent;
	global = true;
	index = i;
	replicated = true;
	noFocusLock = true;

	setKey(MSG_FUNC_SAMPLE_TRIGGER);

    if (index == 0) {
        setName("Sample");
        scriptOnly = true;
    }
    else {
        sprintf(fullName, "Sample%d", i);
        setName(fullName);
    }
}

void SampleTriggerFunction::invoke(Action* action, Mobius* m)
{
	if (action->down) {
		trace(action, m);

        int sampleIndex = index;
        if (sampleIndex == 0)
          sampleIndex = action->arg.getInt();

        // args are 1 based, convert
        sampleIndex--;

        if (sampleIndex >= 0) {
            // got all the way here with no place to go!
            // ordinally would have handled this up in kernel/Recorder
            Trace(1, "SampleTriggerFunction::invoke went through all this work for nothing!\n");
            // m->sampleTrigger(action, sampleIndex);
        }
    }
}

/**
 * We don't schedule events for these yet, but may want to for quantization.
 */
void SampleTriggerFunction::doEvent(Loop* l, Event* e)
{
	Trace(l, 1, "SampleTriggerEvent: no handler defined\n");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

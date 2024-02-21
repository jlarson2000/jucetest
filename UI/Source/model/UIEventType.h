/*
 * System constants that define the types of events
 * that can be scheduled internally on the loop timeline.
 * No behavior is defined here, only that necessary for
 * showing them in the UI.
 *
 * In old code the class is EventType and the static
 * objects are strewn about all over, typically with their
 * Function definitions.  Ordinal mapping will be harder for these.
 */

#pragma once

#include "SystemConstant.h"

class UIEventType : public SystemConstant
{
  public:

    static std::vector<UIEventType*> Instances;

    UIEventType(const char* name, const char* displayName);
    UIEventType(const char* name, const char* displayName, bool start, bool end, bool weird);
    virtual ~UIEventType() {};

    // Characters used to represent this on the loop status timeline
    const char* timelineSymbol;

    // todo: will probably want refernces to icons at some point

    // when there is a start end pair, indiciates this is the start event
    bool isStart;
    bool isEnd;

    // flag I want to set for events I don't understand so we can
    // color them different if they happen
    bool isWeird;
    
};

/*
 * this is all of the types that the engine uses, as we progress determine
 * how many of these really need to be exposed to the UI.
 * Do we need really need pointer constants for these in the UI?
 * They'll always just be returned in MobiusState
 */

extern UIEventType* InvokeEventType;
extern UIEventType* ValidateEventType;
extern UIEventType* RecordEventType;
extern UIEventType* RecordStopEventType;
extern UIEventType* PlayEventType;
extern UIEventType* OverdubEventType;
extern UIEventType* MultiplyEventType;
extern UIEventType* MultiplyEndEventType;
extern UIEventType* InstantMultiplyEventType;
extern UIEventType* InstantDivideEventType;
extern UIEventType* InsertEventType;
extern UIEventType* InsertEndEventType;
extern UIEventType* StutterEventType;
extern UIEventType* ReplaceEventType;
extern UIEventType* SubstituteEventType;
extern UIEventType* LoopEventType;
extern UIEventType* CycleEventType;
extern UIEventType* SubCycleEventType;
extern UIEventType* ReverseEventType;
extern UIEventType* ReversePlayEventType;
extern UIEventType* SpeedEventType;
extern UIEventType* RateEventType;
extern UIEventType* PitchEventType;
extern UIEventType* BounceEventType;
extern UIEventType* MuteEventType;
extern UIEventType* PlayEventType;
extern UIEventType* JumpPlayEventType;
extern UIEventType* UndoEventType;
extern UIEventType* RedoEventType;
extern UIEventType* ScriptEventType;
extern UIEventType* StartPointEventType;
extern UIEventType* RealignEventType;
extern UIEventType* MidiStartEventType;
extern UIEventType* SwitchEventType;
extern UIEventType* ReturnEventType;
extern UIEventType* SUSReturnEventType;
extern UIEventType* TrackEventType;
extern UIEventType* RunScriptEventType;
extern UIEventType* SampleTriggerEventType;
extern UIEventType* SyncEventType;
extern UIEventType* SlipEventType;
extern UIEventType* MoveEventType;
extern UIEventType* ShuffleEventType;
extern UIEventType* SyncCheckEventType;
extern UIEventType* MidiOutEventType;

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

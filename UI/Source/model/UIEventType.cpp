/*
 * System constants that define the types of events
 * that can be scheduled internally on the loop timeline
 * and returned in the MobiusState.
 *
 * Since these are spread all over the old code I'm not exactly
 * sure what the internal/display names are so guess for now
 * and only require the display name.  Not even sure if we really
 * need the difference since these are not looked up by name.
 */

#include <vector>

#include "SystemConstant.h"
#include "UIEventType.h"

std::vector<UIEventType*> UIEventType::Instances;

UIEventType::UIEventType(const char* name, const char* symbol) :
    SystemConstant(name, nullptr)
{
    ordinal = Instances.size();
    Instances.push_back(this);
    
    // if the symbol is not set, this is hidden
    timelineSymbol = symbol;
    isStart = false;
    isEnd = false;
    isWeird = false;
}

UIEventType::UIEventType(const char* name, const char* symbol, bool s, bool e, bool w) :
    UIEventType(name, symbol)
{
    isStart = s;
    isEnd = e;
    isWeird = w;
}

UIEventType* UIEventType::find(const char* name)
{
    UIEventType* found = nullptr;
    for (int i = 0 ; i < Instances.size() ; i++) {
        UIEventType* t = Instances[i];
        if (strcmp(name, t->name) == 0) {
            found = t;
            break;
        }
    }
    return found;
}

// usualy ugly SystemConstant pointer dance for auto-free
// to avoid link conflicts with the same objects in old code
// use the Type suffix, but should really decide if we need these at all
// don't know if the internal names match since they're hard to pin down

// shouldn't try to display these on the timeline
UIEventType InvokeEventObj {"Invoke", "?"};
UIEventType* UIInvokeEventType = &InvokeEventObj;

UIEventType ValidateEventObj {"Validate", "V"};
UIEventType* UIValidateEventType = &ValidateEventObj;


UIEventType RecordEventObj {"Record", "R", true, false, false};
UIEventType* UIRecordEventType = &RecordEventObj;

// same symbol as Record, but isEnd will color it red
UIEventType RecordStopEventObj {"RecordStop", "R", false, true, false};
UIEventType* UIRecordStopEventType = &RecordStopEventObj;

UIEventType PlayEventObj {"Play", "P"};
UIEventType* UIPlayEventType = &PlayEventObj;

UIEventType OverdubEventObj {"Overdub", "O"};
UIEventType* UIOverdubEventType = &OverdubEventObj;

UIEventType MultiplyEventObj {"Multiply", "M", true, false, false};
UIEventType* UIMultiplyEventType = &MultiplyEventObj;

UIEventType MultiplyEndEventObj {"MultiplyEnd", "M", false, true, false};
UIEventType* UIMultiplyEndEventType = &MultiplyEndEventObj;

UIEventType InstantMultiplyEventObj {"InstantMultiply", "IM"};
UIEventType* UIInstantMultiplyEventType = &InstantMultiplyEventObj;

UIEventType InstantDivideEventObj {"InstantDivide", "ID"};
UIEventType* UIInstantDivideEventType = &InstantDivideEventObj;

UIEventType InsertEventObj {"Insert", "I", true, false, false};
UIEventType* UIInsertEventType = &InsertEventObj;

UIEventType InsertEndEventObj {"InsertEnd", "I", false, true, false};
UIEventType* UIInsertEndEventType = &InsertEndEventObj;

UIEventType StutterEventObj {"Stutter", "St"};
UIEventType* UIStutterEventType = &StutterEventObj;

UIEventType ReplaceEventObj {"Replace", "Rp"};
UIEventType* UIReplaceEventType = &ReplaceEventObj;

UIEventType SubstituteEventObj {"Substitute", "S"};
UIEventType* UISubstituteEventType = &SubstituteEventObj;

// I think the next three are internal only
UIEventType LoopEventObj {"Loop", "?"};
UIEventType* UILoopEventType = &LoopEventObj;

UIEventType CycleEventObj {"Cycle", "?"};
UIEventType* UICycleEventType = &CycleEventObj;

UIEventType SubCycleEventObj {"Subcycle", "?"};
UIEventType* UISubCycleEventType = &SubCycleEventObj;

UIEventType ReverseEventObj {"Reverse", "Rv"};
UIEventType* UIReverseEventType = &ReverseEventObj;

// I think internal due to latency compensation
UIEventType ReversePlayEventObj {"ReversePlay", "?"};
UIEventType* UIReversePlayEventType = &ReversePlayEventObj;

UIEventType SpeedEventObj {"Speed", "Sp"};
UIEventType* UISpeedEventType = &SpeedEventObj;

UIEventType RateEventObj {"Rate", "Ra"};
UIEventType* UIRateEventType = &RateEventObj;

UIEventType PitchEventObj {"Pitch", "Pi"};
UIEventType* UIPitchEventType = &PitchEventObj;

UIEventType BounceEventObj {"Bounce", "B"};
UIEventType* UIBounceEventType = &BounceEventObj;

UIEventType MuteEventObj {"Mute", "Mu"};
UIEventType* UIMuteEventType = &MuteEventObj;

// should be filtered
UIEventType JumpPlayEventObj {"Jump", "J"};
UIEventType* UIJumpPlayEventType = &JumpPlayEventObj;

UIEventType UndoEventObj {"Undo", "U"};
UIEventType* UIUndoEventType = &UndoEventObj;

UIEventType RedoEventObj {"Redo", "Re"};
UIEventType* UIRedoEventType = &RedoEventObj;

// how does this differ from RunScriptEventObj?
UIEventType ScriptEventObj {"Script", "Sc", false, false, true};
UIEventType* UIScriptEventType = &ScriptEventObj;

UIEventType StartPointEventObj {"StartPoint", "SP"};
UIEventType* UIStartPointEventType = &StartPointEventObj;

UIEventType RealignEventObj {"Realign", "Rl"};
UIEventType* UIRealignEventType = &RealignEventObj;

// probably only in scripts, but might be nice to see
UIEventType MidiStartEventObj {"MIDIStart", "Ms"};
UIEventType* UIMidiStartEventType = &MidiStartEventObj;

// these are common and really need an icon
UIEventType SwitchEventObj {"Switch", "LS"};
UIEventType* UISwitchEventType = &SwitchEventObj;

UIEventType ReturnEventObj {"Return", "Rt"};
UIEventType* UIReturnEventType = &ReturnEventObj;

// weird, I guess paired with ReturnEvent?
UIEventType SUSReturnEventObj {"SUSReturn", "Rt", false, false, true};
UIEventType* UISUSReturnEventType = &SUSReturnEventObj;

// pretty sure these are instant
UIEventType TrackEventObj {"Track", "Tk"};
UIEventType* UITrackEventType = &TrackEventObj;

// would be nice to capture the Script name in the event summary
// for the extended display
// wait, how does this differ from just ScriptEvent?
UIEventType RunScriptEventObj {"Script", "Sc", false, false, true};
UIEventType* UIRunScriptEventType = &RunScriptEventObj;

UIEventType SampleTriggerEventObj {"Sample", "Sm"};
UIEventType* UISampleTriggerEventType = &SampleTriggerEventObj;

// not sure if these can happen
UIEventType SyncEventObj {"Sync", "Sy"};
UIEventType* UISyncEventType = &SyncEventObj;

UIEventType SlipEventObj {"Slip", "Sl"};
UIEventType* UISlipEventType = &SlipEventObj;

UIEventType MoveEventObj {"Move", "Mv"};
UIEventType* UIMoveEventType = &MoveEventObj;

UIEventType ShuffleEventObj {"Shuffle", "Sh"};
UIEventType* UIShuffleEventType = &ShuffleEventObj;

// I think just somethign I use for debugging
UIEventType SyncCheckEventObj {"SyncCheck", "?"};
UIEventType* UISyncCheckEventType = &SyncCheckEventObj;

UIEventType MidiOutEventObj {"MIDIOut", "Mo"};
UIEventType* UIMidiOutEventType = &MidiOutEventObj;

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

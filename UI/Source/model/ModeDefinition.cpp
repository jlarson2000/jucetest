/**
 * Base model for major operating modes within the engine.
 * These are part of the MobiusState model.
 * A track will always be in one of these modes.
 *
 * The engine model uses an internal/display name pair withh
 * a usually lowercase internal name and an upcased display name.
 * Here we'll just use the upcase name.
 *
 * Think about this!!
 * We don't need definition objects at all in the UI, all we care about
 * is the name to show.  Functions should be free to invent any mode
 * they want without having to model something back in the UI layer.
 * MobiusState could just keep a buffer to hold the mode name string
 * but I don't want to mess with string copying.
 *
 * It would work to have MobiusState just have a SystemConstant*
 * to whatever the internal object is and get the name there.
 * Leave this blown out for now in case there is something
 * else of interest we want to capture about modes besides just the name.
 */

#include "../util/Trace.h"
#include "../util/Util.h"

#include "SystemConstant.h"
#include "ModeDefinition.h"

//////////////////////////////////////////////////////////////////////
//
// Global Registry
//
//////////////////////////////////////////////////////////////////////

/**
 * A registry of all modes, created as they are constructed.
 * Probably not necessary since modes are returned in MobiusState
 * we don't need to look them up by name.  This does also provide
 * the assignment of ordinals however.
 */
std::vector<ModeDefinition*> ModeDefinition::Modes;

void ModeDefinition::dumpModes()
{
    for (int i = 0 ; i < Modes.size() ; i++) {
        ModeDefinition* m = Modes[i];
        trace("Mode %s\n", m->getName());
    }
}

/**
 * Find a Function by name
 * This doesn't happen often so we can do a liner search.
 */
ModeDefinition* ModeDefinition::getMode(const char* name)
{
	ModeDefinition* found = nullptr;
	
	for (int i = 0 ; i < Modes.size() ; i++) {
		ModeDefinition* m = Modes[i];
		if (StringEqualNoCase(m->getName(), name)) {
            found = m;
            break;
        }
	}
	return found;
}

//////////////////////////////////////////////////////////////////////
//
// Base Class
//
//////////////////////////////////////////////////////////////////////

ModeDefinition::ModeDefinition(const char* name) :
    SystemConstant(name, nullptr)
{
    // add to the global registry
    ordinal = Modes.size();
    Modes.push_back(this);

    // todo, will want to flag deprecated functions
    // or better yet, just leave them out
}

ModeDefinition::~ModeDefinition()
{
}

//////////////////////////////////////////////////////////////////////
//
// Mode Definition Objects
//
//////////////////////////////////////////////////////////////////////

//
// Major Modes
//

// not technically a Loop mode, the mode the engine is
// logically in when all theh tracks are in Reset
ModeDefinition GlobalResetModeDef {"Global Reset"};
ModeDefinition* GlobalResetMode = &GlobalResetModeDef;

ModeDefinition ConfirmModeDef {"Confirm"};
ModeDefinition* ConfirmMode = &ConfirmModeDef;

ModeDefinition InsertModeDef {"Insert"};
ModeDefinition* InsertMode = &InsertModeDef;

ModeDefinition MultiplyModeDef {"Multiply"};
ModeDefinition* MultiplyMode = &MultiplyModeDef;

ModeDefinition MuteModeDef {"Mute"};
ModeDefinition* MuteMode = &MuteModeDef;

ModeDefinition OverdubModeDef {"Overdub"};
ModeDefinition* OverdubMode = &OverdubModeDef;

ModeDefinition PauseModeDef {"Pause"};
ModeDefinition* PauseMode = &PauseModeDef;

ModeDefinition PlayModeDef {"Play"};
ModeDefinition* PlayMode = &PlayModeDef;

ModeDefinition RecordModeDef {"Record"};
ModeDefinition* RecordMode = &RecordModeDef;

ModeDefinition RehearseModeDef {"Rehearse"};
ModeDefinition* RehearseMode = &RehearseModeDef;

ModeDefinition RehearseRecordModeDef {"RehearseRecord"};
ModeDefinition* RehearseRecordMode = &RehearseRecordModeDef;

ModeDefinition ReplaceModeDef {"Replace"};
ModeDefinition* ReplaceMode = &ReplaceModeDef;

ModeDefinition ResetModeDef {"Reset"};
ModeDefinition* ResetMode = &ResetModeDef;

ModeDefinition RunModeDef {"Run"};
ModeDefinition* RunMode = &RunModeDef;

ModeDefinition StutterModeDef {"Stutter"};
ModeDefinition* StutterMode = &StutterModeDef;

ModeDefinition SubstituteModeDef {"Substitute"};
ModeDefinition* SubstituteMode = &SubstituteModeDef;

ModeDefinition SwitchModeDef {"Switch"};
ModeDefinition* SwitchMode = &SwitchModeDef;

ModeDefinition SynchronizeModeDef {"Synchronize"};
ModeDefinition* SynchronizeMode = &SynchronizeModeDef;

ModeDefinition ThresholdModeDef {"Threshold"};
ModeDefinition* ThresholdMode = &ThresholdModeDef;


//
// Minor Modes
// 
// Mute and Overdub are both major and minor modes
//

ModeDefinition CaptureModeDef {"Capture"};
ModeDefinition* CaptureMode = &CaptureModeDef;

ModeDefinition GlobalMuteModeDef {"GlobalMute"};
ModeDefinition* GlobalMuteMode = &GlobalMuteModeDef;

ModeDefinition GlobalPauseModeDef {"GlobalPause"};
ModeDefinition* GlobalPauseMode = &GlobalPauseModeDef;

ModeDefinition HalfSpeedModeDef {"HalfSpeed"};
ModeDefinition* HalfSpeedMode = &HalfSpeedModeDef;

ModeDefinition MIDISyncMasterModeDef {"MIDISyncMaster"};
ModeDefinition* MIDISyncMasterMode = &MIDISyncMasterModeDef;

ModeDefinition PitchOctaveModeDef {"PitchOctave"};
ModeDefinition* PitchOctaveMode = &PitchOctaveModeDef;

ModeDefinition PitchStepModeDef {"PitchStep"};
ModeDefinition* PitchStepMode = &PitchStepModeDef;

ModeDefinition PitchBendModeDef {"PitchBend"};
ModeDefinition* PitchBendMode = &PitchBendModeDef;

ModeDefinition SpeedOctaveModeDef {"SpeedOctave"};
ModeDefinition* SpeedOctaveMode = &SpeedOctaveModeDef;

ModeDefinition SpeedStepModeDef {"SpeedStep"};
ModeDefinition* SpeedStepMode = &SpeedStepModeDef;

ModeDefinition SpeedBendModeDef {"SpeedBend"};
ModeDefinition* SpeedBendMode = &SpeedBendModeDef;

ModeDefinition SpeedToggleModeDef {"SpeedToggle"};
ModeDefinition* SpeedToggleMode = &SpeedToggleModeDef;

ModeDefinition TimeStretchModeDef {"TimeStretch"};
ModeDefinition* TimeStretchMode = &TimeStretchModeDef;

ModeDefinition ReverseModeDef {"Reverse"};
ModeDefinition* ReverseMode = &ReverseModeDef;

ModeDefinition SoloModeDef {"Solo"};
ModeDefinition* SoloMode = &SoloModeDef;

ModeDefinition SyncMasterModeDef {"SyncMaster"};
ModeDefinition* SyncMasterMode = &SyncMasterModeDef;

ModeDefinition TrackSyncMasterModeDef {"TrackSyncMaster"};
ModeDefinition* TrackSyncMasterMode = &TrackSyncMasterModeDef;

ModeDefinition WindowModeDef {"Window"};
ModeDefinition* WindowMode = &WindowModeDef;


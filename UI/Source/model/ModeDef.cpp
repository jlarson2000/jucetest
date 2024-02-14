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
#include "ModeDef.h"

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

ModeDefinition ConfirmModeDef {"Confirm"};
ModeDefinition InsertModeDef {"Insert"};
ModeDefinition MultiplyModeDef {"Multiply"};
ModeDefinition MuteModeDef {"Mute"};
ModeDefinition OverdubModeDef {"Overdub"};
ModeDefinition PauseModeDef {"Pause"};
ModeDefinition PlayModeDef {"Play"};
ModeDefinition RecordModeDef {"Record"};
ModeDefinition RehearseModeDef {"Rehearse"};
ModeDefinition RehearseRecordModeDef {"RehearseRecord"};
ModeDefinition ReplaceModeDef {"Replace"};
ModeDefinition ResetModeDef {"Reset"};
ModeDefinition RunModeDef {"Run"};
ModeDefinition StutterModeDef {"Stutter"};
ModeDefinition SubstituteModeDef {"Substitute"};
ModeDefinition SwitchModeDef {"Switch"};
ModeDefinition SynchronizeModeDef {"Synchronize"};
ModeDefinition ThresholdModeDef {"Threshold"};

//
// Minor Modes
// 
// Mute and Overdub are both major and minor modes
//

ModeDefinition CaptureModeDef {"Capture"};
ModeDefinition GlobalMuteModeDef {"GlobalMute"};
ModeDefinition GlobalPauseModeDef {"GlobalPause"};
ModeDefinition HalfSpeedModeDef {"HalfSpeed"};
ModeDefinition MIDISyncMasterModeDef {"MIDISyncMaster"};

ModeDefinition PitchOctaveModeDef {"PitchOctave"};
ModeDefinition PitchStepModeDef {"PitchStep"};
ModeDefinition PitchBendModeDef {"PitchBend"};
ModeDefinition SpeedOctaveModeDef {"SpeedOctave"};
ModeDefinition SpeedStepModeDef {"SpeedStep"};
ModeDefinition SpeedBendModeDef {"SpeedBend"};
ModeDefinition SpeedToggleModeDef {"SpeedToggle"};
ModeDefinition TimeStretchModeDef {"TimeStretch"};

ModeDefinition ReverseModeDef {"Reverse"};
ModeDefinition SoloModeDef {"Solo"};
ModeDefinition SyncMasterModeDef {"SyncMaster"};
ModeDefinition TrackSyncMasterModeDef {"TrackSyncMaster"};
ModeDefinition WindowModeDef {"Window"};

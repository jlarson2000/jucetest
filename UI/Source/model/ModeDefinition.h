/**
 * Base model for major operating modes within the engine.
 * These are part of the MobiusState model.
 * A track will always be in one of the major modes and
 * may be in zero or more minor modes.
 *
 * Like FunctionDefinition, the engine will subclass or map this
 * add mode specific properties and behavior.
 */

#pragma once

#include <vector>
#include "SystemConstant.h"

class ModeDefinition : public SystemConstant
{
  public:

	ModeDefinition(const char* name);
	virtual ~ModeDefinition();

    int ordinal;				// internal number for indexing

    //////////////////////////////////////////////////////////////////////
    // Global Function Registry
    //////////////////////////////////////////////////////////////////////

    static std::vector<ModeDefinition*> Modes;
    static void dumpModes();
	static ModeDefinition* getMode(const char* name);

};

// Major modes

extern ModeDefinition* GlobalResetMode;

extern ModeDefinition* ConfirmMode;
extern ModeDefinition* InsertMode;
extern ModeDefinition* MultiplyMode;
extern ModeDefinition* MuteMode;
extern ModeDefinition* OverdubMode;
extern ModeDefinition* PauseMode;
extern ModeDefinition* PlayMode;
extern ModeDefinition* RecordMode;
extern ModeDefinition* RehearseMode;
extern ModeDefinition* RehearseRecordMode;
extern ModeDefinition* ReplaceMode;
extern ModeDefinition* ResetMode;
extern ModeDefinition* RunMode;
extern ModeDefinition* StutterMode;
extern ModeDefinition* SubstituteMode;
extern ModeDefinition* SwitchMode;
extern ModeDefinition* SynchronizeMode;
extern ModeDefinition* ThresholdMode;


// Minor modes

extern ModeDefinition* CaptureMode;
extern ModeDefinition* GlobalMuteMode;
extern ModeDefinition* GlobalPauseMode;
extern ModeDefinition* HalfSpeedMode;
extern ModeDefinition* MIDISyncMasterMode;
extern ModeDefinition* PitchOctaveMode;
extern ModeDefinition* PitchStepMode;
extern ModeDefinition* PitchBendMode;
extern ModeDefinition* SpeedOctaveMode;
extern ModeDefinition* SpeedStepMode;
extern ModeDefinition* SpeedBendMode;
extern ModeDefinition* SpeedToggleMode;
extern ModeDefinition* TimeStretchMode;
extern ModeDefinition* ReverseMode;
extern ModeDefinition* SoloMode;
extern ModeDefinition* SyncMasterMode;
extern ModeDefinition* TrackSyncMasterMode;
extern ModeDefinition* WindowMode;

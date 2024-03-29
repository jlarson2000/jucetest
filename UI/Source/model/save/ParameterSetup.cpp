/*
 * Copyright (c) 2010 Jeffrey S. Larson  <jeff@circularlabs.com>
 * All rights reserved.
 * See the LICENSE file for the full copyright and license declaration.
 * 
 * ---------------------------------------------------------------------
 * 
 * Static object definitions for Setup parameters.
 * 
 * The target object here is a Setup.
 * Note that we do not keep a private trashable duplicate of the Setup object 
 * like we do for Presets, and sort of do for SetupTracks so any
 * change you make here will be made permanently in the Setup
 * used by the interrupt's MobiusConfig.
 *
 * !! Think about this.
 *
 */

#include <stdio.h>
#include <string.h>
//#include <memory.h>
#include <ctype.h>

#include "../util/Util.h"
#include "../util/Trace.h"
//#include "List.h"
//#include "MessageCatalog.h"
#include "../util/XmlModel.h"
#include "../util/XmlBuffer.h"

//#include "Action.h"
//#include "Audio.h"
//#include "Export.h"
//#include "Function.h"
//#include "Mobius.h"
//#include "MobiusConfig.h"
//#include "Mode.h"
//#include "Project.h"
//#include "Recorder.h"
//#include "Setup.h"
//#include "Track.h"
//#include "Script.h"
//#include "Synchronizer.h"

#include "Messages.h"
#include "ExValue.h"
#include "Parameter.h"
#include "Preset.h"
#include "Setup.h"

/****************************************************************************
 *                                                                          *
 *                              SETUP PARAMETER                             *
 *                                                                          *
 ****************************************************************************/

class SetupParameter : public Parameter
{
  public:

    SetupParameter(const char* name, int key) :
        Parameter(name, key) {
        scope = PARAM_SCOPE_SETUP;
    }

    SetupParameter(const char* name, const char* displayName) :
        Parameter(name, displayName) {
        scope = PARAM_SCOPE_SETUP;
    }

    /**
     * Overload the Parameter versions and cast to a Setup.
     */
	void getConfigValue(void* obj, ExValue* value) override;
	void setConfigValue(void* obj, ExValue* value) override;

    /**
     * Overload the Parameter versions and resolve to a Setup.
     */
	//void getValue(Export* exp, ExValue* value);
	//void setValue(Action* action);

    /**
     * Overload the Parameter version and resolve to a Setup.
     */
	//int getOrdinalValue(Export* exp);

    /**
     * These must always be overloaded.
     */
	virtual void getValue(Setup* s, ExValue* value) = 0;
	virtual void setValue(Setup* s, ExValue* value) = 0;

    /**
     * This must be overloaded by anything that supports ordinals.
     */
	virtual int getOrdinalValue(Setup* s);

  private:

    Setup* getTargetSetup(class Mobius* m);

};

void SetupParameter::getConfigValue(void* obj, ExValue* value)
{
    getValue((Setup*)obj, value);
}

void SetupParameter::setConfigValue(void* obj, ExValue* value)
{
    setValue((Setup*)obj, value);
}

/**
 * Locate the target setup for the export.
 */
/*
Setup* SetupParameter::getTargetSetup(Mobius* m)
{
    Setup* target = nullptr;
    MobiusConfig* iconfig = m->getInterruptConfiguration();
    if (iconfig != nullptr)
      target = iconfig->getCurrentSetup();

    if (target == nullptr)
		Trace(1, "SetupParameter: Unable to resolve setup!\n");

    return target;
}

void SetupParameter::getValue(Export* exp, ExValue* value)
{
	Setup* target = getTargetSetup(exp->getMobius());
    if (target != nullptr)
      getValue(target, value);
    else
      value->setNull();
}

int SetupParameter::getOrdinalValue(Export* exp)
{
    int value = -1;
	Setup* target = getTargetSetup(exp->getMobius());
	if (target != nullptr)
	  value = getOrdinalValue(target);
    return value;
}
*/

/**
 * This one we can't have a default implementation for, it must be overloaded.
 */
int SetupParameter::getOrdinalValue(Setup* s)
{
    Trace(1, "Parameter %s: getOrdinalValue(Setup) not overloaded!\n",
          getName());
    return -1;
}

/*
void SetupParameter::setValue(Action* action)
{
	Setup* target = getTargetSetup(action->mobius);
    if (target != nullptr)
      setValue(target, &(action->arg));
}
*/

//////////////////////////////////////////////////////////////////////
//
// DefaultSyncSource
//
//////////////////////////////////////////////////////////////////////

class DefaultSyncSourceParameterType : public SetupParameter
{
  public:
	DefaultSyncSourceParameterType();
    int getOrdinalValue(Setup* s);
	void getValue(Setup* s, ExValue* value);
	void setValue(Setup* s, ExValue* value);
};

const char* DEFAULT_SYNC_SOURCE_NAMES[] = {
	"none", "track", "out", "host", "midi", nullptr
};

int DEFAULT_SYNC_SOURCE_KEYS[] = {
	MSG_VALUE_SYNC_SOURCE_NONE,
	MSG_VALUE_SYNC_SOURCE_TRACK,
	MSG_VALUE_SYNC_SOURCE_OUT,
	MSG_VALUE_SYNC_SOURCE_HOST,
	MSG_VALUE_SYNC_SOURCE_MIDI,
	0
};

DefaultSyncSourceParameterType::DefaultSyncSourceParameterType() :
    SetupParameter("defaultSyncSource", "Default Sync Source")
{
    bindable = true;
	type = TYPE_ENUM;
	values = DEFAULT_SYNC_SOURCE_NAMES;
	//valueKeys = DEFAULT_SYNC_SOURCE_KEYS;
}

int DefaultSyncSourceParameterType::getOrdinalValue(Setup* s)
{
    // the enumeration has "Default" as the first item, we hide that
    int index = (int)s->getSyncSource();
    if (index > 0)
      index--;
    return index;
}

void DefaultSyncSourceParameterType::getValue(Setup* s, ExValue* value)
{
	value->setString(values[getOrdinalValue(s)]);
}

void DefaultSyncSourceParameterType::setValue(Setup* s, ExValue* value)
{
    // the enumeration has "Default" as the first item, we hide that
    if (value->getType() == EX_INT)
      s->setSyncSource((SyncSource)(value->getInt() + 1));
    else {
        int index = getEnum(value) + 1;
        s->setSyncSource((SyncSource)index);
    }
}

DefaultSyncSourceParameterType DefaultSyncSourceParameterObj;
Parameter* DefaultSyncSourceParameter = &DefaultSyncSourceParameterObj;

//////////////////////////////////////////////////////////////////////
//
// DefaultTrackSyncUnit
//
//////////////////////////////////////////////////////////////////////

class DefaultTrackSyncUnitParameterType : public SetupParameter
{
  public:
	DefaultTrackSyncUnitParameterType();
    int getOrdinalValue(Setup* s);
	void getValue(Setup* s, ExValue* value);
	void setValue(Setup* s, ExValue* value);
};

const char* DEFAULT_TRACK_SYNC_UNIT_NAMES[] = {
	"subcycle", "cycle", "loop", nullptr
};

int DEFAULT_TRACK_SYNC_UNIT_KEYS[] = {
	MSG_VALUE_TRACK_UNIT_SUBCYCLE,
	MSG_VALUE_TRACK_UNIT_CYCLE,
	MSG_VALUE_TRACK_UNIT_LOOP,
	0
};

DefaultTrackSyncUnitParameterType::DefaultTrackSyncUnitParameterType() :
    SetupParameter("defaultTrackSyncUnit", MSG_PARAM_DEFAULT_TRACK_SYNC_UNIT)
{
    bindable = true;
	type = TYPE_ENUM;
	values = DEFAULT_TRACK_SYNC_UNIT_NAMES;
	//valueKeys = DEFAULT_TRACK_SYNC_UNIT_KEYS;
}

int DefaultTrackSyncUnitParameterType::getOrdinalValue(Setup* s)
{
    // the enumeration has "Default" as the first item, we hide that
    int index = (int)s->getSyncTrackUnit();
    if (index > 0)
      index--;
    return index;
}

void DefaultTrackSyncUnitParameterType::getValue(Setup* s, ExValue* value)
{
	value->setString(values[getOrdinalValue(s)]);
}

void DefaultTrackSyncUnitParameterType::setValue(Setup* s, ExValue* value)
{
    // the enumeration has "Default" as the first item, we hide that
    if (value->getType() == EX_INT)
      s->setSyncTrackUnit((SyncTrackUnit)(value->getInt() + 1));
    else {
        int index = getEnum(value) + 1;
        s->setSyncTrackUnit((SyncTrackUnit)index);
    }
}

DefaultTrackSyncUnitParameterType DefaultTrackSyncUnitParameterObj;
Parameter* DefaultTrackSyncUnitParameter = &DefaultTrackSyncUnitParameterObj;

//////////////////////////////////////////////////////////////////////
//
// SyncUnit
//
//////////////////////////////////////////////////////////////////////

class SlaveSyncUnitParameterType : public SetupParameter
{
  public:
	SlaveSyncUnitParameterType();
    int getOrdinalValue(Setup* s);
	void getValue(Setup* s, ExValue* value);
	void setValue(Setup* s, ExValue* value);
};

const char* SYNC_UNIT_NAMES[] = {
	"beat", "bar", nullptr
};

int SYNC_UNIT_KEYS[] = {
	MSG_VALUE_SYNC_UNIT_BEAT,
	MSG_VALUE_SYNC_UNIT_BAR,
	0
};

SlaveSyncUnitParameterType::SlaveSyncUnitParameterType() :
    SetupParameter("slaveSyncUnit", MSG_PARAM_SYNC_UNIT)
{
    bindable = true;
	type = TYPE_ENUM;
	values = SYNC_UNIT_NAMES;
	//valueKeys = SYNC_UNIT_KEYS;
}

int SlaveSyncUnitParameterType::getOrdinalValue(Setup* s)
{
	return (int)s->getSyncUnit();
}

void SlaveSyncUnitParameterType::getValue(Setup* s, ExValue* value)
{
	value->setString(values[(int)s->getSyncUnit()]);
}

void SlaveSyncUnitParameterType::setValue(Setup* s, ExValue* value)
{
	s->setSyncUnit((SyncUnit)getEnum(value));
}

SlaveSyncUnitParameterType SlaveSyncUnitParameterObj;
Parameter* SlaveSyncUnitParameter = &SlaveSyncUnitParameterObj;

//////////////////////////////////////////////////////////////////////
//
// ManualStart
//
//////////////////////////////////////////////////////////////////////

class ManualStartParameterType : public SetupParameter
{
  public:
	ManualStartParameterType();
    int getOrdinalValue(Setup* s);
	void getValue(Setup* s, ExValue* value);
	void setValue(Setup* s, ExValue* value);
};

ManualStartParameterType::ManualStartParameterType() :
    SetupParameter("manualStart", MSG_PARAM_MANUAL_START)
{
    bindable = true;
	type = TYPE_BOOLEAN;
}

int ManualStartParameterType::getOrdinalValue(Setup* s)
{
	return (int)s->isManualStart();
}

void ManualStartParameterType::getValue(Setup* s, ExValue* value)
{
	value->setBool(s->isManualStart());
}

void ManualStartParameterType::setValue(Setup* s, ExValue* value)
{
	s->setManualStart(value->getBool());
}

ManualStartParameterType ManualStartParameterObj;
Parameter* ManualStartParameter = &ManualStartParameterObj;

//////////////////////////////////////////////////////////////////////
//
// MinTempo
//
//////////////////////////////////////////////////////////////////////

class MinTempoParameterType : public SetupParameter
{
  public:
	MinTempoParameterType();
    int getOrdinalValue(Setup* s);
	void getValue(Setup* s, ExValue* value);
	void setValue(Setup* s, ExValue* value);
};

MinTempoParameterType::MinTempoParameterType() :
    SetupParameter("minTempo", MSG_PARAM_MIN_TEMPO)
{
    bindable = true;
	type = TYPE_INT;
    high = 500;
}

int MinTempoParameterType::getOrdinalValue(Setup* s)
{
	return s->getMinTempo();
}

void MinTempoParameterType::getValue(Setup* s, ExValue* value)
{
	value->setInt(s->getMinTempo());
}

void MinTempoParameterType::setValue(Setup* s, ExValue* value)
{
	s->setMinTempo(value->getInt());
}

MinTempoParameterType MinTempoParameterObj;
Parameter* MinTempoParameter = &MinTempoParameterObj;

//////////////////////////////////////////////////////////////////////
//
// MaxTempo
//
//////////////////////////////////////////////////////////////////////

class MaxTempoParameterType : public SetupParameter
{
  public:
	MaxTempoParameterType();
    int getOrdinalValue(Setup* s);
	void getValue(Setup* s, ExValue* value);
	void setValue(Setup* s, ExValue* value);
};

MaxTempoParameterType::MaxTempoParameterType() :
    SetupParameter("maxTempo", MSG_PARAM_MAX_TEMPO)
{
    bindable = true;
	type = TYPE_INT;
    high = 500;
}

int MaxTempoParameterType::getOrdinalValue(Setup* s)
{
	return s->getMaxTempo();
}

void MaxTempoParameterType::getValue(Setup* s, ExValue* value)
{
	value->setInt(s->getMaxTempo());
}

void MaxTempoParameterType::setValue(Setup* s, ExValue* value)
{
	s->setMaxTempo(value->getInt());
}

MaxTempoParameterType MaxTempoParameterObj;
Parameter* MaxTempoParameter = &MaxTempoParameterObj;

//////////////////////////////////////////////////////////////////////
//
// BeatsPerBar
//
//////////////////////////////////////////////////////////////////////

class BeatsPerBarParameterType : public SetupParameter
{
  public:
	BeatsPerBarParameterType();
    int getOrdinalValue(Setup* s);
	void getValue(Setup* s, ExValue* value);
	void setValue(Setup* s, ExValue* value);
};

BeatsPerBarParameterType::BeatsPerBarParameterType() :
    SetupParameter("beatsPerBar", MSG_PARAM_BEATS_PER_BAR)
{
    bindable = true;
	type = TYPE_INT;
    high = 64;
}

int BeatsPerBarParameterType::getOrdinalValue(Setup* s)
{
	return s->getBeatsPerBar();
}

void BeatsPerBarParameterType::getValue(Setup* s, ExValue* value)
{
	value->setInt(s->getBeatsPerBar());
}

void BeatsPerBarParameterType::setValue(Setup* s, ExValue* value)
{
	s->setBeatsPerBar(value->getInt());
}

BeatsPerBarParameterType BeatsPerBarParameterObj;
Parameter* BeatsPerBarParameter = &BeatsPerBarParameterObj;

//////////////////////////////////////////////////////////////////////
//
// MuteSyncMode
//
//////////////////////////////////////////////////////////////////////

class MuteSyncModeParameterType : public SetupParameter
{
  public:
	MuteSyncModeParameterType();
    int getOrdinalValue(Setup* s);
	void getValue(Setup* s, ExValue* value);
	void setValue(Setup* s, ExValue* value);
};

const char* MUTE_SYNC_NAMES[] = {
	"transport", "transportClocks", "clocks", "none", nullptr
};

int MUTE_SYNC_KEYS[] = {
	MSG_VALUE_MUTE_SYNC_TRANSPORT,
	MSG_VALUE_MUTE_SYNC_TRANSPORT_CLOCKS,
	MSG_VALUE_MUTE_SYNC_CLOCKS,
	MSG_VALUE_MUTE_SYNC_NONE,
	0
};

MuteSyncModeParameterType::MuteSyncModeParameterType() :
    SetupParameter("muteSyncMode", MSG_PARAM_MUTE_SYNC_MODE)
{
    bindable = true;
	type = TYPE_ENUM;
	values = MUTE_SYNC_NAMES;
	//valueKeys = MUTE_SYNC_KEYS;
}

int MuteSyncModeParameterType::getOrdinalValue(Setup* s)
{
	return (int)s->getMuteSyncMode();
}

void MuteSyncModeParameterType::getValue(Setup* s, ExValue* value)
{
	value->setString(values[(int)s->getMuteSyncMode()]);
}

void MuteSyncModeParameterType::setValue(Setup* s, ExValue* value)
{
	s->setMuteSyncMode((MuteSyncMode)getEnum(value));
}

MuteSyncModeParameterType MuteSyncModeParameterObj;
Parameter* MuteSyncModeParameter = &MuteSyncModeParameterObj;

//////////////////////////////////////////////////////////////////////
//
// ResizeSyncAdjust
//
//////////////////////////////////////////////////////////////////////

class ResizeSyncAdjustParameterType : public SetupParameter
{
  public:
	ResizeSyncAdjustParameterType();
    int getOrdinalValue(Setup* s);
	void getValue(Setup* s, ExValue* value);
	void setValue(Setup* s, ExValue* value);
};

const char* SYNC_ADJUST_NAMES[] = {
	"none", "tempo", nullptr
};

int SYNC_ADJUST_KEYS[] = {
	MSG_VALUE_SYNC_ADJUST_NONE,
	MSG_VALUE_SYNC_ADJUST_TEMPO,
	0
};

ResizeSyncAdjustParameterType::ResizeSyncAdjustParameterType() :
    SetupParameter("resizeSyncAdjust", MSG_PARAM_RESIZE_SYNC_ADJUST)
{
    bindable = true;
	type = TYPE_ENUM;
	values = SYNC_ADJUST_NAMES;
	//valueKeys = SYNC_ADJUST_KEYS;
}

int ResizeSyncAdjustParameterType::getOrdinalValue(Setup* s)
{
	return (int)s->getResizeSyncAdjust();
}

void ResizeSyncAdjustParameterType::getValue(Setup* s, ExValue* value)
{
	value->setString(values[(int)s->getResizeSyncAdjust()]);
}

void ResizeSyncAdjustParameterType::setValue(Setup* s, ExValue* value)
{
	s->setResizeSyncAdjust((SyncAdjust)getEnum(value));
}

ResizeSyncAdjustParameterType ResizeSyncAdjustParameterObj;
Parameter* ResizeSyncAdjustParameter = &ResizeSyncAdjustParameterObj;

//////////////////////////////////////////////////////////////////////
//
// SpeedSyncAdjust
//
//////////////////////////////////////////////////////////////////////

class SpeedSyncAdjustParameterType : public SetupParameter
{
  public:
	SpeedSyncAdjustParameterType();
    int getOrdinalValue(Setup* s);
	void getValue(Setup* s, ExValue* value);
	void setValue(Setup* s, ExValue* value);
};

SpeedSyncAdjustParameterType::SpeedSyncAdjustParameterType() :
    SetupParameter("speedSyncAdjust", MSG_PARAM_SPEED_SYNC_ADJUST)
{
    bindable = true;
	type = TYPE_ENUM;
	values = SYNC_ADJUST_NAMES;
	//valueKeys = SYNC_ADJUST_KEYS;
}

int SpeedSyncAdjustParameterType::getOrdinalValue(Setup* s)
{
	return (int)s->getSpeedSyncAdjust();
}

void SpeedSyncAdjustParameterType::getValue(Setup* s, ExValue* value)
{
	value->setString(values[(int)s->getSpeedSyncAdjust()]);
}

void SpeedSyncAdjustParameterType::setValue(Setup* s, ExValue* value)
{
	s->setSpeedSyncAdjust((SyncAdjust)getEnum(value));
}

SpeedSyncAdjustParameterType SpeedSyncAdjustParameterObj;
Parameter* SpeedSyncAdjustParameter = &SpeedSyncAdjustParameterObj;

//////////////////////////////////////////////////////////////////////
//
// RealignTime
//
//////////////////////////////////////////////////////////////////////

class RealignTimeParameterType : public SetupParameter
{
  public:
	RealignTimeParameterType();
    int getOrdinalValue(Setup* s);
	void getValue(Setup* s, ExValue* value);
	void setValue(Setup* s, ExValue* value);
};

const char* REALIGN_TIME_NAMES[] = {
    "start", "bar", "beat", "now", nullptr
};

int REALIGN_TIME_KEYS[] = {
	MSG_VALUE_REALIGN_START,
	MSG_VALUE_REALIGN_BAR,
	MSG_VALUE_REALIGN_BEAT,
	MSG_VALUE_REALIGN_NOW,
	0
};

RealignTimeParameterType::RealignTimeParameterType() :
    SetupParameter("realignTime", MSG_PARAM_REALIGN_TIME)
{
    bindable = true;
	type = TYPE_ENUM;
	values = REALIGN_TIME_NAMES;
	//valueKeys = REALIGN_TIME_KEYS;
}

int RealignTimeParameterType::getOrdinalValue(Setup* s)
{
	return (int)s->getRealignTime();
}

void RealignTimeParameterType::getValue(Setup* s, ExValue* value)
{
	value->setString(values[(int)s->getRealignTime()]);
}

void RealignTimeParameterType::setValue(Setup* s, ExValue* value)
{
	s->setRealignTime((RealignTime)getEnum(value));
}

RealignTimeParameterType RealignTimeParameterObj;
Parameter* RealignTimeParameter = &RealignTimeParameterObj;

//////////////////////////////////////////////////////////////////////
//
// OutRealignMode
//
//////////////////////////////////////////////////////////////////////

class OutRealignModeParameterType : public SetupParameter
{
  public:
	OutRealignModeParameterType();
    int getOrdinalValue(Setup* s);
	void getValue(Setup* s, ExValue* value);
	void setValue(Setup* s, ExValue* value);
};

const char* REALIGN_MODE_NAMES[] = {
	"midiStart", "restart", nullptr
};

int REALIGN_MODE_KEYS[] = {
	MSG_VALUE_REALIGN_MIDI_START,
	MSG_VALUE_REALIGN_RESTART,
	0
};

OutRealignModeParameterType::OutRealignModeParameterType() :
    SetupParameter("outRealign", MSG_PARAM_OUT_REALIGN_MODE)
{
    bindable = true;
	type = TYPE_ENUM;
	values = REALIGN_MODE_NAMES;
	//valueKeys = REALIGN_MODE_KEYS;
}

int OutRealignModeParameterType::getOrdinalValue(Setup* s)
{
	return (int)s->getOutRealignMode();
}

void OutRealignModeParameterType::getValue(Setup* s, ExValue* value)
{
	value->setString(values[(int)s->getOutRealignMode()]);
}

void OutRealignModeParameterType::setValue(Setup* s, ExValue* value)
{
    // upgrade old value
    const char* str = value->getString();
    if (StringEqualNoCase(str, "retrigger"))
      value->setString("restart");

	s->setOutRealignMode((OutRealignMode)getEnum(value));
}

OutRealignModeParameterType OutRealignModeParameterObj;
Parameter* OutRealignModeParameter = &OutRealignModeParameterObj;

//////////////////////////////////////////////////////////////////////
//
// InitialTrack
//
// This was not in the original Parameter model, SetupDialog displayed
// it and it could be captured, but was not intended for real-time
// control.  It was only used by Mobius::installConfiguration
//
// Compare this with the global TrackParameter
// That was used I think for bindings to allow current track
// to be set as a parameter rather than a function.
//
// It is related to be different than this one which is the track
// that becomes active when the setup is loaded.
//
//////////////////////////////////////////////////////////////////////

class InitialTrackParameterType : public SetupParameter
{
  public:
	InitialTrackParameterType();
    int getOrdinalValue(Setup* s);
	void getValue(Setup* s, ExValue* value);
	void setValue(Setup* s, ExValue* value);
};

InitialTrackParameterType::InitialTrackParameterType() :
    SetupParameter("activeTrack", "Active Track")
{
	type = TYPE_INT;
    // will need this to be configurable!
    high = 8;
}

int InitialTrackParameterType::getOrdinalValue(Setup* s)
{
	return s->getActiveTrack();
}

void InitialTrackParameterType::getValue(Setup* s, ExValue* value)
{
	value->setInt(s->getActiveTrack());
}

void InitialTrackParameterType::setValue(Setup* s, ExValue* value)
{
	s->setActiveTrack(value->getInt());
}

InitialTrackParameterType InitialTrackParameterObj;
Parameter* InitialTrackParameter = &InitialTrackParameterObj;

//////////////////////////////////////////////////////////////////////
//
// Resetables
//
// Not in the original Mobius model
// This is multi-valued and I don't want to mess with problems
// overflowing the ExValueList model.  Since this is intended
// only for the UI and can't be bound or changed in scripts
// We're introducing alternate juce::var accessors.
// You must NOT use these in core since there is potential
// memory allocation.  Think more about this as it evolves.
//
// The XML model is a csv is can be long and won't fit in
// an ExValue and I don't want to mess with ExValueList just
// for the UI.
//
//////////////////////////////////////////////////////////////////////

class ResetablesParameterType : public SetupParameter
{
  public:
	ResetablesParameterType();
	void getValue(Setup* s, ExValue* value);
	void setValue(Setup* s, ExValue* value);
    void getJuceValue(void* object, juce::var& value) override;
    void setJuceValue(void* object, juce::var& value) override;
};

ResetablesParameterType::ResetablesParameterType() :
    SetupParameter("resetables", "Restore After Reset")
{
	type = TYPE_STRING;
    multi = true;
    bindable = false;
    juceValues = true;
}

void ResetablesParameterType::getValue(Setup* s, ExValue* value)
{
    value->setInt(0);
}

void ResetablesParameterType::setValue(Setup* s, ExValue* value)
{
}

// think: if we're going to be adding Juce specific accessors
// may as well have them in the model we want to use, in this
// case StringArray

void ResetablesParameterType::getJuceValue(void* s, juce::var& value)
{
    // currently a StringList, need to consistently convert these
    // when parsing XML
    StringList* values = ((Setup*)s)->getResetables();
    if (values != nullptr) {
        char* csv = values->toCsv();
        value = csv;
        delete csv;
    }
}

void ResetablesParameterType::setJuceValue(void* s, juce::var& value)
{
    const char* csv = value.toString().toUTF8();
    ((Setup*)s)->setResetables(new StringList(csv));
}

ResetablesParameterType ResetablesParameterObj;
Parameter* ResetablesParameter = &ResetablesParameterObj;

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

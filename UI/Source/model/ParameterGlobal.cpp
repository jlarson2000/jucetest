/*
 * Copyright (c) 2010 Jeffrey S. Larson  <jeff@circularlabs.com>
 * All rights reserved.
 * See the LICENSE file for the full copyright and license declaration.
 * 
 * ---------------------------------------------------------------------
 * 
 * Static object definitions for global parameters.
 * 
 * These are accessible from scripts though we most cannot be bound.
 *
 * Like SetupParmeters, there is no private copy of the MobiusConfig that
 * gets modified, we will directly modify the real MobiusConfig so the
 * change may persist.  
 *
 * If the parameter is cached somewhere, we handle the propgation to
 * whatever internal object is caching it.   Where we can we modify
 * both the "external" MobiusConfig and the "interrupt" MobiusConfig.
 *
 * Few of these are flagged "ordinal" so they can be seen in the UI.
 * Most could but I'm trying to reduce clutter and questions.
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
#include "Messages.h"
//#include "Mobius.h"
//#include "Mode.h"
//#include "Project.h"
//#include "Recorder.h"
//#include "Setup.h"
//#include "Track.h"
//#include "Script.h"
//#include "Synchronizer.h"

#include "ExValue.h"
#include "Parameter.h"
#include "Setup.h"
#include "MobiusConfig.h"

// we propagate some things to Track*
// hide this for awhile and refactor
#define HIDE_TRACK
#define HIDE_EXPORT
#define HIDE_MOBIUS

/****************************************************************************
 *                                                                          *
 *   						  GLOBAL PARAMETER                              *
 *                                                                          *
 ****************************************************************************/

class GlobalParameter : public Parameter {
  public:

    GlobalParameter(const char* name, int key) :
        Parameter(name, key) {
        scope = PARAM_SCOPE_GLOBAL;
        mComplained = false;
    }

    /**
     * Overload the Parameter versions and cast to a MobiusConfig.
     */
	void getObjectValue(void* obj, ExValue* value);
	void setObjectValue(void* obj, ExValue* value);

    /**
     * Overload the Parameter versions and pass a Mobius;
     */
#ifndef HIDE_EXPORT
	virtual void getValue(Export* exp, ExValue* value);
	virtual void setValue(Action* action);

    /**
     * Overload the Parameter versions and resolve a MobiusConfig.
     */
	int getOrdinalValue(Export* exp);
#endif
    
    /**
     * These must always be overloaded.
     */
	virtual void getValue(MobiusConfig* c, ExValue* value) = 0;
	virtual void setValue(MobiusConfig* c, ExValue* value) = 0;

    /**
     * This must be overloaded by anything that supports ordinals.
     */
	virtual int getOrdinalValue(MobiusConfig* c);

  private:
    bool mComplained;

};


void GlobalParameter::getObjectValue(void* obj, ExValue* value)
{
    getValue((MobiusConfig*)obj, value);
}

void GlobalParameter::setObjectValue(void* obj, ExValue* value)
{
    setValue((MobiusConfig*)obj, value);
}

#ifndef HIDE_EXPORT
void GlobalParameter::getValue(Export* exp, ExValue* value)
{
	Mobius* m = (Mobius*)exp->getMobius();
    if (m == nullptr) {
        Trace(1, "Mobius not passed in Export!\n");
		value->setNull();
    }
    else {
        // for gets use the external one
        // !! think about this, should we consistently use the interrupt 
        // config, it probably doesn't matter since only scripts
        // deal with most globals 
        MobiusConfig* config = m->getConfiguration();
        getValue(config, value);
    }
}

void GlobalParameter::setValue(Action* action)
{
	Mobius* m = (Mobius*)action->mobius;
    if (m == nullptr)
	  Trace(1, "Mobius not passed in Action!\n");
    else {
        MobiusConfig* config = m->getConfiguration();
        setValue(config, &(action->arg));

        config = m->getInterruptConfiguration();
        if (config != nullptr)
          setValue(config, &(action->arg));
    }
}

int GlobalParameter::getOrdinalValue(Export* exp)
{
    int value = -1;
	Mobius* m = (Mobius*)exp->getMobius();
    if (m == nullptr) {
        Trace(1, "Mobius not passed in Export!\n");
    }
    else {
        // for gets use the external one
        // !! think about this, should we consistently use the interrupt 
        // config, it probably doesn't matter since only scripts
        // deal with most globals 
        MobiusConfig* config = m->getConfiguration();
        value = getOrdinalValue(config);
    }
    return value;
}
#endif

/**
 * This one we can't have a default implementation for, it must be overloaded.
 */
int GlobalParameter::getOrdinalValue(MobiusConfig* c)
{
    // this soaks up so many resources, only do it once!
    if (!mComplained) {
        Trace(1, "Parameter %s: getOrdinalValue(MobiusConfig) not overloaded!\n",
              getName());
        mComplained = true;
    }

    return -1;
}

//////////////////////////////////////////////////////////////////////
//
// LogStatus
//
//////////////////////////////////////////////////////////////////////

class LogStatusParameterType : public GlobalParameter
{
  public:
	LogStatusParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

LogStatusParameterType::LogStatusParameterType() : 
    GlobalParameter("logStatus", MSG_PARAM_LOG_STATUS)
{
    // not worth bindable
	type = TYPE_BOOLEAN;
}

void LogStatusParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setBool(c->isLogStatus());
}

void LogStatusParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	c->setLogStatus(value->getBool());
}

LogStatusParameterType LogStatusParameterObj;
Parameter* LogStatusParameter = &LogStatusParameterObj;

//////////////////////////////////////////////////////////////////////
//
// SetupName
//
//////////////////////////////////////////////////////////////////////

class SetupNameParameterType : public GlobalParameter
{
  public:
	SetupNameParameterType();


	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
    int getOrdinalValue(MobiusConfig* c);

#ifndef HIDE_EXPORT
	void setValue(Action* action);
#endif

#ifndef HIDE_MOBIUS
	int getHigh(MobiusInterface* m);
    void getOrdinalLabel(MobiusInterface* m, int i, ExValue* value);
#endif    
};

SetupNameParameterType::SetupNameParameterType() :
    // this must match the TargetSetup name
    GlobalParameter("setup", MSG_PARAM_SETUP)
{
	type = TYPE_STRING;
    bindable = true;
	dynamic = true;
}

int SetupNameParameterType::getOrdinalValue(MobiusConfig* c)
{
    Setup* setup = c->getCurrentSetup();
    return setup->getNumber();
}

void SetupNameParameterType::getValue(MobiusConfig* c, ExValue* value)
{
    Setup* setup = c->getCurrentSetup();
	value->setString(setup->getName());
}

/**
 * For scripts accept a name or a number.
 * Number is 1 based like SetupNumberParameter.
 */
void SetupNameParameterType::setValue(MobiusConfig* c, ExValue* value)
{
    Setup* setup = nullptr;

    if (value->getType() == EX_INT)
      setup = c->getSetup(value->getInt());
    else 
      setup = c->getSetup(value->getString());

    if (setup != nullptr)
      c->setCurrentSetup(setup);
}

/**
 * For bindings, we not only update the config object, we also propagate
 * the change through the engine.
 * For scripts accept a name or a number.
 * Number is 1 based like SetupNumberParameter.
 *
 * This is one of the rare overloads to get the Action so we
 * can check the trigger.
 */
#ifndef HIDE_EXPORT
void SetupNameParameterType::setValue(Action* action)
{
	Mobius* m = (Mobius*)action->mobius;
	if (m == nullptr)
	  Trace(1, "Mobius not passed in Action!\n");
    else {
        MobiusConfig* config = m->getConfiguration();

        Setup* setup = nullptr;
        if (action->arg.getType() == EX_INT)
          setup = config->getSetup(action->arg.getInt());
        else 
          setup = config->getSetup(action->arg.getString());

        if (setup != nullptr) {
            // Set the external one so that if you open the setup
            // window you'll see the one we're actually using selected.
            // in theory we could be cloning this config at the same time
            // while opening the setup window but worse case it just gets
            // the wrong selection.
            config->setCurrentSetup(setup);

            // then set the one we're actually using internally
            // we're always inside the interrupt at this point
            m->setSetupInternal(setup->getNumber());
        }
    }
}
#endif

/**
 * !! The max can change as setups are added/removed.
 * Need to work out a way to convey that to ParameterEditor.
 */
#ifndef HIDE_MOBIUS
int SetupNameParameterType::getHigh(MobiusInterface* m)
{
	MobiusConfig* config = m->getConfiguration();
    int max = config->getSetupCount();
    // this is the number of configs, the max ordinal is zero based
    max--;

    return max;
}

/**
 * Given an ordinal, map it into a display label.
 * Since we set the interrupt setup, that
 */
void SetupNameParameterType::getOrdinalLabel(MobiusInterface* mobius,
                                                    int i, ExValue* value)
{
    // use the interrupt config since that's the one we're really using
    Mobius* m = (Mobius*)mobius;
	MobiusConfig* config = m->getInterruptConfiguration();
	Setup* setup = config->getSetup(i);
	if (setup != nullptr)
	  value->setString(setup->getName());
	else
      value->setString("???");
}
#endif

SetupNameParameterType SetupNameParameterObj;
Parameter* SetupNameParameter = &SetupNameParameterObj;

//////////////////////////////////////////////////////////////////////
//
// SetupNumber
//
//////////////////////////////////////////////////////////////////////

/**
 * Provided so scripts can deal with setups as numbers if necessary
 * though I would expect usually they will be referenced using names.
 * 
 * !! NOTE: For consistency with TrackPresetNumber the setup numbers
 * are the zero based intenral numbers.  This is unlike how
 * tracks and loops are numbered from 1.
 */
class SetupNumberParameterType : public GlobalParameter
{
  public:
	SetupNumberParameterType();

	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
#ifndef HIDE_EXPORT
	void setValue(Action* action);
#endif    
};

SetupNumberParameterType SetupNumberParameterObj;
Parameter* SetupNumberParameter = &SetupNumberParameterObj;

SetupNumberParameterType::SetupNumberParameterType() :
    GlobalParameter("setupNumber", MSG_PARAM_SETUP_NUMBER)
{
	type = TYPE_INT;
    // not displayed in the UI, don't include it in the XML
    transient = true;
    // dynmic means it can change after the UI is initialized
    // I don't think we need to say this if it isn't bindable
	dynamic = true;
}

void SetupNumberParameterType::getValue(MobiusConfig* c, ExValue* value)
{
    Setup* setup = c->getCurrentSetup();
    value->setInt(setup->getNumber());
}

/**
 * This is a fake parameter so we won't edit them in the config.
 */
void SetupNumberParameterType::setValue(MobiusConfig* c, ExValue* value)
{
}

#ifndef HIDE_EXPORT
void SetupNumberParameterType::setValue(Action* action)
{
    Mobius* m = action->mobius;
    // validate using the external config
    MobiusConfig* config = m->getConfiguration();
    int index = action->arg.getInt();
    Setup* setup = config->getSetup(index);

    if (setup != nullptr) {
        // we're always in the interrupt so can set it now
        m->setSetupInternal(index);
    }
}
#endif

//////////////////////////////////////////////////////////////////////
//
// Track
//
// !! Not sure I like this.  We already have the track select
// functions but those have TrackCopy semantics so maybe it makes
// sense to have this too (which doesn't).  This also gives us a way
// to switch tracks more easilly through the plugin interface.
//
//////////////////////////////////////////////////////////////////////

class TrackParameterType : public GlobalParameter
{
  public:
	TrackParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
#ifndef HIDE_EXPORT
	void getValue(Export* exp, ExValue* value);
    int getOrdinalValue(Export* exp);
	void setValue(Action* action);
#endif
};

TrackParameterType::TrackParameterType() :
	// changed from "track" to "selectedTrack" to avoid ambiguity
	// with the read-only variable
    GlobalParameter("selectedTrack", MSG_PARAM_TRACK)
{
	type = TYPE_INT;
	low = 1;
	high = 16;
    // not in XML
    transient = true;
    // but a good one for CC bindings
    bindable = true;
}

void TrackParameterType::getValue(MobiusConfig* c, ExValue* value)
{
    // transient, shouldn't be here, 
    // !! the selected track from the Setup could be the same as this
    // think!
    Trace(1, "TrackParameterType::getValue!\n");
}

void TrackParameterType::setValue(MobiusConfig* c, ExValue* value)
{
    // transient, shouldn't be here, 
    Trace(1, "TrackParameterType::setValue!\n");
}

#ifndef HIDE_EXPORT
void TrackParameterType::getValue(Export* exp, ExValue* value)
{
	// let this be 1 based in the script
    Mobius* m = (Mobius*)exp->getMobius();
	Track* t = m->getTrack(m->getActiveTrack());
	if (t != nullptr)
	  value->setInt(t->getDisplayNumber());
	else {
		// assume zero
		value->setInt(1);
	}
}

void TrackParameterType::setValue(Action* action)
{
    Mobius* m = (Mobius*)action->mobius;
	// let this be 1 based in the script
	int ivalue = action->arg.getInt() - 1;
	m->setTrack(ivalue);
}

/**
 * We'll be here since we're bindable and each interrupt
 * may have an Export that will try to export our ordinal value.
 */
int TrackParameterType::getOrdinalValue(Export *exp)
{
    ExValue v;
    getValue(exp, &v);
    return v.getInt();
}
#endif

TrackParameterType TrackParameterObj;
Parameter* TrackParameter = &TrackParameterObj;

//////////////////////////////////////////////////////////////////////
//
// Bindings
//
//////////////////////////////////////////////////////////////////////

/**
 * This is a funny one because ordinal value 0 means "no overlay"
 * and we want to show that and treat it as a valid value.
 */
class BindingsParameterType : public GlobalParameter
{
  public:
	BindingsParameterType();

    int getOrdinalValue(MobiusConfig* c);
#ifndef HIDE_MOBIUS
	int getHigh(MobiusInterface* m);
    void getOrdinalLabel(MobiusInterface* m, int i, ExValue* value);
#endif
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);

#ifndef HIDE_EXPORT
	void setValue(Action* action);
#endif    
};

BindingsParameterType::BindingsParameterType() :
    // formerly "midiConfig" but don't bother with an alias
    // this must match the TargetBindings name
    GlobalParameter("bindings", MSG_PARAM_BINDINGS)
{
	type = TYPE_STRING;
    bindable = true;
    dynamic = true;
}

int BindingsParameterType::getOrdinalValue(MobiusConfig* c)
{
    int value = 0;
	BindingConfig* bindings = c->getOverlayBindingConfig();
    if (bindings != nullptr) 
	  value = bindings->getNumber();
    return value;
}

/**
 * This will return null to mean "no overlay".
 */
void BindingsParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	BindingConfig* bindings = c->getOverlayBindingConfig();
    if (bindings != nullptr) 
	  value->setString(bindings->getName());
	else
      value->setNull();
}

void BindingsParameterType::setValue(MobiusConfig* c, ExValue* value)
{
    if (value->getType() == EX_INT) {
        // assume it's an int, these are numbered from zero but zero 
        // is always the base binding
        int index = value->getInt();
        c->setOverlayBindingConfig(index);
    }
    else {
        c->setOverlayBindingConfig(value->getString());
    }
}

/**
 * Note that we call the setters on Mobius so it will also
 * update the configuration cache.
 *
 * This is one of the rare overloads to get to the Action
 * so we can have side effects on Mobius.
 */
#ifndef HIDE_EXPORT
void BindingsParameterType::setValue(Action* action)
{
    Mobius* m = (Mobius*)action->mobius;
	MobiusConfig* config = m->getConfiguration();

    if (action->arg.isNull()) {
        // clear the overlay
        m->setOverlayBindings((BindingConfig*)nullptr);
    }
    else if (action->arg.getType() == EX_STRING) {
        BindingConfig* b = config->getBindingConfig(action->arg.getString());
        // may be null to clear the overlay
        m->setOverlayBindings(b);
    }
    else {
        // assume it's an int, these are numbered from zero but zero 
        // is always the base binding
        BindingConfig* b = config->getBindingConfig(action->arg.getInt());
        m->setOverlayBindings(b);
    }
}
#endif

/**
 * !! The max can change as bindings are added/removed.
 * Need to work out a way to convey that to ParameterEditor.
 */
#ifndef HIDE_MOBIUS
int BindingsParameterType::getHigh(MobiusInterface* m)
{
    int max = 0;

	MobiusConfig* config = m->getConfiguration();
    max = config->getBindingConfigCount();
    // this is the number of configs, the max ordinal is zero based
    max--;

    return max;
}

/**
 * Given an ordinal, map it into a display label.
 */
void BindingsParameterType::getOrdinalLabel(MobiusInterface* m,
                                                   int i, ExValue* value)
{
	MobiusConfig* config = m->getConfiguration();
	BindingConfig* bindings = config->getBindingConfig(i);
    if (i == 0) {
        // This will be "Common Bindings" but we want to display
        // this as "No Overlay"
        value->setString("No Overlay");
    }
    else if (bindings != nullptr)
	  value->setString(bindings->getName());
	else
	  value->setString("???");
}
#endif

BindingsParameterType BindingsParameterObj;
Parameter* BindingsParameter = &BindingsParameterObj;

//////////////////////////////////////////////////////////////////////
//
// FadeFrames
//
//////////////////////////////////////////////////////////////////////

class FadeFramesParameterType : public GlobalParameter
{
  public:
	FadeFramesParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
#ifndef HIDE_EXPORT
	void setValue(Action* action);
#endif
};

FadeFramesParameterType::FadeFramesParameterType() :
    GlobalParameter("fadeFrames", MSG_PARAM_FADE_FRAMES)
{
    // not bindable
	type = TYPE_INT;
	high = 1024;
}

void FadeFramesParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setInt(c->getFadeFrames());
}
void FadeFramesParameterType::setValue(MobiusConfig* c, ExValue* value)
{
    c->setFadeFrames(value->getInt());
}

/**
 * Binding this is rare but we do set it in test scripts.
 * For this to have any meaning we have to propagate it to the
 * AudioFade class.  
 */
#ifndef HIDE_EXPORT
void FadeFramesParameterType::setValue(Action* action)
{
    int frames = action->arg.getInt();

    // don't bother propagating to the interrupt config, we only
    // need it in AudioFade
	MobiusConfig* config = action->mobius->getConfiguration();
	config->setFadeFrames(frames);

    AudioFade::setRange(frames);
}
#endif

FadeFramesParameterType FadeFramesParameterObj;
Parameter* FadeFramesParameter = &FadeFramesParameterObj;

//////////////////////////////////////////////////////////////////////
//
// MaxSyncDrift
//
//////////////////////////////////////////////////////////////////////

class MaxSyncDriftParameterType : public GlobalParameter
{
  public:
	MaxSyncDriftParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
#ifndef HIDE_EXPORT
	void setValue(Action* action);
#endif
};

MaxSyncDriftParameterType::MaxSyncDriftParameterType() :
    GlobalParameter("maxSyncDrift", MSG_PARAM_SYNC_DRIFT)
{
    // not worth bindable
	type = TYPE_INT;
	high = 1024 * 16;
    // The low end is dependent on the sync source, for
    // Host sync you could set this to zero and get good results,   
    // for MIDI sync the effective minimum has to be around 512 due
    // to jitter.  Unfortunately we can't know that context here so
    // leave the low at zero.
}

void MaxSyncDriftParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setInt(c->getMaxSyncDrift());
}
void MaxSyncDriftParameterType::setValue(MobiusConfig* c, ExValue* value)
{
    c->setMaxSyncDrift(value->getInt());
}

/**
 * Binding this is rare but we occasionally set this in test scripts.
 * For this to have any meaning, we also need to propagate it to the
 * Synchronizer which keeps a cached copy.  Also copy it to the interrupt
 * config just so they stay in sync though that isn't used.
 */
#ifndef HIDE_EXPORT
void MaxSyncDriftParameterType::setValue(Action* action)
{
    int drift = action->arg.getInt();

    Mobius* m = (Mobius*)action->mobius;
	MobiusConfig* config = m->getConfiguration();
	config->setMaxSyncDrift(drift);

    MobiusConfig* iconfig = m->getInterruptConfiguration();
    if (iconfig != nullptr) {
        iconfig->setMaxSyncDrift(drift);
        Synchronizer* sync = m->getSynchronizer();
        sync->updateConfiguration(iconfig);
    }
}
#endif

MaxSyncDriftParameterType MaxSyncDriftParameterObj;
Parameter* MaxSyncDriftParameter = &MaxSyncDriftParameterObj;

//////////////////////////////////////////////////////////////////////
//
// DriftCheckPoint
//
//////////////////////////////////////////////////////////////////////

/**
 * This is not currently exposed int he UI, though it can be set in scripts.
 */
class DriftCheckPointParameterType : public GlobalParameter
{
  public:
	DriftCheckPointParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
#ifndef HIDE_EXPORT
	void setValue(Action* action);
#endif
    
};

const char* DRIFT_CHECK_POINT_NAMES[] = {
	"loop", "external", nullptr
};

DriftCheckPointParameterType::DriftCheckPointParameterType() :
    GlobalParameter("driftCheckPoint", 0)
{
    // don't bother making this bindable
	type = TYPE_ENUM;
	values = DRIFT_CHECK_POINT_NAMES;
}

void DriftCheckPointParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setString(values[c->getDriftCheckPoint()]);
}

void DriftCheckPointParameterType::setValue(MobiusConfig* c, ExValue* value)
{
    DriftCheckPoint dcp = (DriftCheckPoint)getEnum(value);
    c->setDriftCheckPoint(dcp);
}

/**
 * Binding this is rare but we occasionally set this in test scripts.
 * For this to have any meaning, we also need to propagate it to the
 * Synchronizer which keeps a cached copy.  Also copy it to the interrupt
 * config just so they stay in sync though that isn't used.
 */
#ifndef HIDE_EXPORT
void DriftCheckPointParameterType::setValue(Action* action)
{
    DriftCheckPoint dcp = (DriftCheckPoint)getEnum(&(action->arg));

    Mobius* m = (Mobius*)action->mobius;
	MobiusConfig* config = m->getConfiguration();
	config->setDriftCheckPoint(dcp);

    MobiusConfig* iconfig = m->getInterruptConfiguration();
    if (iconfig != nullptr) {
        iconfig->setDriftCheckPoint(dcp);
        Synchronizer* sync = m->getSynchronizer();
        sync->updateConfiguration(iconfig);
    }
}
#endif

DriftCheckPointParameterType DriftCheckPointParameterObj;
Parameter* DriftCheckPointParameter = &DriftCheckPointParameterObj;

//////////////////////////////////////////////////////////////////////
//
// NoiseFloor
//
//////////////////////////////////////////////////////////////////////

class NoiseFloorParameterType : public GlobalParameter
{
  public:
	NoiseFloorParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

NoiseFloorParameterType::NoiseFloorParameterType() :
    GlobalParameter("noiseFloor", MSG_PARAM_NOISE_FLOOR)
{
    // not bindable
	type = TYPE_INT;
    // where the hell did this value come from?
	high = 15359;
}

void NoiseFloorParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setInt(c->getNoiseFloor());
}

void NoiseFloorParameterType::setValue(MobiusConfig* c, ExValue* value)
{
    c->setNoiseFloor(value->getInt());
}

NoiseFloorParameterType NoiseFloorParameterObj;
Parameter* NoiseFloorParameter = &NoiseFloorParameterObj;

//////////////////////////////////////////////////////////////////////
//
// PluginPorts
//
//////////////////////////////////////////////////////////////////////

class PluginPortsParameterType : public GlobalParameter
{
  public:
	PluginPortsParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

PluginPortsParameterType::PluginPortsParameterType() :
    GlobalParameter("pluginPorts", MSG_PARAM_PLUGIN_PORTS)
{
    // not worth bindable
	type = TYPE_INT;
    low = 1;
	high = 8;
}

void PluginPortsParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setInt(c->getPluginPorts());
}

void PluginPortsParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	c->setPluginPorts(value->getInt());
}

PluginPortsParameterType PluginPortsParameterObj;
Parameter* PluginPortsParameter = &PluginPortsParameterObj;

//////////////////////////////////////////////////////////////////////
//
// MidiExport, HostMidiExport
//
//////////////////////////////////////////////////////////////////////

class MidiExportParameterType : public GlobalParameter
{
  public:
	MidiExportParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

MidiExportParameterType::MidiExportParameterType() : 
    GlobalParameter("midiExport", MSG_PARAM_MIDI_EXPORT)
{
    // not worth bindable
	type = TYPE_BOOLEAN;
    // original name
    addAlias("midiFeedback");
}

void MidiExportParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setBool(c->isMidiExport());
}

void MidiExportParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	c->setMidiExport(value->getBool());
}

MidiExportParameterType MidiExportParameterObj;
Parameter* MidiExportParameter = &MidiExportParameterObj;

/////////////////////////////////////////

class HostMidiExportParameterType : public GlobalParameter
{
  public:
	HostMidiExportParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

HostMidiExportParameterType::HostMidiExportParameterType() :
    GlobalParameter("hostMidiExport", MSG_PARAM_HOST_MIDI_EXPORT)
{
    // not worth bindable
	type = TYPE_BOOLEAN;
    addAlias("hostMidiFeedback");
}

void HostMidiExportParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setBool(c->isHostMidiExport());
}

void HostMidiExportParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	c->setHostMidiExport(value->getBool());
}

HostMidiExportParameterType HostMidiExportParameterObj;
Parameter* HostMidiExportParameter = &HostMidiExportParameterObj;

//////////////////////////////////////////////////////////////////////
//
// LongPress
//
//////////////////////////////////////////////////////////////////////

class LongPressParameterType : public GlobalParameter
{
  public:
	LongPressParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

LongPressParameterType::LongPressParameterType() :
    GlobalParameter("longPress", MSG_PARAM_LONG_PRESS)
{
    // not bindable
	type = TYPE_INT;
	low = 250;
	high = 10000;
}

void LongPressParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setInt(c->getLongPress());
}

void LongPressParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	c->setLongPress(value->getInt());
}

LongPressParameterType LongPressParameterObj;
Parameter* LongPressParameter = &LongPressParameterObj;

//////////////////////////////////////////////////////////////////////
//
// SpreadRange
//
//////////////////////////////////////////////////////////////////////

class SpreadRangeParameterType : public GlobalParameter
{
  public:
	SpreadRangeParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

SpreadRangeParameterType::SpreadRangeParameterType() :
    GlobalParameter("spreadRange", MSG_PARAM_SPREAD_RANGE)
{
    // not worth bindable
	type = TYPE_INT;
	low = 1;
	high = 128;
    addAlias("shiftRange");
}

void SpreadRangeParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setInt(c->getSpreadRange());
}

void SpreadRangeParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	c->setSpreadRange(value->getInt());
}

SpreadRangeParameterType SpreadRangeParameterObj;
Parameter* SpreadRangeParameter = &SpreadRangeParameterObj;

//////////////////////////////////////////////////////////////////////
//
// TraceDebugLevel
//
//////////////////////////////////////////////////////////////////////

class TraceDebugLevelParameterType : public GlobalParameter
{
  public:
	TraceDebugLevelParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
#ifndef HIDE_EXPORT
	void setValue(Action* action);
#endif
};

TraceDebugLevelParameterType::TraceDebugLevelParameterType() :
    GlobalParameter("traceDebugLevel", MSG_PARAM_TRACE_DEBUG_LEVEL)
{
    // not worth bindable
	type = TYPE_INT;
	high = 4;
}

void TraceDebugLevelParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setInt(c->getTraceDebugLevel());
}
void TraceDebugLevelParameterType::setValue(MobiusConfig* c, ExValue* value)
{
    c->setTraceDebugLevel(value->getInt());
}

/**
 * Binding is rare but we can set this in a test script.
 * For this to have meaning we need to propagate to the Trace 
 * global variables.
 */
#ifndef HIDE_EXPORT
void TraceDebugLevelParameterType::setValue(Action* action)
{
    int level = action->arg.getInt();

	MobiusConfig* config = action->mobius->getConfiguration();
	config->setTraceDebugLevel(level);

    TraceDebugLevel = level;
}
#endif

TraceDebugLevelParameterType TraceDebugLevelParameterObj;
Parameter* TraceDebugLevelParameter = &TraceDebugLevelParameterObj;

//////////////////////////////////////////////////////////////////////
//
// TracePrintLevel
//
//////////////////////////////////////////////////////////////////////

class TracePrintLevelParameterType : public GlobalParameter
{
  public:
	TracePrintLevelParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
#ifndef HIDE_EXPORT
	void setValue(Action* action);
#endif
};

TracePrintLevelParameterType::TracePrintLevelParameterType() :
    GlobalParameter("tracePrintLevel", MSG_PARAM_TRACE_PRINT_LEVEL)
{
    // not worth bindable
	type = TYPE_INT;
	high = 4;
}

void TracePrintLevelParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setInt(c->getTracePrintLevel());
}
void TracePrintLevelParameterType::setValue(MobiusConfig* c, ExValue* value)
{
    c->setTracePrintLevel(value->getInt());
}

/**
 * Binding is rare but we can set this in a test script.
 * For this to have meaning we need to propagate to the Trace 
 * global variables.
 */
#ifndef HIDE_EXPORT
void TracePrintLevelParameterType::setValue(Action* action)
{
    int level = action->arg.getInt();

	MobiusConfig* config = action->mobius->getConfiguration();
	config->setTracePrintLevel(level);

    TracePrintLevel = level;
}
#endif

TracePrintLevelParameterType TracePrintLevelParameterObj;
Parameter* TracePrintLevelParameter = &TracePrintLevelParameterObj;

//////////////////////////////////////////////////////////////////////
//
// CustomMode
//
//////////////////////////////////////////////////////////////////////

class CustomModeParameterType : public GlobalParameter
{
  public:
	CustomModeParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
#ifndef HIDE_EXPORT
	void getValue(Export* exp, ExValue* value);
	void setValue(Action* action);
#endif    
};

CustomModeParameterType::CustomModeParameterType() :
    GlobalParameter("customMode", MSG_PARAM_CUSTOM_MODE)
{
    // not bindable
	type = TYPE_STRING;
    // sould this be in the Setup?
    transient = true;
}

void CustomModeParameterType::getValue(MobiusConfig* c, ExValue* value)
{
    Trace(1, "CustomModeParameterType::getValue!\n");
}

void CustomModeParameterType::setValue(MobiusConfig* c, ExValue* value)
{
    Trace(1, "CustomModeParameterType::setValue!\n");
}

#ifndef HIDE_EXPORT
void CustomModeParameterType::getValue(Export* exp, ExValue* value)
{
    Mobius* m = (Mobius*)exp->getMobius();
	value->setString(m->getCustomMode());
}

void CustomModeParameterType::setValue(Action* action)
{
    Mobius* m = (Mobius*)action->mobius;
	m->setCustomMode(action->arg.getString());
}
#endif

CustomModeParameterType CustomModeParameterObj;
Parameter* CustomModeParameter = &CustomModeParameterObj;

//////////////////////////////////////////////////////////////////////
//
// AutoFeedbackReduction
//
//////////////////////////////////////////////////////////////////////

class AutoFeedbackReductionParameterType : public GlobalParameter
{
  public:
    AutoFeedbackReductionParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
#ifndef HIDE_EXPORT
    void setValue(Action* action);
#endif
};

AutoFeedbackReductionParameterType::AutoFeedbackReductionParameterType() :
    GlobalParameter("autoFeedbackReduction", MSG_PARAM_AUTO_FEEDBACK_REDUCTION)
{
    // not worth bindable
    type = TYPE_BOOLEAN;
}

void AutoFeedbackReductionParameterType::getValue(MobiusConfig* c, 
                                                  ExValue* value)
{
	value->setBool(c->isAutoFeedbackReduction());
}
void AutoFeedbackReductionParameterType::setValue(MobiusConfig* c, 
                                                  ExValue* value)
{
    c->setAutoFeedbackReduction(value->getBool());
}

/**
 * Binding this is rare but we do set this in test scripts.
 * For this to have any meaning we have to propagate this to the
 * Loops via the Tracks.
 */
#ifndef HIDE_EXPORT
void AutoFeedbackReductionParameterType::setValue(Action* action)
{
    bool afr = action->arg.getBool();

    Mobius* m = (Mobius*)action->mobius;
    MobiusConfig* config = m->getConfiguration();
    config->setAutoFeedbackReduction(afr);

    MobiusConfig* iconfig = m->getInterruptConfiguration();
    if (iconfig != nullptr) {
        iconfig->setAutoFeedbackReduction(afr);

        for (int i = 0 ; i < m->getTrackCount() ; i++) {
            Track* t = m->getTrack(i);
            t->updateGlobalParameters(iconfig);
        }
    }
}
#endif

AutoFeedbackReductionParameterType AutoFeedbackReductionParameterObj;
Parameter* AutoFeedbackReductionParameter = &AutoFeedbackReductionParameterObj;

//////////////////////////////////////////////////////////////////////
//
// IsolateOverdubs
//
//////////////////////////////////////////////////////////////////////

class IsolateOverdubsParameterType : public GlobalParameter
{
  public:
	IsolateOverdubsParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

IsolateOverdubsParameterType::IsolateOverdubsParameterType() :
    GlobalParameter("isolateOverdubs", MSG_PARAM_ISOLATE_OVERDUBS)
{
    // not worth bindable
	type = TYPE_BOOLEAN;
}

void IsolateOverdubsParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setBool(c->isIsolateOverdubs());
}

void IsolateOverdubsParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	c->setIsolateOverdubs(value->getBool());
}

IsolateOverdubsParameterType IsolateOverdubsParameterObj;
Parameter* IsolateOverdubsParameter = &IsolateOverdubsParameterObj;

//////////////////////////////////////////////////////////////////////
//
// MonitorAudio
//
//////////////////////////////////////////////////////////////////////

class MonitorAudioParameterType : public GlobalParameter
{
  public:
	MonitorAudioParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
#ifndef HIDE_EXPORT
    void setValue(Action* action);
#endif
};

MonitorAudioParameterType::MonitorAudioParameterType() :
    GlobalParameter("monitorAudio", MSG_PARAM_MONITOR_AUDIO)
{
    // not worth bindable
	type = TYPE_BOOLEAN;
}

void MonitorAudioParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setBool(c->isMonitorAudio());
}
void MonitorAudioParameterType::setValue(MobiusConfig* c, ExValue* value)
{
    c->setMonitorAudio(value->getBool());
}

/**
 * Binding this is rare, but we do set it in test scripts.
 * For this to have any meaning we need to propagate it to the
 * interrupt config where Track will look at it, and also 
 * to the Recorder.
 */
#ifndef HIDE_EXPORT
void MonitorAudioParameterType::setValue(Action* action)
{
    bool monitor = action->arg.getBool();

    Mobius* m = (Mobius*)action->mobius;
    MobiusConfig* config = m->getConfiguration();
	config->setMonitorAudio(monitor);

    MobiusConfig* iconfig = m->getInterruptConfiguration();
    if (iconfig != nullptr)
      iconfig->setMonitorAudio(monitor);

    Recorder* rec = m->getRecorder();
    if (rec != nullptr)
      rec->setEcho(monitor);
}
#endif

MonitorAudioParameterType MonitorAudioParameterObj;
Parameter* MonitorAudioParameter = &MonitorAudioParameterObj;

//////////////////////////////////////////////////////////////////////
//
// SaveLayers
//
//////////////////////////////////////////////////////////////////////

class SaveLayersParameterType : public GlobalParameter
{
  public:
	SaveLayersParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

SaveLayersParameterType::SaveLayersParameterType() :
    GlobalParameter("saveLayers", MSG_PARAM_SAVE_LAYERS)
{
    // not worth bindable
	type = TYPE_BOOLEAN;
}

void SaveLayersParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setBool(c->isSaveLayers());
}

void SaveLayersParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	c->setSaveLayers(value->getBool());
}

SaveLayersParameterType SaveLayersParameterObj;
Parameter* SaveLayersParameter = &SaveLayersParameterObj;

//////////////////////////////////////////////////////////////////////
//
// QuickSave
//
//////////////////////////////////////////////////////////////////////

class QuickSaveParameterType : public GlobalParameter
{
  public:
	QuickSaveParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

QuickSaveParameterType::QuickSaveParameterType() :
    GlobalParameter("quickSave", MSG_PARAM_QUICK_SAVE)
{
    // not bindable
	type = TYPE_STRING;
}

void QuickSaveParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setString(c->getQuickSave());
}

void QuickSaveParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	c->setQuickSave(value->getString());
}

QuickSaveParameterType QuickSaveParameterObj;
Parameter* QuickSaveParameter = &QuickSaveParameterObj;

//////////////////////////////////////////////////////////////////////
//
// UnitTests
//
//////////////////////////////////////////////////////////////////////

class UnitTestsParameterType : public GlobalParameter
{
  public:
	UnitTestsParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

UnitTestsParameterType::UnitTestsParameterType() :
    GlobalParameter("unitTests", MSG_PARAM_UNIT_TESTS)
{
    // not bindable
	type = TYPE_STRING;
}

void UnitTestsParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setString(c->getUnitTests());
}

void UnitTestsParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	c->setUnitTests(value->getString());
}

UnitTestsParameterType UnitTestsParameterObj;
Parameter* UnitTestsParameter = &UnitTestsParameterObj;

//////////////////////////////////////////////////////////////////////
//
// IntegerWaveFile
//
//////////////////////////////////////////////////////////////////////

class IntegerWaveFileParameterType : public GlobalParameter
{
  public:
	IntegerWaveFileParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
#ifndef HIDE_EXPORT
	void setValue(Action* action);
#endif
};

IntegerWaveFileParameterType::IntegerWaveFileParameterType() :
    GlobalParameter("16BitWaveFile", MSG_PARAM_INTEGER_WAVE_FILE)
{
    // not worth bindable
	type = TYPE_BOOLEAN;
}

void IntegerWaveFileParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setBool(c->isIntegerWaveFile());
}
void IntegerWaveFileParameterType::setValue(MobiusConfig* c, ExValue* value)
{
    c->setIntegerWaveFile(value->getBool());
}

/**
 * Binding this is rare but we do set it in test scripts.
 * For this to have any meaning we have to propagate it to the
 * Audio class.  
 */
#ifndef HIDE_EXPORT
void IntegerWaveFileParameterType::setValue(Action* action)
{
    bool isInt = action->arg.getBool();
    // don't bother propagating this to the interrupt config
    Mobius* m = (Mobius*)action->mobius;
    MobiusConfig* config = m->getConfiguration();
	config->setIntegerWaveFile(isInt);

    Audio::setWriteFormatPCM(isInt);
}
#endif

IntegerWaveFileParameterType IntegerWaveFileParameterObj;
Parameter* IntegerWaveFileParameter = &IntegerWaveFileParameterObj;

//////////////////////////////////////////////////////////////////////
//
// AltFeedbackDisable
//
//////////////////////////////////////////////////////////////////////

class AltFeedbackDisableParameterType : public GlobalParameter
{
  public:
	AltFeedbackDisableParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
#ifndef HIDE_MOBIUS
	void getValue(Mobius* m, ExValue* value);
	void setValue(Mobius* m, ExValue* value);
#endif
};

AltFeedbackDisableParameterType::AltFeedbackDisableParameterType() :
    GlobalParameter("altFeedbackDisable", MSG_PARAM_ALT_FEEDBACK_DISABLE)
{
    // not bindable
	type = TYPE_STRING;
}

void AltFeedbackDisableParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	StringList* l = c->getAltFeedbackDisables();
	if (l == nullptr)
	  value->setString(nullptr);
	else {
		char* str = l->toCsv();
		value->setString(str);
		delete str;
	}
}

void AltFeedbackDisableParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	if (value->isNull())
	  c->setAltFeedbackDisables(nullptr);
	else
	  c->setAltFeedbackDisables(new StringList(value->getString()));
}

AltFeedbackDisableParameterType AltFeedbackDisableParameterObj;
Parameter* AltFeedbackDisableParameter = &AltFeedbackDisableParameterObj;

//////////////////////////////////////////////////////////////////////
//
// GroupFocusLock
//
//////////////////////////////////////////////////////////////////////

class GroupFocusLockParameterType : public GlobalParameter
{
  public:
	GroupFocusLockParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

GroupFocusLockParameterType::GroupFocusLockParameterType() :
    GlobalParameter("groupFocusLock", MSG_PARAM_GROUP_FOCUS_LOCK)
{
    // not worth bindable?
	type = TYPE_BOOLEAN;
}

void GroupFocusLockParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setBool(c->isGroupFocusLock());
}

void GroupFocusLockParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	c->setGroupFocusLock(value->getBool());
}

GroupFocusLockParameterType GroupFocusLockParameterObj;
Parameter* GroupFocusLockParameter = &GroupFocusLockParameterObj;

//////////////////////////////////////////////////////////////////////
//
// FocusLockFunctions
//
//////////////////////////////////////////////////////////////////////

class FocusLockFunctionsParameterType : public GlobalParameter
{
  public:
	FocusLockFunctionsParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

FocusLockFunctionsParameterType::FocusLockFunctionsParameterType() :
    GlobalParameter("focusLockFunctions", MSG_PARAM_FOCUS_LOCK_FUNCTIONS)
{
    // not bindable
	type = TYPE_STRING;
    // the old name
    addAlias("groupFunctions");
}

void FocusLockFunctionsParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	StringList* l = c->getFocusLockFunctions();
	if (l == nullptr)
	  value->setString(nullptr);
	else {
		char* str = l->toCsv();
		value->setString(str);
		delete str;
	}
}

void FocusLockFunctionsParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	if (value->isNull())
	  c->setFocusLockFunctions(nullptr);
	else
	  c->setFocusLockFunctions(new StringList(value->getString()));
}

FocusLockFunctionsParameterType FocusLockFunctionsParameterObj;
Parameter* FocusLockFunctionsParameter = &FocusLockFunctionsParameterObj;

//////////////////////////////////////////////////////////////////////
//
// MuteCancelFunctions
//
//////////////////////////////////////////////////////////////////////

class MuteCancelFunctionsParameterType : public GlobalParameter
{
  public:
	MuteCancelFunctionsParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
#ifndef HIDE_EXPORT
	void setValue(Action* action);
#endif
};

MuteCancelFunctionsParameterType::MuteCancelFunctionsParameterType() :
    GlobalParameter("muteCancelFunctions", MSG_PARAM_MUTE_CANCEL_FUNCTIONS)
{
    // not bindable
	type = TYPE_STRING;
}

void MuteCancelFunctionsParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	StringList* l = c->getMuteCancelFunctions();
	if (l == nullptr)
	  value->setString(nullptr);
	else {
		char* str = l->toCsv();
		value->setString(str);
		delete str;
	}
}

void MuteCancelFunctionsParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	if (value->isNull())
	  c->setMuteCancelFunctions(nullptr);
	else
	  c->setMuteCancelFunctions(new StringList(value->getString()));
}

/**
 * Binding this is impossible but we might set it in a test script.
 * For this to have any meaning we have to propagate it to the
 * Funtion class.
 */
#ifndef HIDE_EXPORT
void MuteCancelFunctionsParameterType::setValue(Action* action)
{
    // don't bother propagating to the interrupt
    Mobius* m = (Mobius*)action->mobius;
	MobiusConfig* config = m->getConfiguration();
	if (action->arg.isNull())
	  config->setMuteCancelFunctions(nullptr);
	else
	  config->setMuteCancelFunctions(new StringList(action->arg.getString()));

    // this is normally called by installConfig when the
    // scripts are compiled, here we track dynamic changes
    // sigh, not in MobiusInterface
    m->updateGlobalFunctionPreferences();
}
#endif

MuteCancelFunctionsParameterType MuteCancelFunctionsParameterObj;
Parameter* MuteCancelFunctionsParameter = &MuteCancelFunctionsParameterObj;

//////////////////////////////////////////////////////////////////////
//
// ConfirmationFunctions
//
//////////////////////////////////////////////////////////////////////

class ConfirmationFunctionsParameterType : public GlobalParameter
{
  public:
	ConfirmationFunctionsParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
#ifndef HIDE_EXPORT
	void setValue(Action* action);
#endif
};

ConfirmationFunctionsParameterType::ConfirmationFunctionsParameterType() :
    GlobalParameter("confirmationFunctions", MSG_PARAM_CONFIRMATION_FUNCTIONS)
{
    // not bindable
	type = TYPE_STRING;
}

void ConfirmationFunctionsParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	StringList* l = c->getConfirmationFunctions();
	if (l == nullptr)
	  value->setString(nullptr);
	else {
		char* str = l->toCsv();
		value->setString(str);
		delete str;
	}
}

void ConfirmationFunctionsParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	if (value->isNull())
	  c->setConfirmationFunctions(nullptr);
	else
	  c->setConfirmationFunctions(new StringList(value->getString()));
}

/**
 * Binding this is impossible but we might set it in a test script.
 * For this to have any meaning we have to propagate it to the
 * Funtion class.
 */
#ifndef HIDE_EXPORT
void ConfirmationFunctionsParameterType::setValue(Action* action)
{
    // don't bother propagating to the interrupt
    Mobius* m = (Mobius*)action->mobius;
	MobiusConfig* config = m->getConfiguration();
	if (action->arg.isNull())
	  config->setConfirmationFunctions(nullptr);
	else
	  config->setConfirmationFunctions(new StringList(action->arg.getString()));

    // this is normally called by installConfig when the
    // scripts are compiled, here we track dynamic changes
    // sigh, not in MobiusInterface
    m->updateGlobalFunctionPreferences();
}
#endif

ConfirmationFunctionsParameterType ConfirmationFunctionsParameterObj;
Parameter* ConfirmationFunctionsParameter = &ConfirmationFunctionsParameterObj;

//////////////////////////////////////////////////////////////////////
//
// MidiRecordMode
//
//////////////////////////////////////////////////////////////////////

class MidiRecordModeParameterType : public GlobalParameter
{
  public:
	MidiRecordModeParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
#ifndef HIDE_EXPORT
	void setValue(Action* action);
#endif
};

const char* MIDI_RECORD_MODE_NAMES[] = {
	"average", "smooth", "pulse", nullptr
};

MidiRecordModeParameterType::MidiRecordModeParameterType() :
    GlobalParameter("midiRecordMode", 0)
{
    // not worth bindable
	type = TYPE_ENUM;
	values = MIDI_RECORD_MODE_NAMES;
}

void MidiRecordModeParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setString(values[c->getMidiRecordMode()]);
}

void MidiRecordModeParameterType::setValue(MobiusConfig* c, ExValue* value)
{
    MidiRecordMode mode = (MidiRecordMode)getEnum(value);
    c->setMidiRecordMode(mode);
}

/**
 * Binding this is rare but we occasionally set this in test scripts.
 * For this to have any meaning, we also need to propagate it to the
 * Synchronizer which keeps a cached copy.  Also copy it to the interrupt
 * config just so they stay in sync though that isn't used.
 */
#ifndef HIDE_EXPORT
void MidiRecordModeParameterType::setValue(Action* action)
{
    MidiRecordMode mode = (MidiRecordMode)getEnum(&(action->arg));

    Mobius* m = (Mobius*)action->mobius;
    MobiusConfig* config = m->getConfiguration();
	config->setMidiRecordMode(mode);

    MobiusConfig* iconfig = m->getInterruptConfiguration();
    if (iconfig != nullptr) {
        iconfig->setMidiRecordMode(mode);
        Synchronizer* sync = m->getSynchronizer();
        sync->updateConfiguration(iconfig);
    }
}
#endif

MidiRecordModeParameterType MidiRecordModeParameterObj;
Parameter* MidiRecordModeParameter = &MidiRecordModeParameterObj;

//////////////////////////////////////////////////////////////////////
//
// DualPluginWindow
//
//////////////////////////////////////////////////////////////////////

class DualPluginWindowParameterType : public GlobalParameter
{
  public:
	DualPluginWindowParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

DualPluginWindowParameterType::DualPluginWindowParameterType() :
    GlobalParameter("dualPluginWindow", MSG_PARAM_DUAL_PLUGIN_WINDOW)
{
    // not worth bindable
	type = TYPE_BOOLEAN;
}

void DualPluginWindowParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setBool(c->isDualPluginWindow());
}

void DualPluginWindowParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	c->setDualPluginWindow(value->getBool());
}

DualPluginWindowParameterType DualPluginWindowParameterObj;
Parameter* DualPluginWindowParameter = &DualPluginWindowParameterObj;

//////////////////////////////////////////////////////////////////////
//
// CustomMessageFile
//
//////////////////////////////////////////////////////////////////////

class CustomMessageFileParameterType : public GlobalParameter
{
  public:
	CustomMessageFileParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

CustomMessageFileParameterType::CustomMessageFileParameterType() :
    GlobalParameter("customMessageFile", MSG_PARAM_CUSTOM_MESSAGE_FILE)
{
    // not bindable
	type = TYPE_STRING;
}

void CustomMessageFileParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setString(c->getCustomMessageFile());
}

void CustomMessageFileParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	c->setCustomMessageFile(value->getString());
}

CustomMessageFileParameterType CustomMessageFileParameterObj;
Parameter* CustomMessageFileParameter = &CustomMessageFileParameterObj;

//////////////////////////////////////////////////////////////////////
//
// Tracks
//
//////////////////////////////////////////////////////////////////////

class TracksParameterType : public GlobalParameter
{
  public:
	TracksParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

TracksParameterType::TracksParameterType() :
    GlobalParameter("tracks", MSG_PARAM_TRACKS)
{
    // not bindable
	type = TYPE_INT;
    low = 1;
	high = 16;
}

void TracksParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setInt(c->getTracks());
}

void TracksParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	c->setTracks(value->getInt());
}

TracksParameterType TracksParameterObj;
Parameter* TracksParameter = &TracksParameterObj;

//////////////////////////////////////////////////////////////////////
//
// TrackGroups
//
//////////////////////////////////////////////////////////////////////

class TrackGroupsParameterType : public GlobalParameter
{
  public:
	TrackGroupsParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

TrackGroupsParameterType::TrackGroupsParameterType() :
    GlobalParameter("trackGroups", MSG_PARAM_TRACK_GROUPS)
{
    // not bindable
	type = TYPE_INT;
    high = 8;
}

void TrackGroupsParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setInt(c->getTrackGroups());
}

void TrackGroupsParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	c->setTrackGroups(value->getInt());
}

TrackGroupsParameterType TrackGroupsParameterObj;
Parameter* TrackGroupsParameter = &TrackGroupsParameterObj;

//////////////////////////////////////////////////////////////////////
//
// MaxLoops
//
//////////////////////////////////////////////////////////////////////

class MaxLoopsParameterType : public GlobalParameter
{
  public:
	MaxLoopsParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

MaxLoopsParameterType::MaxLoopsParameterType() :
    GlobalParameter("maxLoops", MSG_PARAM_MAX_LOOPS)
{
    // not bindable
	type = TYPE_INT;
    high = 16;
}

void MaxLoopsParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setInt(c->getMaxLoops());
}

void MaxLoopsParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	c->setMaxLoops(value->getInt());
}

MaxLoopsParameterType MaxLoopsParameterObj;
Parameter* MaxLoopsParameter = &MaxLoopsParameterObj;

/****************************************************************************
 *                                                                          *
 *                                    OSC                                   *
 *                                                                          *
 ****************************************************************************/

//////////////////////////////////////////////////////////////////////
//
// OscInputPort
//
//////////////////////////////////////////////////////////////////////

class OscInputPortParameterType : public GlobalParameter
{
  public:
	OscInputPortParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

OscInputPortParameterType::OscInputPortParameterType() :
    GlobalParameter("oscInputPort", MSG_PARAM_OSC_INPUT_PORT)
{
    // not bindable
	type = TYPE_INT;
}

void OscInputPortParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setInt(c->getOscInputPort());
}

void OscInputPortParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	c->setOscInputPort(value->getInt());
}

OscInputPortParameterType OscInputPortParameterObj;
Parameter* OscInputPortParameter = &OscInputPortParameterObj;

//////////////////////////////////////////////////////////////////////
//
// OscOutputPort
//
//////////////////////////////////////////////////////////////////////

class OscOutputPortParameterType : public GlobalParameter
{
  public:
	OscOutputPortParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

OscOutputPortParameterType::OscOutputPortParameterType() :
    GlobalParameter("oscOutputPort", MSG_PARAM_OSC_OUTPUT_PORT)
{
    // not bindable
	type = TYPE_INT;
}

void OscOutputPortParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setInt(c->getOscOutputPort());
}

void OscOutputPortParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	c->setOscOutputPort(value->getInt());
}

OscOutputPortParameterType OscOutputPortParameterObj;
Parameter* OscOutputPortParameter = &OscOutputPortParameterObj;

//////////////////////////////////////////////////////////////////////
//
// OscOutputHost
//
//////////////////////////////////////////////////////////////////////

class OscOutputHostParameterType : public GlobalParameter
{
  public:
	OscOutputHostParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

OscOutputHostParameterType::OscOutputHostParameterType() :
    GlobalParameter("oscOutputHost", MSG_PARAM_OSC_OUTPUT_HOST)
{
    // not bindable
	type = TYPE_STRING;
}

void OscOutputHostParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setString(c->getOscOutputHost());
}

void OscOutputHostParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	c->setOscOutputHost(value->getString());
}

OscOutputHostParameterType OscOutputHostParameterObj;
Parameter* OscOutputHostParameter = &OscOutputHostParameterObj;

//////////////////////////////////////////////////////////////////////
//
// OscTrace
//
//////////////////////////////////////////////////////////////////////

class OscTraceParameterType : public GlobalParameter
{
  public:
	OscTraceParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

OscTraceParameterType::OscTraceParameterType() :
    GlobalParameter("oscTrace", MSG_PARAM_OSC_TRACE)
{
    // not worth bindable
	type = TYPE_BOOLEAN;
}

void OscTraceParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setBool(c->isOscTrace());
}

void OscTraceParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	c->setOscTrace(value->getBool());
}

OscTraceParameterType OscTraceParameterObj;
Parameter* OscTraceParameter = &OscTraceParameterObj;

//////////////////////////////////////////////////////////////////////
//
// OscEnable
//
//////////////////////////////////////////////////////////////////////

class OscEnableParameterType : public GlobalParameter
{
  public:
	OscEnableParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

OscEnableParameterType::OscEnableParameterType() :
    GlobalParameter("oscEnable", MSG_PARAM_OSC_ENABLE)
{
    // not worth bindable
	type = TYPE_BOOLEAN;
}

void OscEnableParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setBool(c->isOscEnable());
}

void OscEnableParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	c->setOscEnable(value->getBool());
}

OscEnableParameterType OscEnableParameterObj;
Parameter* OscEnableParameter = &OscEnableParameterObj;

/****************************************************************************
 *                                                                          *
 *   							   DEVICES                                  *
 *                                                                          *
 ****************************************************************************/

//////////////////////////////////////////////////////////////////////
//
// InputLatency
//
//////////////////////////////////////////////////////////////////////

class InputLatencyParameterType : public GlobalParameter
{
  public:
	InputLatencyParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
#ifndef HIDE_EXPORT
	void setValue(Action* action);
#endif
};

InputLatencyParameterType::InputLatencyParameterType() :
    GlobalParameter("inputLatency", MSG_PARAM_INPUT_LATENCY)
{
    // not bindable
	type = TYPE_INT;
}

void InputLatencyParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setInt(c->getInputLatency());
}

void InputLatencyParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	c->setInputLatency(value->getInt());
}

/**
 * Binding this is rare but we do set this in test scripts.
 * For this to have any meaning we have to propagate this to the
 * Streams and the Loops via the Tracks.
 */
#ifndef HIDE_EXPORT
void InputLatencyParameterType::setValue(Action* action)
{
    int latency = action->arg.getInt();
    
    Mobius* m = (Mobius*)action->mobius;
	MobiusConfig* config = m->getConfiguration();
	config->setInputLatency(latency);
    
    MobiusConfig* iconfig = m->getInterruptConfiguration();
    if (iconfig != nullptr) {
        iconfig->setInputLatency(latency);

#ifndef HIDE_TRACK
        for (int i = 0 ; i < m->getTrackCount() ; i++) {
            Track* t = m->getTrack(i);
            t->updateGlobalParameters(iconfig);
        }
    }
#endif
}
#endif

InputLatencyParameterType InputLatencyParameterObj;
Parameter* InputLatencyParameter = &InputLatencyParameterObj;

//////////////////////////////////////////////////////////////////////
//
// OutputLatency
//
//////////////////////////////////////////////////////////////////////

class OutputLatencyParameterType : public GlobalParameter
{
  public:
	OutputLatencyParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
#ifndef HIDE_EXPORT
	void setValue(Action* action);
#endif
};

OutputLatencyParameterType::OutputLatencyParameterType() :
    GlobalParameter("outputLatency", MSG_PARAM_OUTPUT_LATENCY)
{
    // not bindable
	type = TYPE_INT;
}

void OutputLatencyParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setInt(c->getOutputLatency());
}

void OutputLatencyParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	c->setOutputLatency(value->getInt());
}

/**
 * Binding this is rare but we do set this in test scripts.
 * For this to have any meaning we have to propagate this to the
 * Streams and Loops via the Tracks.
 */
#ifndef HIDE_EXPORT
void OutputLatencyParameterType::setValue(Action* action)
{
    int latency = action->arg.getInt();

    Mobius* m = (Mobius*)action->mobius;
	MobiusConfig* config = m->getConfiguration();
	config->setOutputLatency(latency);

    MobiusConfig* iconfig = m->getInterruptConfiguration();
    if (iconfig != nullptr) {
        iconfig->setOutputLatency(latency);
        for (int i = 0 ; i < m->getTrackCount() ; i++) {
            Track* t = m->getTrack(i);
            t->updateGlobalParameters(iconfig);
        }
    }
}
#endif

OutputLatencyParameterType OutputLatencyParameterObj;
Parameter* OutputLatencyParameter = &OutputLatencyParameterObj;

//////////////////////////////////////////////////////////////////////
//
// MidiInput
//
//////////////////////////////////////////////////////////////////////

class MidiInputParameterType : public GlobalParameter
{
  public:
	MidiInputParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

MidiInputParameterType::MidiInputParameterType() :
    GlobalParameter("midiInput", MSG_PARAM_MIDI_INPUT)
{
    // not bindable
	type = TYPE_STRING;
}

void MidiInputParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setString(c->getMidiInput());
}

void MidiInputParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	c->setMidiInput(value->getString());
}

MidiInputParameterType MidiInputParameterObj;
Parameter* MidiInputParameter = &MidiInputParameterObj;

//////////////////////////////////////////////////////////////////////
//
// MidiOutput
//
//////////////////////////////////////////////////////////////////////

class MidiOutputParameterType : public GlobalParameter
{
  public:
	MidiOutputParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

MidiOutputParameterType::MidiOutputParameterType() :
    GlobalParameter("midiOutput", MSG_PARAM_MIDI_OUTPUT)
{
    // not bindable
	type = TYPE_STRING;
}

void MidiOutputParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setString(c->getMidiOutput());
}

void MidiOutputParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	c->setMidiOutput(value->getString());
}

MidiOutputParameterType MidiOutputParameterObj;
Parameter* MidiOutputParameter = &MidiOutputParameterObj;

//////////////////////////////////////////////////////////////////////
//
// MidiThrough
//
//////////////////////////////////////////////////////////////////////

class MidiThroughParameterType : public GlobalParameter
{
  public:
	MidiThroughParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

MidiThroughParameterType::MidiThroughParameterType() :
    GlobalParameter("midiThrough", MSG_PARAM_MIDI_THRU)
{
    // not bindable
	type = TYPE_STRING;
}

void MidiThroughParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setString(c->getMidiThrough());
}

void MidiThroughParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	c->setMidiThrough(value->getString());
}

MidiThroughParameterType MidiThroughParameterObj;
Parameter* MidiThroughParameter = &MidiThroughParameterObj;

//////////////////////////////////////////////////////////////////////
//
// PluginMidiInput
//
//////////////////////////////////////////////////////////////////////

class PluginMidiInputParameterType : public GlobalParameter
{
  public:
	PluginMidiInputParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

PluginMidiInputParameterType::PluginMidiInputParameterType() :
    GlobalParameter("pluginMidiInput", MSG_PARAM_PLUGIN_MIDI_INPUT)
{
    // not bindable
	type = TYPE_STRING;
    addAlias("vstMidiInput");
}

void PluginMidiInputParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setString(c->getPluginMidiInput());
}

void PluginMidiInputParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	c->setPluginMidiInput(value->getString());
}

PluginMidiInputParameterType PluginMidiInputParameterObj;
Parameter* PluginMidiInputParameter = &PluginMidiInputParameterObj;

//////////////////////////////////////////////////////////////////////
//
// PluginMidiOutput
//
//////////////////////////////////////////////////////////////////////

class PluginMidiOutputParameterType : public GlobalParameter
{
  public:
	PluginMidiOutputParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

PluginMidiOutputParameterType::PluginMidiOutputParameterType() :
    GlobalParameter("pluginMidiOutput", MSG_PARAM_PLUGIN_MIDI_OUTPUT)
{
    // not bindable
	type = TYPE_STRING;
	addAlias("vstMidiOutput");
}

void PluginMidiOutputParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setString(c->getPluginMidiOutput());
}

void PluginMidiOutputParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	c->setPluginMidiOutput(value->getString());
}

PluginMidiOutputParameterType PluginMidiOutputParameterObj;
Parameter* PluginMidiOutputParameter = &PluginMidiOutputParameterObj;

//////////////////////////////////////////////////////////////////////
//
// PluginMidiThrough
//
//////////////////////////////////////////////////////////////////////

class PluginMidiThroughParameterType : public GlobalParameter
{
  public:
	PluginMidiThroughParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

PluginMidiThroughParameterType::PluginMidiThroughParameterType() :
    GlobalParameter("pluginMidiThrough", MSG_PARAM_PLUGIN_MIDI_THRU)
{
    // not bindable
	type = TYPE_STRING;
	addAlias("vstMidiThrough");
}

void PluginMidiThroughParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setString(c->getPluginMidiThrough());
}

void PluginMidiThroughParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	c->setPluginMidiThrough(value->getString());
}

PluginMidiThroughParameterType PluginMidiThroughParameterObj;
Parameter* PluginMidiThroughParameter = &PluginMidiThroughParameterObj;

//////////////////////////////////////////////////////////////////////
//
// AudioInput
//
//////////////////////////////////////////////////////////////////////

class AudioInputParameterType : public GlobalParameter
{
  public:
	AudioInputParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

AudioInputParameterType::AudioInputParameterType() :
    GlobalParameter("audioInput", MSG_PARAM_AUDIO_INPUT)
{
    // not bindable
	type = TYPE_STRING;
}

void AudioInputParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setString(c->getAudioInput());
}

void AudioInputParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	c->setAudioInput(value->getString());
}

AudioInputParameterType AudioInputParameterObj;
Parameter* AudioInputParameter = &AudioInputParameterObj;

//////////////////////////////////////////////////////////////////////
//
// AudioOutput
//
//////////////////////////////////////////////////////////////////////

class AudioOutputParameterType : public GlobalParameter
{
  public:
	AudioOutputParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

AudioOutputParameterType::AudioOutputParameterType() :
    GlobalParameter("audioOutput", MSG_PARAM_AUDIO_OUTPUT)
{
    // not bindable
	type = TYPE_STRING;
}

void AudioOutputParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setString(c->getAudioOutput());
}

void AudioOutputParameterType::setValue(MobiusConfig* c, ExValue* value)
{
	c->setAudioOutput(value->getString());
}

AudioOutputParameterType AudioOutputParameterObj;
Parameter* AudioOutputParameter = &AudioOutputParameterObj;

//////////////////////////////////////////////////////////////////////
//
// SampleRate
//
//////////////////////////////////////////////////////////////////////

class SampleRateParameterType : public GlobalParameter
{
  public:
	SampleRateParameterType();
	void getValue(MobiusConfig* c, ExValue* value);
	void setValue(MobiusConfig* c, ExValue* value);
};

const char* SAMPLE_RATE_NAMES[] = {
	"44100", "48000", nullptr
};

/**
 * Could be a int, but an enum helps is constrain the value better.
 */
SampleRateParameterType::SampleRateParameterType() :
    GlobalParameter("sampleRate", MSG_PARAM_SAMPLE_RATE)
{
    // not worth bindable
	type = TYPE_ENUM;
	values = SAMPLE_RATE_NAMES;
}

void SampleRateParameterType::getValue(MobiusConfig* c, ExValue* value)
{
	value->setString(values[c->getSampleRate()]);
}

void SampleRateParameterType::setValue(MobiusConfig* c, ExValue* value)
{
    AudioSampleRate rate = (AudioSampleRate)getEnum(value);
	c->setSampleRate(rate);
}

SampleRateParameterType SampleRateParameterObj;
Parameter* SampleRateParameter = &SampleRateParameterObj;
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

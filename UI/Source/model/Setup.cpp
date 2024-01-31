/*
 * Copyright (c) 2010 Jeffrey S. Larson  <jeff@circularlabs.com>
 * All rights reserved.
 * See the LICENSE file for the full copyright and license declaration.
 * 
 * ---------------------------------------------------------------------
 * 
 * Model for a "track setup", a collection of parameters for for
 * all tracks.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../util/Util.h"
#include "../util/MidiUtil.h"

// uses StringList for resettables
#include "../util/List.h"

#include "Parameter.h"

#include "Binding.h"
#include "ExValue.h"
#include "Preset.h"
#include "UserVariable.h"

#include "Setup.h"

/****************************************************************************
 *                                                                          *
 *                                 CONSTANTS                                *
 *                                                                          *
 ****************************************************************************/

/**
 * Parameter defaults.
 * Note that the unit tests depend on some of these, do NOT change them
 * without understanding the consequences for the tests.
 */
#define DEFAULT_MIN_TEMPO 20
#define DEFAULT_MAX_TEMPO 300
#define DEFAULT_BAR_BEATS 4

/****************************************************************************
 *                                                                          *
 *                               XML CONSTANTS                              *
 *                                                                          *
 ****************************************************************************/

#define EL_SETUP_TRACK "SetupTrack"
#define EL_VARIABLES "Variables"

#define ATT_BINDINGS "bindings"
#define ATT_MIDI_CONFIG "midiConfig"

#define ATT_NAME "name"
#define ATT_ACTIVE "active"
#define ATT_TRACK_GROUPS "trackGroups"
#define ATT_RESETABLES "reset"
#define ATT_ACTIVE "active"

/****************************************************************************
 *                                                                          *
 *                                  GLOBALS                                 *
 *                                                                          *
 ****************************************************************************/

// Global functions to return trace names for Setup enumerations
// !! figure out a way to share these with Parameter definitions

const char* GetSyncSourceName(SyncSource src) 
{
    const char* name = "???";
    switch (src) {
        case SYNC_DEFAULT: name = "Default"; break;
        case SYNC_NONE: name = "None"; break;
        case SYNC_TRACK: name = "Track"; break;
        case SYNC_OUT: name = "Out"; break;
        case SYNC_HOST: name = "Host"; break;
        case SYNC_MIDI: name = "MIDI"; break;
    }
    return name;
}

/****************************************************************************
 *                                                                          *
 *   								SETUP                                   *
 *                                                                          *
 ****************************************************************************/

Setup::Setup()
{
	init();
}

Setup::~Setup()
{
	delete mTracks;
	delete mBindings;
}

void Setup::init()
{
	mNext = nullptr;
	mTracks = nullptr;
	mActive = 0;
	mResetables = nullptr;
	mBindings = nullptr;

    initParameters();
}

/**
 * Restore the default parameters expected by the unit tests.
 */
void Setup::initParameters()
{
    // Sync
    mSyncSource         = SYNC_TRACK;
    mSyncUnit           = SYNC_UNIT_BEAT;
    mSyncTrackUnit      = TRACK_UNIT_LOOP;
    mManualStart        = false;
	mMinTempo			= DEFAULT_MIN_TEMPO;
	mMaxTempo			= DEFAULT_MAX_TEMPO;
	mBeatsPerBar		= DEFAULT_BAR_BEATS;
	mMuteSyncMode   	= MUTE_SYNC_TRANSPORT;
    mResizeSyncAdjust   = SYNC_ADJUST_NONE;
	mSpeedSyncAdjust	= SYNC_ADJUST_NONE;
	mRealignTime		= REALIGN_START;
	mOutRealignMode		= REALIGN_RESTART;
}

/**
 * Put the setup into the standard state for unit tests.
 */
void Setup::reset(Preset* p)
{
	mActive = 0;

    // need a default list of these?
    setResetables(nullptr);

    // don't really care what the binding configs are
	setBindings(nullptr);

    // start over with a new SetupTrack list
    setTracks(nullptr);

    for (int i = 0 ; i < DEFAULT_TRACK_COUNT ; i++) {
        SetupTrack* t = getTrack(i);
        t->reset();
        if (p != nullptr)
          t->setPreset(p->getName());
    }

    initParameters();
}

Target* Setup::getTarget()
{
	return TargetSetup;
}

void Setup::setNext(Setup* s) 
{
	mNext = s;
}

Setup* Setup::getNext()
{
	return mNext;
}

Bindable* Setup::getNextBindable()
{
	return mNext;
}

void Setup::setBindings(const char* name)
{
	delete mBindings;
	mBindings = CopyString(name);
}

const char* Setup::getBindings()
{
	return mBindings;
}

int Setup::getActiveTrack()
{
	return mActive;
}

void Setup::setActiveTrack(int i)
{
	mActive = i;
}

void Setup::setResetables(StringList* l)
{
	if (mResetables != l) {
		delete mResetables;
		mResetables = l;
	}
}

StringList* Setup::getResetables()
{
	return mResetables;
}

bool Setup::isResetable(Parameter* p)
{
	return (mResetables != nullptr && mResetables->indexOf((void*)p->getName()) >= 0);
}

SetupTrack* Setup::getTracks()
{
	return mTracks;
}

SetupTrack* Setup::stealTracks()
{
	SetupTrack* list = mTracks;
	mTracks = nullptr;
	return list;
}

void Setup::setTracks(SetupTrack* list)
{
    if (list != mTracks) {
		delete mTracks;
		mTracks = list;
    }
}

SetupTrack* Setup::getTrack(int index)
{
	SetupTrack* track = mTracks;
	SetupTrack* prev = nullptr;

	for (int i = 0 ; i <= index ; i++) {
		if (track == nullptr) {
			track = new SetupTrack();
			if (prev == nullptr)
			  mTracks = track;
			else
			  prev->setNext(track);
		}
		if (i < index) {
			prev = track;
			track = track->getNext();
		}
	}
	return track;
}

/****************************************************************************
 *                                                                          *
 *                              SETUP PARAMETERS                            *
 *                                                                          *
 ****************************************************************************/

SyncSource Setup::getSyncSource()
{
    return mSyncSource;
}

void Setup::setSyncSource(SyncSource src)
{
    mSyncSource = src;
}

SyncUnit Setup::getSyncUnit()
{
    return mSyncUnit;
}

void Setup::setSyncUnit(SyncUnit src)
{
    mSyncUnit = src;
}

SyncTrackUnit Setup::getSyncTrackUnit()
{
    return mSyncTrackUnit;
}

void Setup::setSyncTrackUnit(SyncTrackUnit unit)
{
    mSyncTrackUnit = unit;
}

bool Setup::isManualStart()
{
    return mManualStart;
}

void Setup::setManualStart(bool b)
{
    mManualStart = b;
}

int Setup::getMinTempo()
{
	return mMinTempo;
}

void Setup::setMinTempo(int i)
{
    if (i == 0) i = DEFAULT_MIN_TEMPO;
	mMinTempo = i;
}

int Setup::getMaxTempo()
{
	return mMaxTempo;
}

void Setup::setMaxTempo(int i)
{
    if (i == 0) i = DEFAULT_MAX_TEMPO;
	mMaxTempo = i;
}

int Setup::getBeatsPerBar()
{
	return mBeatsPerBar;
}

void Setup::setBeatsPerBar(int i)
{
	mBeatsPerBar = i;
}

void Setup::setMuteSyncMode(MuteSyncMode i) {
	mMuteSyncMode = i;
}

void Setup::setMuteSyncMode(int i) {
	setMuteSyncMode((MuteSyncMode)i);
}

MuteSyncMode Setup::getMuteSyncMode() {
	return mMuteSyncMode;
}

void Setup::setResizeSyncAdjust(SyncAdjust i) {
	mResizeSyncAdjust = i;
}

void Setup::setResizeSyncAdjust(int i) {
	setResizeSyncAdjust((SyncAdjust)i);
}

SyncAdjust Setup::getResizeSyncAdjust() {
	return mResizeSyncAdjust;
}

void Setup::setSpeedSyncAdjust(SyncAdjust i) {
	mSpeedSyncAdjust = i;
}

void Setup::setSpeedSyncAdjust(int i) {
	setSpeedSyncAdjust((SyncAdjust)i);
}

SyncAdjust Setup::getSpeedSyncAdjust() {
	return mSpeedSyncAdjust;
}

void Setup::setRealignTime(RealignTime t) {
	mRealignTime = t;
}

void Setup::setRealignTime(int i) {
	setRealignTime((RealignTime)i);
}

RealignTime Setup::getRealignTime() {
	return mRealignTime;
}

void Setup::setOutRealignMode(OutRealignMode m) {
	mOutRealignMode = m;
}

void Setup::setOutRealignMode(int i) {
	setOutRealignMode((OutRealignMode)i);
}

OutRealignMode Setup::getOutRealignMode() {
	return mOutRealignMode;
}

/**
 * This one had a clone method that didn't use XML for some reason
 * !! don't like this
 */
Setup* Setup::clone()
{
	Setup* clone = new Setup();
	SetupTrack* tracks = nullptr;
	SetupTrack* last = nullptr;

	// name, number
	clone->Bindable::clone(this);
	
    // can leverage the Parameter list to do the clone
    // not as effiient but saves hard codeing them again
	for (int i = 0 ; Parameter::Parameters[i] != nullptr ; i++) {
		Parameter* p = Parameter::Parameters[i];
        if (p->scope == PARAM_SCOPE_SETUP) {
            ExValue value;
            p->getObjectValue(this, &value);
            p->setObjectValue(clone, &value);
        }
    }

	for (SetupTrack* t = mTracks ; t != nullptr ; t = t->getNext()) {
		SetupTrack* cloneTrack = t->clone();
		if (tracks == nullptr)
		  tracks = cloneTrack;
		else
		  last->setNext(cloneTrack);
		last = cloneTrack;
	}

	clone->setTracks(tracks);

    return clone;
}

/****************************************************************************
 *                                                                          *
 *                                SETUP TRACK                               *
 *                                                                          *
 ****************************************************************************/

SetupTrack::SetupTrack()
{
	init();
}

void SetupTrack::init()
{
	mNext = nullptr;
    mName = nullptr;
	mPreset = nullptr;
	mVariables = nullptr;
	reset();
}

/**
 * Called by the UI to return the track to an initial state.
 * Since we've already been initialized have to be careful
 * about the preset name.
 * !! not sure about variables yet
 * This is also used by the UnitTestSetup script command when
 * initializing the default test setup.
 */
void SetupTrack::reset()
{
	setPreset(nullptr);
    setName(nullptr);
	mFocusLock = false;
    mGroup = 0;
	mInputLevel = 127;
	mOutputLevel = 127;
	mFeedback = 127;
	mAltFeedback = 127;
	mPan = 64;
	mMono = false;
	mAudioInputPort = 0;
	mAudioOutputPort = 0;
	mPluginInputPort = 0;
	mPluginOutputPort = 0;
    mSyncSource = SYNC_DEFAULT;
    mSyncTrackUnit = TRACK_UNIT_DEFAULT;
}

/**
 * Capture the state of an active Track.
 */
#if 0
void SetupTrack::capture(MobiusState* state)
{
    TrackState* t = state->track;

	setPreset(t->preset->getName());

	mFocusLock = t->focusLock;
	mGroup = t->group;
	mInputLevel = t->inputLevel;
	mOutputLevel = t->outputLevel;
	mFeedback = t->feedback;
	mAltFeedback = t->altFeedback;
	mPan = t->pan;

    // not there yet...
	//mMono = t->mono;

	// !! track only has one set of ports for both vst/audio
	// does it even make sense to capture these?
    // Since MobiusState doesn't have them, punt...
    /*
	mAudioInputPort = t->getInputPort();
	mAudioOutputPort = t->getOutputPort();
	mPluginInputPort = t->getInputPort();
	mPluginOutputPort = t->getOutputPort();
    */

    // can no longer get to the Track's Setup
    // via MobiusState
    /*
    SetupTrack* st = t->getSetup();
    mSyncSource = st->getSyncSource();
    mSyncTrackUnit = st->getSyncTrackUnit();
    */

}
#endif

SetupTrack::~SetupTrack()
{
	SetupTrack *el, *next;

	delete mName;
	delete mPreset;
	delete mVariables;

	for (el = mNext ; el != nullptr ; el = next) {
		next = el->getNext();
		el->setNext(nullptr);
		delete el;
	}
}

SetupTrack* SetupTrack::clone()
{
	SetupTrack* t = new SetupTrack();

	// everything but mNext
	t->setName(mName);
	t->setPreset(mPreset);

    // consider using a Parameter loop like we do in Setup
	t->mFocusLock = mFocusLock;
	t->mGroup = mGroup;
	t->mInputLevel = mInputLevel;
	t->mOutputLevel = mOutputLevel;
	t->mFeedback = mFeedback;
	t->mAltFeedback = mAltFeedback;
	t->mPan = mPan;
	t->mMono = mMono;
	t->mAudioInputPort = mAudioInputPort;
	t->mAudioOutputPort = mAudioOutputPort;
	t->mPluginInputPort = mPluginInputPort;
	t->mPluginOutputPort = mPluginOutputPort;
    t->mSyncSource = mSyncSource;
    t->mSyncTrackUnit = mSyncTrackUnit;

	// !! TODO: copy mVariables

	return t;
}


void SetupTrack::setNext(SetupTrack* s) 
{
	mNext = s;
}

SetupTrack* SetupTrack::getNext()
{
	return mNext;
}

void SetupTrack::setName(const char* s)
{
	delete mName;
	mName = CopyString(s);
}

const char* SetupTrack::getName()
{
	return mName;
}

void SetupTrack::setPreset(const char* p)
{
	delete mPreset;
	mPreset = CopyString(p);
}

const char* SetupTrack::getPreset()
{
	return mPreset;
}

void SetupTrack::setFocusLock(bool b)
{
	mFocusLock = b;
}

bool SetupTrack::isFocusLock()
{
	return mFocusLock;
}

int SetupTrack::getGroup()
{
    return mGroup;
}

void SetupTrack::setGroup(int i)
{
    mGroup = i;
}

void SetupTrack::setInputLevel(int i)
{
	mInputLevel = i;
}

int SetupTrack::getInputLevel()
{
	return mInputLevel;
}

void SetupTrack::setOutputLevel(int i)
{
	mOutputLevel = i;
}

int SetupTrack::getOutputLevel()
{
	return mOutputLevel;
}

void SetupTrack::setFeedback(int i)
{
	mFeedback = i;
}

int SetupTrack::getFeedback()
{
	return mFeedback;
}

void SetupTrack::setAltFeedback(int i)
{
	mAltFeedback = i;
}

int SetupTrack::getAltFeedback()
{
	return mAltFeedback;
}

void SetupTrack::setPan(int i)
{
	mPan = i;
}

int SetupTrack::getPan()
{
	return mPan;
}

void SetupTrack::setMono(bool b)
{
	mMono = b;
}

bool SetupTrack::isMono()
{
	return mMono;
}

void SetupTrack::setAudioInputPort(int i)
{
	mAudioInputPort = i;
}

int SetupTrack::getAudioInputPort()
{
	return mAudioInputPort;
}

void SetupTrack::setAudioOutputPort(int i)
{
	mAudioOutputPort = i;
}

int SetupTrack::getAudioOutputPort()
{
	return mAudioOutputPort;
}

void SetupTrack::setPluginInputPort(int i)
{
	mPluginInputPort = i;
}

int SetupTrack::getPluginInputPort()
{
	return mPluginInputPort;
}

void SetupTrack::setPluginOutputPort(int i)
{
	mPluginOutputPort = i;
}

int SetupTrack::getPluginOutputPort()
{
	return mPluginOutputPort;
}

SyncSource SetupTrack::getSyncSource()
{
    return mSyncSource;
}

void SetupTrack::setSyncSource(SyncSource src)
{
    mSyncSource = src;
}

SyncTrackUnit SetupTrack::getSyncTrackUnit()
{
    return mSyncTrackUnit;
}

void SetupTrack::setSyncTrackUnit(SyncTrackUnit unit)
{
    mSyncTrackUnit = unit;
}

UserVariables* SetupTrack::getVariables()
{
    return mVariables;
}

void SetupTrack::setVariables(UserVariables* vars)
{
    delete mVariables;
    mVariables = vars;
}

void SetupTrack::setVariable(const char* name, ExValue* value)
{
	if (name != nullptr) {
		if (mVariables == nullptr)
		  mVariables = new UserVariables();
		mVariables->set(name, value);
	}
}

void SetupTrack::getVariable(const char* name, ExValue* value)
{
	value->setNull();
	if (mVariables != nullptr)
	  mVariables->get(name, value);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

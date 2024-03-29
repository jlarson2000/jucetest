/*
 * Copyright (c) 2010 Jeffrey S. Larson  <jeff@circularlabs.com>
 * All rights reserved.
 * See the LICENSE file for the full copyright and license declaration.
 * 
 * ---------------------------------------------------------------------
 * 
 * Model for the Mobius core configuration.
 * UIConfig has a model for most of the UI configuration.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../util/Trace.h"
#include "../util/Util.h"
#include "../util/List.h"
#include "../util/MidiUtil.h"

#include "Binding.h"
#include "Preset.h"
#include "Setup.h"
#include "ScriptConfig.h"
#include "SampleConfig.h"
#include "OscConfig.h"

#include "MobiusConfig.h"

// defined down in Audio.h, think about where this should live

/**
 * Maximum number of frames that may be used for cross fading.
 */
#define AUDIO_MAX_FADE_FRAMES 256

/**
 * Minimum number of frames that may be used for cross fading.
 * Formerly we allowed zero here, but it's easy for bad config files
 * to leave this out or set it to zero since zero usually means "default".
 * 
 */
#define AUDIO_MIN_FADE_FRAMES 16

/**
 * Default number of frames to use during fade in/out
 * of a newly recorded segment.  At a sample rate of 44100, 
 * a value of 441 results in a 1/10 second fade.  But
 * we don't really need that much.    If the range is too large
 * you hear "breathing".
 *
 * UDPATE: Since we use static buffers for this and the max is only 256, it
 * isn't really possible to set large values for effect.  While fade frames
 * can be set in a global parameter this is no longer exposed in the user interface.
 */
#define AUDIO_DEFAULT_FADE_FRAMES 128

/****************************************************************************
 *                                                                          *
 *   								CONFIG                                  *
 *                                                                          *
 ****************************************************************************/

MobiusConfig::MobiusConfig()
{
	init();
}

MobiusConfig::MobiusConfig(bool dflt)
{
	init();
    mDefault = dflt;
}

void MobiusConfig::init()
{
    mError[0] = 0;
    mDefault = false;
    mHistory = nullptr;
	mMidiInput = nullptr;
	mMidiOutput = nullptr;
	mMidiThrough = nullptr;
	mPluginMidiInput = nullptr;
	mPluginMidiOutput = nullptr;
	mPluginMidiThrough = nullptr;
	mAudioInput = nullptr;
	mAudioOutput = nullptr;
	mUIConfig = nullptr;
	mQuickSave = nullptr;
    mCustomMessageFile = nullptr;
	mUnitTests = nullptr;

	mNoiseFloor = DEFAULT_NOISE_FLOOR;
	mSuggestedLatency = 0;
	mInputLatency = 0;
	mOutputLatency = 0;
	mFadeFrames = AUDIO_DEFAULT_FADE_FRAMES;
	mMaxSyncDrift = DEFAULT_MAX_SYNC_DRIFT;
	mTracks = DEFAULT_TRACKS;
    mTrackGroups = DEFAULT_TRACK_GROUPS;
    mMaxLoops = DEFAULT_MAX_LOOPS;
    mLongPress = DEFAULT_LONG_PRESS_MSECS;

	mFocusLockFunctions = nullptr;
	mMuteCancelFunctions = nullptr;
	mConfirmationFunctions = nullptr;
	mAltFeedbackDisables = nullptr;

	mPresets = nullptr;
    mDefaultPresetName = nullptr;
    mDefaultPreset = nullptr;
    
	mSetups = nullptr;
    mStartingSetupName = nullptr;
    mStartingSetup = nullptr;
    
	mBindings = nullptr;
    mOverlayBindings = nullptr;
    mScriptConfig = nullptr;
    mSampleConfig = nullptr;
	mOscConfig = nullptr;

    mSampleRate = SAMPLE_RATE_44100;
	mMonitorAudio = false;
    mHostRewinds = false;
	mPluginPins = DEFAULT_PLUGIN_PINS;
    mAutoFeedbackReduction = false;
    mIsolateOverdubs = false;
    mIntegerWaveFile = false;
	mSpreadRange = DEFAULT_SPREAD_RANGE;
	mTracePrintLevel = 1;
	mTraceDebugLevel = 2;
	mSaveLayers = false;
	mDriftCheckPoint = DRIFT_CHECK_LOOP;
	mMidiRecordMode = MIDI_TEMPO_AVERAGE;
    mMidiExport = false;
    mHostMidiExport = false;
    mGroupFocusLock = false;

    mNoPresetChanges = false;
    mNoSetupChanges = false;

    // this causes confusion when not on since key bindings often don't work
#ifdef _WIN32
    mDualPluginWindow = true;
#else
    mDualPluginWindow = false;  
#endif

    mOscEnable = false;
    mOscTrace = false;
    mOscInputPort = 7000;
    mOscOutputPort = 8000;
    mOscOutputHost = nullptr;

    mNoSyncBeatRounding = false;
    mLogStatus = false;

    mEdpisms = false;
}

MobiusConfig::~MobiusConfig()
{
    // delete the history list if we have one
	MobiusConfig *el, *next;
	for (el = mHistory ; el != nullptr ; el = next) {
		next = el->getHistory();
		el->setHistory(nullptr);
		delete el;
	}

    delete mMidiInput;
    delete mMidiOutput;
    delete mMidiThrough;
    delete mPluginMidiInput;
    delete mPluginMidiOutput;
    delete mPluginMidiThrough;
    delete mAudioInput;
    delete mAudioOutput;
	delete mUIConfig;
	delete mQuickSave;
    delete mCustomMessageFile;
	delete mUnitTests;

	delete mFocusLockFunctions;
	delete mMuteCancelFunctions;
	delete mConfirmationFunctions;
	delete mAltFeedbackDisables;
	delete mPresets;
    delete mDefaultPresetName;
    delete mSetups;
    delete mStartingSetupName;
    delete mBindings;
    delete mOverlayBindings;
    delete mScriptConfig;
	delete mOscConfig;
	delete mSampleConfig;
}

bool MobiusConfig::isDefault()
{
    return mDefault;
}

void MobiusConfig::setHistory(MobiusConfig* config)
{
    mHistory = config;
}

MobiusConfig* MobiusConfig::getHistory()
{
    return mHistory;
}

int MobiusConfig::getHistoryCount()
{
    int count = 0;
    for (MobiusConfig* c = this ; c != nullptr ; c = c->getHistory())
      count++;
    return count;
}

void MobiusConfig::setMonitorAudio(bool b)
{
	mMonitorAudio = b;
}

bool MobiusConfig::isMonitorAudio()
{
	return mMonitorAudio;
}

void MobiusConfig::setPluginPins(int i)
{
    // zero looks confusing in the UI, default it if we have 
    // an old config file
    if (i == 0) i = DEFAULT_PLUGIN_PINS;
	mPluginPins = i;
}

int MobiusConfig::getPluginPins()
{
	return mPluginPins;
}

/**
 * Pseudo property to expose the pin count as "ports" which
 * are sets of stereo pins.  Ports are what we deal within all other
 * places so this makes a more logical global parameter.
 */
int MobiusConfig::getPluginPorts()
{
	return (mPluginPins / 2);
}

void MobiusConfig::setPluginPorts(int i) 
{
	mPluginPins = i * 2;
}

void MobiusConfig::setHostRewinds(bool b)
{
	mHostRewinds = b;
}

bool MobiusConfig::isHostRewinds()
{
	return mHostRewinds;
}

void MobiusConfig::setAutoFeedbackReduction(bool b)
{
	mAutoFeedbackReduction = b;
}

bool MobiusConfig::isAutoFeedbackReduction()
{
	return mAutoFeedbackReduction;
}

void MobiusConfig::setIsolateOverdubs(bool b)
{
	mIsolateOverdubs = b;
}

bool MobiusConfig::isIsolateOverdubs()
{
	return mIsolateOverdubs;
}

void MobiusConfig::setIntegerWaveFile(bool b)
{
	mIntegerWaveFile = b;
}

bool MobiusConfig::isIntegerWaveFile()
{
	return mIntegerWaveFile;
}

void MobiusConfig::setSpreadRange(int i)
{
	// backward compatibility with old files
	if (i <= 0) 
      i = DEFAULT_SPREAD_RANGE;
    else if (i > MAX_RATE_STEP)
      i = MAX_RATE_STEP;

	mSpreadRange = i;
}

int MobiusConfig::getSpreadRange()
{
	return mSpreadRange;
}

const char* MobiusConfig::getMidiInput() {
	return mMidiInput;
}

void MobiusConfig::setMidiInput(const char* s) {
    delete mMidiInput;
	mMidiInput = CopyString(s);
}

const char* MobiusConfig::getMidiOutput() {
	return mMidiOutput;
}

void MobiusConfig::setMidiOutput(const char* s) {
    delete mMidiOutput;
	mMidiOutput = CopyString(s);
}

const char* MobiusConfig::getMidiThrough() {
	return mMidiThrough;
}

void MobiusConfig::setMidiThrough(const char* s) {
    delete mMidiThrough;
	mMidiThrough = CopyString(s);
}

const char* MobiusConfig::getPluginMidiInput() {
	return mPluginMidiInput;
}

void MobiusConfig::setPluginMidiInput(const char* s) {
    delete mPluginMidiInput;
	mPluginMidiInput = CopyString(s);
}

const char* MobiusConfig::getPluginMidiOutput() {
	return mPluginMidiOutput;
}

void MobiusConfig::setPluginMidiOutput(const char* s) {
    delete mPluginMidiOutput;
	mPluginMidiOutput = CopyString(s);
}

const char* MobiusConfig::getPluginMidiThrough() {
	return mPluginMidiThrough;
}

void MobiusConfig::setPluginMidiThrough(const char* s) {
    delete mPluginMidiThrough;
	mPluginMidiThrough = CopyString(s);
}

const char* MobiusConfig::getAudioInput() {
	return mAudioInput;
}

void MobiusConfig::setAudioInput(const char* s) {
	delete mAudioInput;
    mAudioInput = CopyString(s);
}

const char* MobiusConfig::getAudioOutput() {
	return mAudioOutput;
}

void MobiusConfig::setAudioOutput(const char* s) {
	delete mAudioOutput;
	mAudioOutput = CopyString(s);
}

AudioSampleRate MobiusConfig::getSampleRate() {
	return mSampleRate;
}

void MobiusConfig::setSampleRate(AudioSampleRate rate) {
	mSampleRate = rate;
}

void MobiusConfig::setTracePrintLevel(int i) {
	mTracePrintLevel = i;
}

int MobiusConfig::getTracePrintLevel() {
	return mTracePrintLevel;
}

void MobiusConfig::setTraceDebugLevel(int i) {
	mTraceDebugLevel = i;
}

int MobiusConfig::getTraceDebugLevel() {
	return mTraceDebugLevel;
}

void MobiusConfig::setSaveLayers(bool b) {
	mSaveLayers = b;
}

bool MobiusConfig::isSaveLayers() {
	return mSaveLayers;
}

int MobiusConfig::getNoiseFloor()
{
	return mNoiseFloor;
}

void MobiusConfig::setNoiseFloor(int i)
{
    // this has been stuck zero for quite awhile, initialize it
    if (i == 0) i = DEFAULT_NOISE_FLOOR;
	mNoiseFloor = i;
}

int MobiusConfig::getTracks()
{
	return mTracks;
}

void MobiusConfig::setTracks(int i)
{
	if (i == 0) i = DEFAULT_TRACKS;
	mTracks = i;
}

int MobiusConfig::getTrackGroups()
{
	return mTrackGroups;
}

void MobiusConfig::setTrackGroups(int i)
{
	mTrackGroups = i;
}

int MobiusConfig::getMaxLoops()
{
	return mMaxLoops;
}

void MobiusConfig::setMaxLoops(int i)
{
	mMaxLoops = i;
}

void MobiusConfig::setSuggestedLatencyMsec(int i)
{
	mSuggestedLatency = i;
}

int MobiusConfig::getSuggestedLatencyMsec()
{
	return mSuggestedLatency;
}

int MobiusConfig::getInputLatency()
{
	return mInputLatency;
}

void MobiusConfig::setInputLatency(int i)
{
	mInputLatency = i;
}

int MobiusConfig::getOutputLatency()
{
	return mOutputLatency;
}

void MobiusConfig::setOutputLatency(int i)
{
	mOutputLatency = i;
}

/**
 * Hmm, wanted to let 0 default because upgrades won't have
 * this parameter set.  But this leaves no way to turn off long presses.
 */
void MobiusConfig::setLongPress(int i)
{
	if (i <= 0) 
	  i = DEFAULT_LONG_PRESS_MSECS;
	mLongPress = i;
}

int MobiusConfig::getLongPress()
{
	return mLongPress;
}

/**
 * Originally this was a configurable parameter but the
 * range had to be severly restricted to prevent stack
 * overflow since fade buffers are allocated on the stack.
 * With the reduced range there isn't much need to set this
 * so force it to 128.
 */
int MobiusConfig::getFadeFrames()
{
	return mFadeFrames;
}

void MobiusConfig::setFadeFrames(int i)
{
    // force this to a normal value
	if (i <= 0)
      i = AUDIO_DEFAULT_FADE_FRAMES;

    else if (i < AUDIO_MIN_FADE_FRAMES)
      i = AUDIO_MIN_FADE_FRAMES;

    else if (i > AUDIO_MAX_FADE_FRAMES)
      i = AUDIO_MAX_FADE_FRAMES;

    mFadeFrames = i;
}

int MobiusConfig::getMaxSyncDrift()
{
	return mMaxSyncDrift;
}

void MobiusConfig::setMaxSyncDrift(int i)
{
    // this was stuck low for many people, try to correct that
    if (i == 0) i = 512;
    mMaxSyncDrift = i;
}

void MobiusConfig::setDriftCheckPoint(DriftCheckPoint dcp)
{
	mDriftCheckPoint = dcp;
}

DriftCheckPoint MobiusConfig::getDriftCheckPoint()
{
	return mDriftCheckPoint;
}

ScriptConfig* MobiusConfig::getScriptConfig()
{
    if (mScriptConfig == nullptr)
      mScriptConfig = new ScriptConfig();
    return mScriptConfig;
}

void MobiusConfig::setScriptConfig(ScriptConfig* dc)
{
    if (dc != mScriptConfig) {
        delete mScriptConfig;
        mScriptConfig = dc;
    }
}

OscConfig* MobiusConfig::getOscConfig()
{
	return mOscConfig;
}

void MobiusConfig::setOscConfig(OscConfig* c)
{
	if (c != mOscConfig) {
		delete mOscConfig;
		mOscConfig = c;
	}
}

void MobiusConfig::setUIConfig(const char* s) 
{
	delete mUIConfig;
	mUIConfig = CopyString(s);
}

const char* MobiusConfig::getUIConfig()
{
	return mUIConfig;
}

void MobiusConfig::setQuickSave(const char* s) 
{
	delete mQuickSave;
	mQuickSave = CopyString(s);
}

const char* MobiusConfig::getQuickSave()
{
	return mQuickSave;
}

void MobiusConfig::setCustomMessageFile(const char* s) 
{
	delete mCustomMessageFile;
	mCustomMessageFile = CopyString(s);
}

const char* MobiusConfig::getCustomMessageFile()
{
	return mCustomMessageFile;
}

void MobiusConfig::setUnitTests(const char* s) 
{
	delete mUnitTests;
	mUnitTests = CopyString(s);
}

const char* MobiusConfig::getUnitTests()
{
	return mUnitTests;
}

void MobiusConfig::setSampleConfig(SampleConfig* s)
{
	if (mSampleConfig != s) {
		delete mSampleConfig;
		mSampleConfig = s;
	}
}

SampleConfig* MobiusConfig::getSampleConfig()
{
	return mSampleConfig;
}

StringList* MobiusConfig::getFocusLockFunctions()
{
	return mFocusLockFunctions;
}

void MobiusConfig::setFocusLockFunctions(StringList* l) 
{
	delete mFocusLockFunctions;
	mFocusLockFunctions = l;
}

StringList* MobiusConfig::getMuteCancelFunctions()
{
	return mMuteCancelFunctions;
}

void MobiusConfig::setMuteCancelFunctions(StringList* l) 
{
	delete mMuteCancelFunctions;
	mMuteCancelFunctions = l;
}

StringList* MobiusConfig::getConfirmationFunctions()
{
	return mConfirmationFunctions;
}

void MobiusConfig::setConfirmationFunctions(StringList* l) 
{
	delete mConfirmationFunctions;
	mConfirmationFunctions = l;
}

StringList* MobiusConfig::getAltFeedbackDisables() 
{
	return mAltFeedbackDisables;
}

void MobiusConfig::setAltFeedbackDisables(StringList* l) 
{
	delete mAltFeedbackDisables;
	mAltFeedbackDisables = l;
}

void MobiusConfig::setMidiRecordMode(MidiRecordMode mode) {
	mMidiRecordMode = mode;
}

MidiRecordMode MobiusConfig::getMidiRecordMode() {
	return mMidiRecordMode;
}

void MobiusConfig::setDualPluginWindow(bool b) {
	mDualPluginWindow = b;
}

bool MobiusConfig::isDualPluginWindow() {
	return mDualPluginWindow;
}

void MobiusConfig::setMidiExport(bool b) {
	mMidiExport = b;
}

bool MobiusConfig::isMidiExport() {
	return mMidiExport;
}

void MobiusConfig::setHostMidiExport(bool b) {
	mHostMidiExport = b;
}

bool MobiusConfig::isHostMidiExport() {
	return mHostMidiExport;
}

void MobiusConfig::setGroupFocusLock(bool b) {
	mGroupFocusLock = b;
}

bool MobiusConfig::isGroupFocusLock() {
	return mGroupFocusLock;
}

void MobiusConfig::setNoPresetChanges(bool b) {
	mNoPresetChanges = b;
}

bool MobiusConfig::isNoPresetChanges() {
	return mNoPresetChanges;
}

void MobiusConfig::setNoSetupChanges(bool b) {
	mNoSetupChanges = b;
}

bool MobiusConfig::isNoSetupChanges() {
	return mNoSetupChanges;
}

void MobiusConfig::setNoSyncBeatRounding(bool b) {
	mNoSyncBeatRounding = b;
}

bool MobiusConfig::isNoSyncBeatRounding() {
	return mNoSyncBeatRounding;
}

void MobiusConfig::setLogStatus(bool b) {
	mLogStatus = b;
}

bool MobiusConfig::isLogStatus() {
	return mLogStatus;
}

void MobiusConfig::setEdpisms(bool b) {
	mEdpisms = b;
}

bool MobiusConfig::isEdpisms() {
	return mEdpisms;
}

/****************************************************************************
 *                                                                          *
 *                                    OSC                                   *
 *                                                                          *
 ****************************************************************************/

void MobiusConfig::setOscInputPort(int port)
{
    mOscInputPort = port;
}

int MobiusConfig::getOscInputPort()
{
    return mOscInputPort;
}

void MobiusConfig::setOscOutputPort(int port)
{
    mOscOutputPort = port;
}

int MobiusConfig::getOscOutputPort()
{
    return mOscOutputPort;
}

void MobiusConfig::setOscOutputHost(const char* s)
{
    delete mOscOutputHost;
    mOscOutputHost = CopyString(s);
}

const char* MobiusConfig::getOscOutputHost()
{
    return mOscOutputHost;
}

void MobiusConfig::setOscTrace(bool b)
{
    mOscTrace = b;
}

bool MobiusConfig::isOscTrace()
{
    return mOscTrace;
}

void MobiusConfig::setOscEnable(bool b)
{
    mOscEnable = b;
}

bool MobiusConfig::isOscEnable()
{
    return mOscEnable;
}

/****************************************************************************
 *                                                                          *
 *                             PRESET MANAGEMENT                            *
 *                                                                          *
 ****************************************************************************/

Preset* MobiusConfig:: getPresets() 
{
	return mPresets;
}

void MobiusConfig::setPresets(Preset* list)
{
    if (list != mPresets) {
		delete mPresets;
		mPresets = list;
    }
}
	
void MobiusConfig::addPreset(Preset* p) 
{
    mPresets = (Preset*)Structure::append(mPresets, p);
}

/**
 * Look up a preset by name.
 */
Preset* MobiusConfig::getPreset(const char* name)
{
    return (Preset*)(Structure::find(mPresets, name));
}

/**
 * Look up a preset by ordinal.
 */
Preset* MobiusConfig::getPreset(int ordinal)
{
    return (Preset*)Structure::get(mPresets, ordinal);
}

const char* MobiusConfig::getDefaultPresetName()
{
    return mDefaultPresetName;
}

void MobiusConfig::setDefaultPresetName(const char* name)
{
    delete mDefaultPresetName;
    mDefaultPresetName = CopyString(name);
}

/**
 * Return the Preset object that is considered the default preset.
 * This is a transient runtime value that is calculated by
 * searching the Preset list using the persistent mDefaultPresetName.
 * It is cached to avoid a linear string search very time.
 *
 * Like getStartingSetup we will try to fix misconfiguration so an
 * object can always be returned.
 */
Preset* MobiusConfig::getDefaultPreset()
{
    if (mDefaultPreset == nullptr) {
        if (mDefaultPresetName == nullptr) {
            // misconfiguration, pick the first one
            // note that this does memory allocation and should not
            // be done in the kernel but it's an unusual situation
            // still should switch to static arrays for names
            Trace(1, "Default preset name not set, choosing the first\n"); 
            if (mPresets == nullptr) {
                // really raw config, bootstrap one
                Trace(1, "Bootstrapping default preset, shouldn't be here\n");
                mPresets = new Preset();
                mPresets->setName("Default");
            }
            setDefaultPresetName(mPresets->getName());
        }

        // now the usual lookup by name
        mDefaultPreset = getPreset(mDefaultPresetName);
        
        if (mDefaultPreset == nullptr) {
            // name was misconfigured, should not happen
            Trace(1, "Misconfigured default preset: %s does not exist, choosing the first\n",
                  mDefaultPresetName);
            // note that setting the name clears the cache so have to do that first
            setDefaultPresetName(mPresets->getName());
            mDefaultPreset = mPresets;
        }
    }
    return mDefaultPreset;
}

/****************************************************************************
 *                                                                          *
 *   						   SETUP MANAGEMENT                             *
 *                                                                          *
 ****************************************************************************/

Setup* MobiusConfig::getSetups() 
{
	return mSetups;
}

void MobiusConfig::setSetups(Setup* list)
{
    if (list != mSetups) {
		delete mSetups;
		mSetups = list;
        // setting the list might invalidate the activeSetup name
    }
}
	
void MobiusConfig::addSetup(Setup* s) 
{
    mSetups = (Setup*)Structure::append(mSetups, s);
}

Setup* MobiusConfig::getSetup(const char* name)
{
    return (Setup*)Structure::find(mSetups, name);
}

Setup* MobiusConfig::getSetup(int ordinal)
{
    return (Setup*)Structure::get(mSetups, ordinal);
}

//
// Starting Setup
//

const char* MobiusConfig::getStartingSetupName()
{
    return mStartingSetupName;
}

void MobiusConfig::setStartingSetupName(const char* name)
{
    delete mStartingSetupName;
    mStartingSetupName = CopyString(name);
    // cache is now invalid
    mStartingSetup = nullptr;
}

/**
 * Return the Setup object that is considered the starting setup.
 * This is a transient runtime value that is calculated by
 * searching the Setup list using the persistent mStartingSetupName.
 * It is cached to avoid a linear string search very time.
 *
 * So system code can depend on the return value being non-null
 * we will boostrap an object if one cannot be found and fix misconfiguration.
 * Not sure I like this but it would be unusual, and avoids crashes.
 * The code here looks complex but it's mostly just handling corner
 * cases that shouldn't happen.
 */
Setup* MobiusConfig::getStartingSetup()
{
    if (mStartingSetup == nullptr) {
        if (mStartingSetupName == nullptr) {
            // misconfiguration, pick the first one
            // note that this does memory allocation and should not
            // be done in the kernel but it's an unusual situation
            // still should switch to static arrays for names
            Trace(1, "Starting setup name not set, default to the first one\n"); 
            if (mSetups == nullptr) {
                // really raw config, bootstrap one
                Trace(1, "Bootstrapping Setup, shouldn't be here\n");
                mSetups = new Setup();
                mSetups->setName("Default");
            }
            setStartingSetupName(mSetups->getName());
        }

        // now the usual lookup by name
        mStartingSetup = getSetup(mStartingSetupName);
        
        if (mStartingSetup == nullptr) {
            // name was misconfigured, should not happen
            Trace(1, "Misconfigured starting setup, %s does not exist, defaulting to first\n",
                  mStartingSetupName);
            // note that setting the name clears the cache so have to do that first
            setStartingSetupName(mSetups->getName());
            mStartingSetup = mSetups;
        }
    }
    return mStartingSetup;
}

/****************************************************************************
 *                                                                          *
 *   						 BINDINGS MANAGEMENT                            *
 *                                                                          *
 ****************************************************************************/
/*
 * The first object on the list is always considered to be the "base"
 * configuration and is always active.  One additional "overlay"
 * configuration may also be selected.
 */

BindingSet* MobiusConfig::getBindingSets()
{
	return mBindings;
}

// no set function, I guess to enforce that you can't take the
// first one away, not really necessary

void MobiusConfig::addBindingSet(BindingSet* bs) 
{
    mBindings = (BindingSet*)Structure::append(mBindings, bs);
}

const char* MobiusConfig::getOverlayBindings()
{
	return mOverlayBindings;
}

void MobiusConfig::setOverlayBindings(const char* name)
{
    delete mOverlayBindings;
    mOverlayBindings = CopyString(name);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

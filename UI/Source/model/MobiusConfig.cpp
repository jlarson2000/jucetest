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
#include "../util/XmlModel.h"
#include "../util/XmlBuffer.h"
#include "../util/XomParser.h"

//#include "Qwin.h"

//#include "Binding.h"
//#include "Function.h"
//#include "Mobius.h"
//#include "Parameter.h"
//#include "Resampler.h"
//#include "Script.h"
//#include "Setup.h"

// add these later
//#include "Sample.h"
#define HIDE_SAMPLES

#define HIDE_OSC
//#include "OscConfig.h"

#define HIDE_UICONFIG

// temporary
#include "OldBinding.h"

#include "Binding.h"
#include "Preset.h"
#include "Setup.h"
#include "MobiusConfig.h"

// this does XML like I think it should
// don't have Parameter do the XML, do it in the object and reference
// the Parameter for the names
#include "Parameter.h"

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

//////////////////////////////////////////////////////////////////////
//
// XML Constants
//
//////////////////////////////////////////////////////////////////////

#define EL_CONFIG "MobiusConfig"
#define ATT_LANGUAGE "language"
#define ATT_SETUP "setup"
#define ATT_MIDI_CONFIG "midiConfig"
#define ATT_SUGGESTED_LATENCY "suggestedLatencyMsec"
#define ATT_UI_CONFIG  "uiConfig"
#define ATT_PLUGIN_PINS "pluginPins"
#define ATT_PLUGIN_HOST_REWINDS "pluginHostRewinds"

#define ATT_NO_SYNC_BEAT_ROUNDING "noSyncBeatRounding"

#define ATT_OVERLAY_BINDINGS "overlayBindings"

#define EL_FOCUS_LOCK_FUNCTIONS "FocusLockFunctions"
// old name for FocusLockFunctions
#define EL_GROUP_FUNCTIONS "GroupFunctions"
#define EL_MUTE_CANCEL_FUNCTIONS "MuteCancelFunctions"
#define EL_CONFIRMATION_FUNCTIONS "ConfirmationFunctions"
#define EL_ALT_FEEDBACK_DISABLES "AltFeedbackDisables"
#define EL_STRING "String"

#define EL_SCRIPT_CONFIG "ScriptConfig"
#define EL_SCRIPT_REF "ScripRef"
#define ATT_FILE "file"

#define EL_CONTROL_SURFACE "ControlSurface"
#define ATT_NAME "name"

#define EL_OSC_CONFIG "OscConfig"

#define ATT_LOG_STATUS "logStatus"
#define ATT_EDPISMS "edpisms"

/****************************************************************************
 *                                                                          *
 *   							  UTILITIES                                 *
 *                                                                          *
 ****************************************************************************/

int XmlGetEnum(XmlElement* e, const char *name, const char** names)
{
	int value = 0;
	const char *attval = e->getAttribute(name);
	if (attval != NULL) {
		for (int i = 0 ; names[i] != NULL ; i++) {
			if (!strcmp(attval, names[i])) {
				value = i;
				break;
			}
		}
	}
	return value;
}

int XmlGetEnum(const char* str, const char** names)
{
	int value = 0;
	if (str != NULL) {
		for (int i = 0 ; names[i] != NULL ; i++) {
			if (!strcmp(str, names[i])) {
				value = i;
				break;
			}
		}
	}
	return value;
}

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

MobiusConfig::MobiusConfig(const char *xml)
{
	init();
	parseXml(xml);
}


void MobiusConfig::init()
{
    mError[0] = 0;
    mDefault = false;
    mHistory = NULL;
	mLanguage = NULL;
	mMidiInput = NULL;
	mMidiOutput = NULL;
	mMidiThrough = NULL;
	mPluginMidiInput = NULL;
	mPluginMidiOutput = NULL;
	mPluginMidiThrough = NULL;
	mAudioInput = NULL;
	mAudioOutput = NULL;
#ifndef HIDE_UICONFIG
	mUIConfig = NULL;
#endif    
	mQuickSave = NULL;
    mCustomMessageFile = NULL;
	mUnitTests = NULL;

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

	mFocusLockFunctions = NULL;
	mMuteCancelFunctions = NULL;
	mConfirmationFunctions = NULL;
	mAltFeedbackDisables = NULL;

	mPresets = NULL;
	mPreset = NULL;
	mSetups = NULL;
	mSetup = NULL;
	mBindingConfigs = NULL;
    mOverlayBindings = NULL;
	mMidiConfigs = NULL;
    mSelectedMidiConfig = NULL;
    mScriptConfig = NULL;
	mControlSurfaces = NULL;
#ifndef HIDE_OSC
	mOscConfig = NULL;
#endif    
#ifdef HIDE_SAMPLES
	mSamples = NULL;
#endif
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
    mOscOutputHost = NULL;

    mNoSyncBeatRounding = false;
    mLogStatus = false;

    mEdpisms = false;
}

MobiusConfig::~MobiusConfig()
{
    // delete the history list if we have one
	MobiusConfig *el, *next;
	for (el = mHistory ; el != NULL ; el = next) {
		next = el->getHistory();
		el->setHistory(NULL);
		delete el;
	}

	delete mLanguage;
    delete mMidiInput;
    delete mMidiOutput;
    delete mMidiThrough;
    delete mPluginMidiInput;
    delete mPluginMidiOutput;
    delete mPluginMidiThrough;
    delete mAudioInput;
    delete mAudioOutput;
#ifndef HIDE_UICONFIG
	delete mUIConfig;
#endif    
	delete mQuickSave;
    delete mCustomMessageFile;
	delete mUnitTests;

	delete mFocusLockFunctions;
	delete mMuteCancelFunctions;
	delete mConfirmationFunctions;
	delete mAltFeedbackDisables;
	delete mPresets;
    delete mSetups;
    delete mBindingConfigs;
    delete mMidiConfigs;
    delete mSelectedMidiConfig;
    delete mScriptConfig;
    delete mControlSurfaces;
#ifndef HIDE_OSC
	delete mOscConfig;
#endif    
#ifndef HIDE_SAMPLES
	delete mSamples;
#endif    
}

bool MobiusConfig::isDefault()
{
    return mDefault;
}

MobiusConfig* MobiusConfig::clone()
{
    char* xml = toXml();
    MobiusConfig* clone = new MobiusConfig(xml);
    delete xml;

    clone->setCurrentPreset(getCurrentPresetIndex());
	clone->setCurrentSetup(getCurrentSetupIndex());
    clone->setOverlayBindingConfig(getOverlayBindingConfigIndex());

    // these aren't handled by XML serialization
    clone->mNoPresetChanges = mNoPresetChanges;
    clone->mNoSetupChanges = mNoSetupChanges;

    return clone;
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
    for (MobiusConfig* c = this ; c != NULL ; c = c->getHistory())
      count++;
    return count;
}

/**
 * Number the presets, setups, or binding configs after editing.
 */
void MobiusConfig::numberThings(Bindable* things)
{
	int count = 0;
	for (Bindable* b = things ; b != NULL ; b = b->getNextBindable())
	  b->setNumber(count++);
}

int MobiusConfig::countThings(Bindable* things)
{
    int count = 0;
    for (Bindable* b = things ; b != NULL ; b = b->getNextBindable())
      count++;
    return count;
}

const char* MobiusConfig::getLanguage()
{
	return mLanguage;
}

void MobiusConfig::setLanguage(const char* lang)
{
	delete mLanguage;
	mLanguage = CopyString(lang);
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
    if (mScriptConfig == NULL)
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

ControlSurfaceConfig* MobiusConfig::getControlSurfaces()
{
    return mControlSurfaces;
}

void MobiusConfig::setControlSurfaces(ControlSurfaceConfig* list)
{
	if (list != mControlSurfaces) {
		delete mControlSurfaces;
		mControlSurfaces = list;
	}
}

void MobiusConfig::addControlSurface(ControlSurfaceConfig* cs)
{
	// keep them ordered
	ControlSurfaceConfig *prev;
	for (prev = mControlSurfaces ; prev != NULL && prev->getNext() != NULL ; 
		 prev = prev->getNext());

	if (prev == NULL)
	  mControlSurfaces = cs;
	else
	  prev->setNext(cs);
}

#ifndef HIDE_OSC
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
#endif

#ifndef HIDE_UICONFIG
void MobiusConfig::setUIConfig(const char* s) 
{
	delete mUIConfig;
	mUIConfig = CopyString(s);
}

const char* MobiusConfig::getUIConfig()
{
	return mUIConfig;
}
#endif

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

#ifndef HIDE_SAMPLES
void MobiusConfig::setSamples(Samples* s)
{
	if (mSamples != s) {
		delete mSamples;
		mSamples = s;
	}
}

Samples* MobiusConfig::getSamples()
{
	return mSamples;
}
#endif

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

/**
 * Ensure that all of the presets and midi configs have names.
 * Necessary so they can be identified in a GUI.
 */
void MobiusConfig::generateNames()
{
    generateNames(mPresets, "Preset", NULL);
    generateNames(mSetups, "Setup", NULL);
    generateNames(mBindingConfigs, "Bindings", MIDI_COMMON_BINDINGS_NAME);
}

/**
 * Generate unique names for a list of bindables.
 * This isn't as simple as just genering "Foo N" names based
 * on list position since the previously generated names may still 
 * exist in the list but in a different position.
 *
 * In theory this is an ineffecient algorithm if the list is long
 * and the number of previously generated names is large.  That isn't
 * normal or advised, so screw 'em.
 */
void MobiusConfig::generateNames(Bindable* bindables, 
                                         const char* prefix,
                                         const char* baseName)
{
    char buf[128];
    int count = 1;

	for (Bindable* b = bindables ; b != NULL ; b = b->getNextBindable()) {
        if (baseName && b == bindables) {
            // force the name of the first one
            if (!StringEqual(baseName, b->getName()))
              b->setName(baseName);
        }
        else if (b->getName() == NULL) {
            Bindable* existing;
            do {
                // search for name in use
                existing = NULL;
                sprintf(buf, "%s %d", prefix, count);
                for (Bindable* b2 = bindables ; b2 != NULL ; 
                     b2 = b2->getNextBindable()) {
                    if (StringEqual(buf, b2->getName())) {
                        existing = b2;
                        break;
                    }
                }
                if (existing != NULL)
                  count++;
            } while (existing != NULL);

            b->setName(buf);
        }
    }
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

int MobiusConfig::getPresetCount()
{
    return countThings(mPresets);
}

void MobiusConfig::setPresets(Preset* list)
{
    if (list != mPresets) {
		delete mPresets;
		mPresets = list;
		numberThings(mPresets);
    }
}
	
void MobiusConfig::addPreset(Preset* p) 
{
	int count = 0;

	// keep them ordered
	Preset *prev;
	for (prev = mPresets ; prev != NULL && prev->getNext() != NULL ; 
		 prev = prev->getNext());

	if (prev == NULL)
	  mPresets = p;
	else
	  prev->setNext(p);

    if (mPreset == NULL)
      mPreset = p;

	numberThings(mPresets);
}

/**
 * Note that this should only be called on a cloned MobiusConfig that
 * the interrupt handler can't be using.
 */
void MobiusConfig::removePreset(Preset* preset) 
{
	Preset* prev = NULL;
	for (Preset* p = mPresets ; p != NULL ; p = p->getNext()) {
		if (p != preset)
		  prev = p;
		else {
			if (prev == NULL)
			  mPresets = p->getNext();
			else 
			  prev->setNext(p->getNext());
			p->setNext(NULL);

			if (p == mPreset)
			  mPreset = mPresets;
		}
	}
	numberThings(mPresets);
}

Preset* MobiusConfig::getPreset(const char* name)
{
	Preset* found = NULL;
	if (name != NULL) {
		for (Preset* p = mPresets ; p != NULL ; p = p->getNext()) {
            if (StringEqualNoCase(name, p->getName())) {
				found = p;
				break;
			}
		}
	}
	return found;
}

Preset* MobiusConfig::getPreset(int index)
{
    Preset* found = NULL;
    int i = 0;

    for (Preset* p = mPresets ; p != NULL ; p = p->getNext(), i++) {
        if (i == index) {
            found = p;
            break;
        }
    }
    return found;
}

/**
 * Get the first preset, bootstrapping if we have to.
 */
Preset* MobiusConfig::getDefaultPreset()
{
    if (mPresets == NULL)
      mPresets = new Preset("Default");
    return mPresets;
}

/**
 * Get what is considered to be the current preset.
 * This is used only when conveying preset selection between
 * Mobius and the PresetDialog.
 */
Preset* MobiusConfig::getCurrentPreset()
{
	if (mPreset == NULL) {
		if (mPresets == NULL)
		  mPresets = new Preset("Default");
		mPreset = mPresets;
	}
	return mPreset;
}

int MobiusConfig::getCurrentPresetIndex()
{
    int index = 0;
    int i = 0;

	if (mPreset == NULL)
	  mPreset = mPresets;

    // don't need to do it this way if we can assume they're numbered!?
    for (Preset* p = mPresets ; p != NULL ; p = p->getNext(), i++) {
        if (p == mPreset) {
            index = i;
            break;
        }
    }
    return index;
}

void MobiusConfig::setCurrentPreset(Preset* p)
{
	mPreset = p;
}

Preset* MobiusConfig::setCurrentPreset(int index)
{
    Preset* p = getPreset(index);
    if (p != NULL) 
	  mPreset = p;
    return mPreset;
}

Preset* MobiusConfig::setCurrentPreset(const char* name)
{
	mPreset = getPreset(name);

	// would it be more useful to return the previous preset?
	return mPreset;
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

int MobiusConfig::getSetupCount()
{
    return countThings(mSetups);
}

void MobiusConfig::setSetups(Setup* list)
{
    if (list != mSetups) {
		delete mSetups;
		mSetups = list;
		numberThings(mSetups);
    }
}
	
void MobiusConfig::addSetup(Setup* p) 
{
	int count = 0;

	// keep them ordered
	Setup *prev;
	for (prev = mSetups ; prev != NULL && prev->getNext() != NULL ; 
		 prev = prev->getNext());

	if (prev == NULL)
	  mSetups = p;
	else
	  prev->setNext(p);

    if (mSetup == NULL)
      mSetup = p;

    numberThings(mSetups);
}

/**
 * Note that this should only be called on a cloned MobiusConfig that
 * the interrupt handler can't be using.
 */
void MobiusConfig::removeSetup(Setup* preset) 
{
	Setup* prev = NULL;
	for (Setup* p = mSetups ; p != NULL ; p = p->getNext()) {
		if (p != preset)
		  prev = p;
		else {
			if (prev == NULL)
			  mSetups = p->getNext();
			else 
			  prev->setNext(p->getNext());
			p->setNext(NULL);

			if (p == mSetup)
			  mSetup = mSetups;
		}
	}
    numberThings(mSetups);
}

Setup* MobiusConfig::getSetup(const char* name)
{
	Setup* found = NULL;
	if (name != NULL) {
		for (Setup* p = mSetups ; p != NULL ; p = p->getNext()) {
			if (StringEqualNoCase(name, p->getName())) {
				found = p;
				break;
			}
		}
	}
	return found;
}

Setup* MobiusConfig::getSetup(int index)
{
    Setup* found = NULL;
    int i = 0;

    for (Setup* p = mSetups ; p != NULL ; p = p->getNext(), i++) {
        if (i == index) {
            found = p;
            break;
        }
    }
    return found;
}

/**
 * If there is no currently selected setup, we pick the first one.
 */
Setup* MobiusConfig::getCurrentSetup()
{
	if (mSetup == NULL) {
		if (mSetups == NULL)
		  mSetups = new Setup();
		mSetup = mSetups;
	}
	return mSetup;
}

int MobiusConfig::getCurrentSetupIndex()
{
    int index = 0;
    int i = 0;

	if (mSetup == NULL)
	  mSetup = mSetups;

    for (Setup* p = mSetups ; p != NULL ; p = p->getNext(), i++) {
        if (p == mSetup) {
            index = i;
            break;
        }
    }
    return index;
}

/**
 * Normally we'll be given an object that is on our list
 * but we make sure.  We have historically chosen the object
 * with a matching name whether or not it was the same object.
 * Note that this means you have to generate names first if you've
 * just added something.
 */
void MobiusConfig::setCurrentSetup(Setup* p)
{
	if (p != NULL) {
		// these should be the same object, but make sure
 Setup* cur = getSetup(p->getName());
		if (cur != NULL) 
		  mSetup= cur;
	}
}

Setup* MobiusConfig::setCurrentSetup(int index)
{
    Setup* p = getSetup(index);
    if (p != NULL) 
	  mSetup = p;
    return mSetup;
}

Setup* MobiusConfig::setCurrentSetup(const char* name)
{
	mSetup = getSetup(name);

	// would it be more useful to return the previous preset?
	return mSetup;
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

BindingConfig* MobiusConfig::getBindingConfigs()
{
	return mBindingConfigs;
}

/**
 * Number of possible binding configs.
 * Currently used only by OscConfig to gether tha max value for
 * selectable binding configs.
 */
int MobiusConfig::getBindingConfigCount()
{
    return countThings(mBindingConfigs);
}

void MobiusConfig::addBindingConfig(BindingConfig* c) 
{
	// keep them ordered
	BindingConfig *prev;
	for (prev = mBindingConfigs ; prev != NULL && prev->getNext() != NULL ; 
		 prev = prev->getNext());
	if (prev == NULL)
	  mBindingConfigs = c;
	else
	  prev->setNext(c);

    numberThings(mBindingConfigs);
}

/**
 * This should ONLY be called for secondary BindingConfigs, the first
 * one on the list is not supposed to be removable.
 */
void MobiusConfig::removeBindingConfig(BindingConfig* config) 
{
	BindingConfig* prev = NULL;
	for (BindingConfig* p = mBindingConfigs ; p != NULL ; p = p->getNext()) {
		if (p != config)
		  prev = p;
		else {
			if (prev == NULL) {
                // UI should have prevented this
                Trace(1, "Removing base BindingConfig!!\n");
                mBindingConfigs = p->getNext();
            }
			else 
			  prev->setNext(p->getNext());

			p->setNext(NULL);

			if (p == mOverlayBindings)
			  mOverlayBindings = NULL;
		}
	}
    numberThings(mBindingConfigs);
}

BindingConfig* MobiusConfig::getBindingConfig(const char* name)
{
	BindingConfig* found = NULL;
    if (name == NULL) {
        // always the base config
        found = mBindingConfigs;
    }
    else {
		for (BindingConfig* p = mBindingConfigs ; p != NULL ; p = p->getNext()) {
			if (StringEqualNoCase(name, p->getName())) {
				found = p;
				break;
			}
		}
	}
	return found;
}

BindingConfig* MobiusConfig::getBindingConfig(int index)
{
    BindingConfig* found = NULL;
    int i = 0;

    for (BindingConfig* c = mBindingConfigs ; c != NULL ; c = c->getNext(), i++) {
        if (i == index) {
            found = c;
            break;
        }
    }
    return found;
}

/**
 * The "base" binding config is always the first.
 */
BindingConfig* MobiusConfig::getBaseBindingConfig()
{
    if (mBindingConfigs == NULL)
      mBindingConfigs = new BindingConfig();
	return mBindingConfigs;
}

BindingConfig* MobiusConfig::getOverlayBindingConfig()
{
    // it is important this self-heal if it got corrupted
    if (mOverlayBindings == mBindingConfigs)
      mOverlayBindings = NULL;
	return mOverlayBindings;
}

int MobiusConfig::getOverlayBindingConfigIndex()
{
    BindingConfig* overlay = getOverlayBindingConfig();

    int index = 0;
    int i = 0;
    for (BindingConfig* b = mBindingConfigs ; b != NULL ; 
         b = b->getNext(), i++) {

        if (b == overlay) {
            index = i;
            break;
        }
    }
    return index;
}

void MobiusConfig::setOverlayBindingConfig(BindingConfig* b)
{
    // ignore if it's the base
    // it is important we do this so it can self-heal if the
    // XML got screwed up or when processing dynamic Actions with a
    // bad overlay number
    if (b == mBindingConfigs)
      mOverlayBindings = NULL;
    else
      mOverlayBindings = b;
}

BindingConfig* MobiusConfig::setOverlayBindingConfig(const char* name)
{
    setOverlayBindingConfig(getBindingConfig(name));
	// would it be more useful to return the previous config?
	return mOverlayBindings;
}

BindingConfig* MobiusConfig::setOverlayBindingConfig(int index)
{
    BindingConfig* b = getBindingConfig(index);
    // ignore invalid indexes, don't reset to the base?
    if (b != NULL)
      setOverlayBindingConfig(b);

    return mOverlayBindings;
}

/****************************************************************************
 *                                                                          *
 *   								 XML                                    *
 *                                                                          *
 ****************************************************************************/

void MobiusConfig::parseXml(const char *src) 
{
    mError[0] = 0;
	XomParser* p = new XomParser();
	XmlDocument* d = p->parse(src);
    XmlElement* e = NULL;

	if (d != NULL)
      e = d->getChildElement();

    if (e != NULL)
      parseXml(e);
    else {
        // must have been a parse error
        CopyString(p->getError(), mError, sizeof(mError));
    }
    delete d;
	delete p;
}

/**
 * Return the error message if it is set.
 */
const char* MobiusConfig::getError()
{
    return (mError[0] != 0) ? mError : NULL;
}

void MobiusConfig::parseXml(XmlElement* e)
{
    const char* setup = e->getAttribute(ATT_SETUP);
	const char* bconfig = e->getAttribute(ATT_OVERLAY_BINDINGS);

    // save this for upgrade
    setSelectedMidiConfig(e->getAttribute(ATT_MIDI_CONFIG));

	// !! need to start iterating over GlobalParameters to 
	// automatic some of this

	setLanguage(e->getAttribute(ATT_LANGUAGE));
	setMidiInput(e->getAttribute(MidiInputParameter->getName()));
	setMidiOutput(e->getAttribute(MidiOutputParameter->getName()));
	setMidiThrough(e->getAttribute(MidiThroughParameter->getName()));
	setPluginMidiInput(e->getAttribute(PluginMidiInputParameter->getName()));
	setPluginMidiOutput(e->getAttribute(PluginMidiOutputParameter->getName()));
	setPluginMidiThrough(e->getAttribute(PluginMidiThroughParameter->getName()));
	setAudioInput(e->getAttribute(AudioInputParameter->getName()));
	setAudioOutput(e->getAttribute(AudioOutputParameter->getName()));
#ifndef HIDE_UICONFIG
	setUIConfig(e->getAttribute(ATT_UI_CONFIG));
#endif    
	setQuickSave(e->getAttribute(QuickSaveParameter->getName()));
	setUnitTests(e->getAttribute(UnitTestsParameter->getName()));
	setCustomMessageFile(e->getAttribute(CustomMessageFileParameter->getName()));

	setNoiseFloor(e->getIntAttribute(NoiseFloorParameter->getName()));
	setSuggestedLatencyMsec(e->getIntAttribute(ATT_SUGGESTED_LATENCY));
	setInputLatency(e->getIntAttribute(InputLatencyParameter->getName()));
	setOutputLatency(e->getIntAttribute(OutputLatencyParameter->getName()));
	setMaxSyncDrift(e->getIntAttribute(MaxSyncDriftParameter->getName()));
	setTracks(e->getIntAttribute(TracksParameter->getName()));
	setTrackGroups(e->getIntAttribute(TrackGroupsParameter->getName()));
	setMaxLoops(e->getIntAttribute(MaxLoopsParameter->getName()));
	setLongPress(e->getIntAttribute(LongPressParameter->getName()));

	setMonitorAudio(e->getBoolAttribute(MonitorAudioParameter->getName()));
	setHostRewinds(e->getBoolAttribute(ATT_PLUGIN_HOST_REWINDS));
	setPluginPins(e->getIntAttribute(ATT_PLUGIN_PINS));
	setAutoFeedbackReduction(e->getBoolAttribute(AutoFeedbackReductionParameter->getName()));
    // don't allow this to be persisted any more, can only be set in scripts
	//setIsolateOverdubs(e->getBoolAttribute(IsolateOverdubsParameter->getName()));
	setIntegerWaveFile(e->getBoolAttribute(IntegerWaveFileParameter->getName()));
	setSpreadRange(e->getIntAttribute(SpreadRangeParameter->getName()));
	setTracePrintLevel(e->getIntAttribute(TracePrintLevelParameter->getName()));
	setTraceDebugLevel(e->getIntAttribute(TraceDebugLevelParameter->getName()));
	setSaveLayers(e->getBoolAttribute(SaveLayersParameter->getName()));
	setDriftCheckPoint((DriftCheckPoint)XmlGetEnum(e, DriftCheckPointParameter->getName(), DriftCheckPointParameter->values));
	setMidiRecordMode((MidiRecordMode)XmlGetEnum(e, MidiRecordModeParameter->getName(), MidiRecordModeParameter->values));
    setDualPluginWindow(e->getBoolAttribute(DualPluginWindowParameter->getName()));
    setMidiExport(e->getBoolAttribute(MidiExportParameter->getName()));
    setHostMidiExport(e->getBoolAttribute(HostMidiExportParameter->getName()));

    setOscInputPort(e->getIntAttribute(OscInputPortParameter->getName()));
    setOscOutputPort(e->getIntAttribute(OscOutputPortParameter->getName()));
    setOscOutputHost(e->getAttribute(OscOutputHostParameter->getName()));
    setOscTrace(e->getBoolAttribute(OscTraceParameter->getName()));
    setOscEnable(e->getBoolAttribute(OscEnableParameter->getName()));

    // this isn't a parameter yet
    setNoSyncBeatRounding(e->getBoolAttribute(ATT_NO_SYNC_BEAT_ROUNDING));
    setLogStatus(e->getBoolAttribute(ATT_LOG_STATUS));

    // not an official parameter yet
    setEdpisms(e->getBoolAttribute(ATT_EDPISMS));

	setSampleRate((AudioSampleRate)XmlGetEnum(e, SampleRateParameter->getName(), SampleRateParameter->values));

    // fade frames can no longer be set high so we don't bother exposing it
	//setFadeFrames(e->getIntAttribute(FadeFramesParameter->getName()));

	for (XmlElement* child = e->getChildElement() ; child != NULL ; 
		 child = child->getNextElement()) {

		if (child->isName(EL_PRESET)) {
			Preset* p = new Preset(child);
			addPreset(p);
		}
		else if (child->isName(EL_SETUP)) {
			Setup* s = new Setup(child);
			addSetup(s);
		}
		else if (child->isName(EL_BINDING_CONFIG)) {
			BindingConfig* c = new BindingConfig(child);
			addBindingConfig(c);
		}
		else if (child->isName(EL_MIDI_CONFIG)) {
			MidiConfig* c = new MidiConfig(child);
			addMidiConfig(c);
		}
		else if (child->isName(EL_SCRIPT_CONFIG)) {
			mScriptConfig = new ScriptConfig(child);
		}
		else if (child->isName(EL_CONTROL_SURFACE)) {
			ControlSurfaceConfig* cs = new ControlSurfaceConfig(child);
			addControlSurface(cs);
		}
#ifndef HIDE_OSC        
		else if (child->isName(EL_OSC_CONFIG)) {
			setOscConfig(new OscConfig(child));
		}
#endif        
#ifndef HIDE_SAMPLES
		else if (child->isName(EL_SAMPLES)) {
			mSamples = new Samples(child);
        }
#endif
		else if (child->isName(EL_FOCUS_LOCK_FUNCTIONS) ||
                 child->isName(EL_GROUP_FUNCTIONS)) {
            // changed the name in 1.43
			StringList* functions = new StringList();
			for (XmlElement* gchild = child->getChildElement() ; 
				 gchild != NULL ; 
				 gchild = gchild->getNextElement()) {
				// assumed to be <String>xxx</String>
				const char* name = gchild->getContent();
				if (name != NULL) 
				  functions->add(name);
			}
			setFocusLockFunctions(functions);
		}
		else if (child->isName(EL_MUTE_CANCEL_FUNCTIONS)) {
			StringList* functions = new StringList();
			for (XmlElement* gchild = child->getChildElement() ; 
				 gchild != NULL ; 
				 gchild = gchild->getNextElement()) {
				// assumed to be <String>xxx</String>
				const char* name = gchild->getContent();
				if (name != NULL) 
				  functions->add(name);
			}
			setMuteCancelFunctions(functions);
		}
		else if (child->isName(EL_CONFIRMATION_FUNCTIONS)) {
			StringList* functions = new StringList();
			for (XmlElement* gchild = child->getChildElement() ; 
				 gchild != NULL ; 
				 gchild = gchild->getNextElement()) {
				// assumed to be <String>xxx</String>
				const char* name = gchild->getContent();
				if (name != NULL) 
				  functions->add(name);
			}
			setConfirmationFunctions(functions);
		}
		else if (child->isName(EL_ALT_FEEDBACK_DISABLES)) {
			StringList* controls = new StringList();
			for (XmlElement* gchild = child->getChildElement() ; 
				 gchild != NULL ; 
				 gchild = gchild->getNextElement()) {
				// assumed to be <String>xxx</String>
				const char* name = gchild->getContent();
				if (name != NULL) 
				  controls->add(name);
			}
			setAltFeedbackDisables(controls);
		}
	}

	// have to wait until these are populated
	setOverlayBindingConfig(bconfig);
    setCurrentSetup(setup);
}

char* MobiusConfig::toXml()
{
	char* xml = NULL;
	XmlBuffer* b = new XmlBuffer();
	toXml(b);
	xml = b->stealString();
	delete b;
	return xml;
}

void MobiusConfig::toXml(XmlBuffer* b)
{
	// !! this really needs to be table driven like Preset parameters

	b->addOpenStartTag(EL_CONFIG);

    b->addAttribute(ATT_LANGUAGE, mLanguage);
    b->addAttribute(MidiInputParameter->getName(), mMidiInput);
    b->addAttribute(MidiOutputParameter->getName(), mMidiOutput);
    b->addAttribute(MidiThroughParameter->getName(), mMidiThrough);
    b->addAttribute(PluginMidiInputParameter->getName(), mPluginMidiInput);
    b->addAttribute(PluginMidiOutputParameter->getName(), mPluginMidiOutput);
    b->addAttribute(PluginMidiThroughParameter->getName(), mPluginMidiThrough);
    b->addAttribute(AudioInputParameter->getName(), mAudioInput);
    b->addAttribute(AudioOutputParameter->getName(), mAudioOutput);
	b->addAttribute(ATT_UI_CONFIG, mUIConfig);
	b->addAttribute(QuickSaveParameter->getName(), mQuickSave);
	b->addAttribute(CustomMessageFileParameter->getName(), mCustomMessageFile);
	b->addAttribute(UnitTestsParameter->getName(), mUnitTests);

    b->addAttribute(NoiseFloorParameter->getName(), mNoiseFloor);
	b->addAttribute(ATT_SUGGESTED_LATENCY, mSuggestedLatency);
	b->addAttribute(InputLatencyParameter->getName(), mInputLatency);
	b->addAttribute(OutputLatencyParameter->getName(), mOutputLatency);
    // don't bother saving this until it can have a more useful range
	//b->addAttribute(FadeFramesParameter->getName(), mFadeFrames);
	b->addAttribute(MaxSyncDriftParameter->getName(), mMaxSyncDrift);
    b->addAttribute(TracksParameter->getName(), mTracks);
    b->addAttribute(TrackGroupsParameter->getName(), mTrackGroups);
    b->addAttribute(MaxLoopsParameter->getName(), mMaxLoops);
	b->addAttribute(LongPressParameter->getName(), mLongPress);
	b->addAttribute(MonitorAudioParameter->getName(), mMonitorAudio);
	b->addAttribute(ATT_PLUGIN_HOST_REWINDS, mHostRewinds);
	b->addAttribute(ATT_PLUGIN_PINS, mPluginPins);
	b->addAttribute(AutoFeedbackReductionParameter->getName(), mAutoFeedbackReduction);
    // don't allow this to be persisted any more, can only be set in scripts
	//b->addAttribute(IsolateOverdubsParameter->getName(), mIsolateOverdubs);
	b->addAttribute(IntegerWaveFileParameter->getName(), mIntegerWaveFile);
	b->addAttribute(SpreadRangeParameter->getName(), mSpreadRange);
	b->addAttribute(TracePrintLevelParameter->getName(), mTracePrintLevel);
	b->addAttribute(TraceDebugLevelParameter->getName(), mTraceDebugLevel);
	b->addAttribute(SaveLayersParameter->getName(), mSaveLayers);
	b->addAttribute(DriftCheckPointParameter->getName(), DriftCheckPointParameter->values[mDriftCheckPoint]);
	b->addAttribute(MidiRecordModeParameter->getName(), MidiRecordModeParameter->values[mMidiRecordMode]);
	b->addAttribute(DualPluginWindowParameter->getName(), mDualPluginWindow);
	b->addAttribute(MidiExportParameter->getName(), mMidiExport);
	b->addAttribute(HostMidiExportParameter->getName(), mHostMidiExport);
	b->addAttribute(GroupFocusLockParameter->getName(), mGroupFocusLock);

    b->addAttribute(ATT_NO_SYNC_BEAT_ROUNDING, mNoSyncBeatRounding);
    b->addAttribute(ATT_LOG_STATUS, mLogStatus);

	b->addAttribute(OscInputPortParameter->getName(), mOscInputPort);
	b->addAttribute(OscOutputPortParameter->getName(), mOscOutputPort);
	b->addAttribute(OscOutputHostParameter->getName(), mOscOutputHost);
    b->addAttribute(OscTraceParameter->getName(), mOscTrace);
    b->addAttribute(OscEnableParameter->getName(), mOscEnable);

	b->addAttribute(SampleRateParameter->getName(), SampleRateParameter->values[mSampleRate]);

    // The setup is all we store, if the preset has been overridden
    // this is not saved in the config.
	if (mSetup != NULL)
	  b->addAttribute(ATT_SETUP, mSetup->getName());

    BindingConfig* overlay = getOverlayBindingConfig();
	if (overlay != NULL)
	  b->addAttribute(ATT_OVERLAY_BINDINGS, mOverlayBindings->getName());

    // not an official Parameter yet
    if (mEdpisms)
      b->addAttribute(ATT_EDPISMS, "true");

	b->add(">\n");
	b->incIndent();

	if (mScriptConfig != NULL)
      mScriptConfig->toXml(b);

	for (Preset* p = mPresets ; p != NULL ; p = p->getNext())
	  p->toXml(b);

	for (Setup* s = mSetups ; s != NULL ; s = s->getNext())
	  s->toXml(b);

	for (BindingConfig* c = mBindingConfigs ; c != NULL ; c = c->getNext())
	  c->toXml(b);

    // should have cleaned these up by now
    if (mMidiConfigs != NULL) {
        Trace(1, "Still have MidiConfigs!!\n");
        for (MidiConfig* mc = mMidiConfigs ; mc != NULL ; mc = mc->getNext())
          mc->toXml(b);
    }

	for (ControlSurfaceConfig* cs = mControlSurfaces ; cs != NULL ; cs = cs->getNext())
	  cs->toXml(b);

#ifndef HIDE_SAMPLES
	if (mSamples != NULL)
	  mSamples->toXml(b);
#endif
    
	if (mFocusLockFunctions != NULL && mFocusLockFunctions->size() > 0) {
		b->addStartTag(EL_FOCUS_LOCK_FUNCTIONS, true);
		b->incIndent();
		for (int i = 0 ; i < mFocusLockFunctions->size() ; i++) {
			const char* name = mFocusLockFunctions->getString(i);
			b->addElement(EL_STRING, name);
		}
		b->decIndent();
		b->addEndTag(EL_FOCUS_LOCK_FUNCTIONS, true);
	}		

	if (mMuteCancelFunctions != NULL && mMuteCancelFunctions->size() > 0) {
		b->addStartTag(EL_MUTE_CANCEL_FUNCTIONS, true);
		b->incIndent();
		for (int i = 0 ; i < mMuteCancelFunctions->size() ; i++) {
			const char* name = mMuteCancelFunctions->getString(i);
			b->addElement(EL_STRING, name);
		}
		b->decIndent();
		b->addEndTag(EL_MUTE_CANCEL_FUNCTIONS, true);
	}		

	if (mConfirmationFunctions != NULL && mConfirmationFunctions->size() > 0) {
		b->addStartTag(EL_CONFIRMATION_FUNCTIONS, true);
		b->incIndent();
		for (int i = 0 ; i < mConfirmationFunctions->size() ; i++) {
			const char* name = mConfirmationFunctions->getString(i);
			b->addElement(EL_STRING, name);
		}
		b->decIndent();
		b->addEndTag(EL_CONFIRMATION_FUNCTIONS, true);
	}		

	if (mAltFeedbackDisables != NULL && mAltFeedbackDisables->size() > 0) {
		b->addStartTag(EL_ALT_FEEDBACK_DISABLES, true);
		b->incIndent();
		for (int i = 0 ; i < mAltFeedbackDisables->size() ; i++) {
			const char* name = mAltFeedbackDisables->getString(i);
			b->addElement(EL_STRING, name);
		}
		b->decIndent();
		b->addEndTag(EL_ALT_FEEDBACK_DISABLES, true);
	}		

	b->decIndent();

	b->addEndTag(EL_CONFIG);
}

/****************************************************************************
 *                                                                          *
 *                               SCRIPT CONFIG                              *
 *                                                                          *
 ****************************************************************************/

ScriptConfig::ScriptConfig()
{
    mScripts = NULL;
}

ScriptConfig::ScriptConfig(XmlElement* e)
{
    mScripts = NULL;
    parseXml(e);
}

ScriptConfig::~ScriptConfig()
{
	clear();
}

/**
 * Clone for difference detection.
 * All we really need are the original file names.
 */
ScriptConfig* ScriptConfig::clone()
{
    ScriptConfig* clone = new ScriptConfig();
    for (ScriptRef* s = mScripts ; s != NULL ; s = s->getNext()) {
        ScriptRef* s2 = new ScriptRef(s);
        clone->add(s2);
    }
    return clone;
}

void ScriptConfig::clear()
{
    ScriptRef* ref = NULL;
    ScriptRef* next = NULL;
    for (ref = mScripts ; ref != NULL ; ref = next) {
        next = ref->getNext();
        delete ref;
    }
	mScripts = NULL;
}

ScriptRef* ScriptConfig::getScripts()
{
    return mScripts;
}

void ScriptConfig::setScripts(ScriptRef* refs) 
{
	clear();
	mScripts = refs;
}

void ScriptConfig::add(ScriptRef* neu) 
{
    ScriptRef* last = NULL;
    for (last = mScripts ; last != NULL && last->getNext() != NULL ; 
         last = last->getNext());

	if (last == NULL)
	  mScripts = neu;
	else
	  last->setNext(neu);
}

void ScriptConfig::add(const char* file) 
{
	add(new ScriptRef(file));
}

void ScriptConfig::toXml(XmlBuffer* b)
{
    b->addStartTag(EL_SCRIPT_CONFIG);
    b->incIndent();

    for (ScriptRef* ref = mScripts ; ref != NULL ; ref = ref->getNext())
      ref->toXml(b);

    b->decIndent();
    b->addEndTag(EL_SCRIPT_CONFIG);
}

void ScriptConfig::parseXml(XmlElement* e)
{
    ScriptRef* last = NULL;
    for (ScriptRef* ref = mScripts ; ref != NULL && ref->getNext() != NULL ; 
         ref = ref->getNext());

    for (XmlElement* child = e->getChildElement() ; child != NULL ; 
         child = child->getNextElement()) {
        ScriptRef* ref = new ScriptRef(child);
        if (last == NULL)
          mScripts = ref;   
        else
          last->setNext(ref);
        last = ref;
    }
}

/**
 * Utility for difference detection.
 */
bool ScriptConfig::isDifference(ScriptConfig* other)
{
    bool difference = false;

    int myCount = 0;
    for (ScriptRef* s = mScripts ; s != NULL ; s = s->getNext())
      myCount++;

    int otherCount = 0;
    if (other != NULL) {
        for (ScriptRef* s = other->getScripts() ; s != NULL ; s = s->getNext())
          otherCount++;
    }

    if (myCount != otherCount) {
        difference = true;
    }
    else {
        for (ScriptRef* s = mScripts ; s != NULL ; s = s->getNext()) {
            ScriptRef* ref = other->get(s->getFile());
            if (ref == NULL) {
                difference = true;
                break;
            }
        }
    }
    return difference;
}

ScriptRef* ScriptConfig::get(const char* file)
{
    ScriptRef* found = NULL;

    for (ScriptRef* s = mScripts ; s != NULL ; s = s->getNext()) {
        if (StringEqual(s->getFile(), file)) {
            found = s;
            break;
        }
    }
    return found;
}

//////////////////////////////////////////////////////////////////////
//
// ScriptRef
//
//////////////////////////////////////////////////////////////////////

ScriptRef::ScriptRef()
{
    init();
}

ScriptRef::ScriptRef(XmlElement* e)
{
    init();
    parseXml(e);
}

ScriptRef::ScriptRef(const char* file)
{
    init();
    setFile(file);
}

ScriptRef::ScriptRef(ScriptRef* src)
{
    init();
    setFile(src->getFile());
}

void ScriptRef::init()
{
    mNext = NULL;
    mFile = NULL;
}

ScriptRef::~ScriptRef()
{
	delete mFile;
}

void ScriptRef::setNext(ScriptRef* ref)
{
    mNext = ref;
}

ScriptRef* ScriptRef::getNext()
{
    return mNext;
}

void ScriptRef::setFile(const char* file)
{
    delete mFile;
    mFile = CopyString(file);
}

const char* ScriptRef::getFile()
{
    return mFile;
}

void ScriptRef::toXml(XmlBuffer* b)
{
    b->addOpenStartTag(EL_SCRIPT_REF);
    b->addAttribute(ATT_FILE, mFile);
    b->add("/>\n");
}

void ScriptRef::parseXml(XmlElement* e)
{
    setFile(e->getAttribute(ATT_FILE));
}

//////////////////////////////////////////////////////////////////////
//
// Control Surface
//
//////////////////////////////////////////////////////////////////////

ControlSurfaceConfig::ControlSurfaceConfig()
{
	init();
}

ControlSurfaceConfig::ControlSurfaceConfig(XmlElement* e)
{
	init();
	parseXml(e);
}

void ControlSurfaceConfig::init()
{
	mNext = NULL;
	mName = NULL;
}

ControlSurfaceConfig::~ControlSurfaceConfig()
{
	delete mName;

    ControlSurfaceConfig* el;
    ControlSurfaceConfig* next = NULL;

    for (el = mNext ; el != NULL ; el = next) {
        next = el->getNext();
		el->setNext(NULL);
        delete el;
    }
}

ControlSurfaceConfig* ControlSurfaceConfig::getNext()
{
	return mNext;
}

void ControlSurfaceConfig::setNext(ControlSurfaceConfig* cs)
{
	mNext = cs;
}

const char* ControlSurfaceConfig::getName()
{
	return mName;
}

void ControlSurfaceConfig::setName(const char* s)
{
	delete mName;
	mName = CopyString(s);
}

void ControlSurfaceConfig::parseXml(XmlElement* e)
{
    setName(e->getAttribute(ATT_NAME));
}

void ControlSurfaceConfig::toXml(XmlBuffer* b)
{
    b->addOpenStartTag(EL_CONTROL_SURFACE);
    b->addAttribute(ATT_NAME, mName);
    b->add("/>\n");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

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
	mLanguage = nullptr;
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
	mPreset = nullptr;
	mSetups = nullptr;
	mSetup = nullptr;
	mBindingConfigs = nullptr;
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

	delete mLanguage;
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
    delete mSetups;
    delete mBindingConfigs;
    delete mScriptConfig;
	delete mOscConfig;
	delete mSampleConfig;
}

bool MobiusConfig::isDefault()
{
    return mDefault;
}

// will need this eventually
#if 0
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
#endif

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

/**
 * Number the presets, setups, or binding configs after editing.
 */
void MobiusConfig::numberThings(Bindable* things)
{
	int count = 0;
	for (Bindable* b = things ; b != nullptr ; b = b->getNextBindable())
	  b->setNumber(count++);
}

int MobiusConfig::countThings(Bindable* things)
{
    int count = 0;
    for (Bindable* b = things ; b != nullptr ; b = b->getNextBindable())
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

/**
 * Ensure that all of the presets and midi configs have names.
 * Necessary so they can be identified in a GUI.
 */
void MobiusConfig::generateNames()
{
    generateNames(mPresets, "Preset", nullptr);
    generateNames(mSetups, "Setup", nullptr);
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

	for (Bindable* b = bindables ; b != nullptr ; b = b->getNextBindable()) {
        if (baseName && b == bindables) {
            // force the name of the first one
            if (!StringEqual(baseName, b->getName()))
              b->setName(baseName);
        }
        else if (b->getName() == nullptr) {
            Bindable* existing;
            do {
                // search for name in use
                existing = nullptr;
                sprintf(buf, "%s %d", prefix, count);
                for (Bindable* b2 = bindables ; b2 != nullptr ; 
                     b2 = b2->getNextBindable()) {
                    if (StringEqual(buf, b2->getName())) {
                        existing = b2;
                        break;
                    }
                }
                if (existing != nullptr)
                  count++;
            } while (existing != nullptr);

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
	for (prev = mPresets ; prev != nullptr && prev->getNext() != nullptr ; 
		 prev = prev->getNext());

	if (prev == nullptr)
	  mPresets = p;
	else
	  prev->setNext(p);

    if (mPreset == nullptr)
      mPreset = p;

	numberThings(mPresets);
}

/**
 * Note that this should only be called on a cloned MobiusConfig that
 * the interrupt handler can't be using.
 */
void MobiusConfig::removePreset(Preset* preset) 
{
	Preset* prev = nullptr;
	for (Preset* p = mPresets ; p != nullptr ; p = p->getNext()) {
		if (p != preset)
		  prev = p;
		else {
			if (prev == nullptr)
			  mPresets = p->getNext();
			else 
			  prev->setNext(p->getNext());
			p->setNext(nullptr);

			if (p == mPreset)
			  mPreset = mPresets;
		}
	}
	numberThings(mPresets);
}

Preset* MobiusConfig::getPreset(const char* name)
{
	Preset* found = nullptr;
	if (name != nullptr) {
		for (Preset* p = mPresets ; p != nullptr ; p = p->getNext()) {
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
    Preset* found = nullptr;
    int i = 0;

    for (Preset* p = mPresets ; p != nullptr ; p = p->getNext(), i++) {
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
    if (mPresets == nullptr)
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
	if (mPreset == nullptr) {
		if (mPresets == nullptr)
		  mPresets = new Preset("Default");
		mPreset = mPresets;
	}
	return mPreset;
}

int MobiusConfig::getCurrentPresetIndex()
{
    int index = 0;
    int i = 0;

	if (mPreset == nullptr)
	  mPreset = mPresets;

    // don't need to do it this way if we can assume they're numbered!?
    for (Preset* p = mPresets ; p != nullptr ; p = p->getNext(), i++) {
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
    if (p != nullptr) 
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
	for (prev = mSetups ; prev != nullptr && prev->getNext() != nullptr ; 
		 prev = prev->getNext());

	if (prev == nullptr)
	  mSetups = p;
	else
	  prev->setNext(p);

    if (mSetup == nullptr)
      mSetup = p;

    numberThings(mSetups);
}

/**
 * Note that this should only be called on a cloned MobiusConfig that
 * the interrupt handler can't be using.
 */
void MobiusConfig::removeSetup(Setup* preset) 
{
	Setup* prev = nullptr;
	for (Setup* p = mSetups ; p != nullptr ; p = p->getNext()) {
		if (p != preset)
		  prev = p;
		else {
			if (prev == nullptr)
			  mSetups = p->getNext();
			else 
			  prev->setNext(p->getNext());
			p->setNext(nullptr);

			if (p == mSetup)
			  mSetup = mSetups;
		}
	}
    numberThings(mSetups);
}

Setup* MobiusConfig::getSetup(const char* name)
{
	Setup* found = nullptr;
	if (name != nullptr) {
		for (Setup* p = mSetups ; p != nullptr ; p = p->getNext()) {
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
    Setup* found = nullptr;
    int i = 0;

    for (Setup* p = mSetups ; p != nullptr ; p = p->getNext(), i++) {
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
	if (mSetup == nullptr) {
		if (mSetups == nullptr)
		  mSetups = new Setup();
		mSetup = mSetups;
	}
	return mSetup;
}

int MobiusConfig::getCurrentSetupIndex()
{
    int index = 0;
    int i = 0;

	if (mSetup == nullptr)
	  mSetup = mSetups;

    for (Setup* p = mSetups ; p != nullptr ; p = p->getNext(), i++) {
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
	if (p != nullptr) {
		// these should be the same object, but make sure
 Setup* cur = getSetup(p->getName());
		if (cur != nullptr) 
		  mSetup= cur;
	}
}

Setup* MobiusConfig::setCurrentSetup(int index)
{
    Setup* p = getSetup(index);
    if (p != nullptr) 
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
	for (prev = mBindingConfigs ; prev != nullptr && prev->getNext() != nullptr ; 
		 prev = prev->getNext());
	if (prev == nullptr)
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
	BindingConfig* prev = nullptr;
	for (BindingConfig* p = mBindingConfigs ; p != nullptr ; p = p->getNext()) {
		if (p != config)
		  prev = p;
		else {
			if (prev == nullptr) {
                // UI should have prevented this
                Trace(1, "Removing base BindingConfig!!\n");
                mBindingConfigs = p->getNext();
            }
			else 
			  prev->setNext(p->getNext());

			p->setNext(nullptr);

			if (p == mOverlayBindings)
			  mOverlayBindings = nullptr;
		}
	}
    numberThings(mBindingConfigs);
}

BindingConfig* MobiusConfig::getBindingConfig(const char* name)
{
	BindingConfig* found = nullptr;
    if (name == nullptr) {
        // always the base config
        found = mBindingConfigs;
    }
    else {
		for (BindingConfig* p = mBindingConfigs ; p != nullptr ; p = p->getNext()) {
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
    BindingConfig* found = nullptr;
    int i = 0;

    for (BindingConfig* c = mBindingConfigs ; c != nullptr ; c = c->getNext(), i++) {
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
    if (mBindingConfigs == nullptr)
      mBindingConfigs = new BindingConfig();
	return mBindingConfigs;
}

BindingConfig* MobiusConfig::getOverlayBindingConfig()
{
    // it is important this self-heal if it got corrupted
    if (mOverlayBindings == mBindingConfigs)
      mOverlayBindings = nullptr;
	return mOverlayBindings;
}

int MobiusConfig::getOverlayBindingConfigIndex()
{
    BindingConfig* overlay = getOverlayBindingConfig();

    int index = 0;
    int i = 0;
    for (BindingConfig* b = mBindingConfigs ; b != nullptr ; 
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
      mOverlayBindings = nullptr;
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
    if (b != nullptr)
      setOverlayBindingConfig(b);

    return mOverlayBindings;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

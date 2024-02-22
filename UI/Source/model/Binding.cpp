/*
 * Copyright (c) 2010 Jeffrey S. Larson  <jeff@circularlabs.com>
 * All rights reserved.
 * See the LICENSE file for the full copyright and license declaration.
 * 
 * ---------------------------------------------------------------------
 * 
 * Model for binding triggers to functions, controls, parameters,
 * and configuration objects within Mobius.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include <vector>

#include "../util/Util.h"
#include "../util/Trace.h"
#include "../util/MidiUtil.h"  // MidiNoteName
#include "../util/KeyCode.h"   // GetKeyString

#include "Binding.h"

/****************************************************************************
 *                                                                          *
 *   							   BINDABLE                                 *
 *                                                                          *
 ****************************************************************************/

#define ATT_NAME "name"
#define ATT_NUMBER "number"

Bindable::Bindable()
{
	mNumber	= 0;
	mName	= nullptr;
}

Bindable::~Bindable()
{
	delete mName;
}

void Bindable::setNumber(int i)
{
	mNumber = i;
}

int Bindable::getNumber()
{
	return mNumber;
}

void Bindable::setName(const char* s)
{
	delete mName;
	mName = CopyString(s);
}

const char* Bindable::getName()
{
	return mName;
}

void Bindable::clone(Bindable* src)
{
	setName(src->mName);
	mNumber = src->mNumber;
}

/****************************************************************************
 *                                                                          *
 *   							   TRIGGERS                                 *
 *                                                                          *
 ****************************************************************************/

Trigger::Trigger(const char* name, const char* display, bool isBindable) :
    SystemConstant(name, display)
{
    Triggers.push_back(this);
    bindable = isBindable;
}

/**
 * Lookup a bindable trigger by name.
 */
Trigger* Trigger::getBindable(const char* name) 
{
	Trigger* found = nullptr;
	if (name != nullptr) {
		for (int i = 0 ; Triggers.size() ; i++) {
			Trigger* t = Triggers[i];
			if (t->bindable && !strcmp(t->getName(), name)) {
				found = t;
				break;
			}
		}
	}
	return found;
}

std::vector<Trigger*> Trigger::Triggers;

// unlike Parameter we don't have subclasses so can just extern
// the Trigger object
// everything really wants to deal with a pointer to them and I don't want
// to mess with reference conversion right now

Trigger TriggerKeyObj("key", "Key", true);
Trigger* TriggerKey = &TriggerKeyObj;

Trigger TriggerMidiObj("midi", "MIDI", false);
Trigger* TriggerMidi = &TriggerMidiObj;

Trigger TriggerNoteObj("note", "Note", true);
Trigger* TriggerNote = &TriggerNoteObj;

Trigger TriggerProgramObj("program", "Program", true);
Trigger* TriggerProgram = &TriggerProgramObj;

Trigger TriggerControlObj("control", "Control", true);
Trigger* TriggerControl = &TriggerControlObj;

Trigger TriggerPitchObj("pitch", "Pitch Bend", true);
Trigger* TriggerPitch = &TriggerPitchObj;

Trigger TriggerHostObj("host", "Host", true);
Trigger* TriggerHost = &TriggerHostObj;

Trigger TriggerOscObj("osc", "OSC", false);
Trigger* TriggerOsc = &TriggerOscObj;

Trigger TriggerUIObj("ui", "UI", true);
Trigger* TriggerUI = &TriggerUIObj;

Trigger TriggerScriptObj("script", "Script", false);
Trigger* TriggerScript = &TriggerScriptObj;

Trigger TriggerAlertObj("alert", "Alert", false);
Trigger* TriggerAlert = &TriggerAlertObj;

Trigger TriggerEventObj("event", "Event", false);
Trigger* TriggerEvent = &TriggerEventObj;

Trigger TriggerThreadObj("thread", "Mobius Thread", false);
Trigger* TriggerThread = &TriggerThreadObj;

Trigger TriggerUnknownObj("unknown", "unknown", false);
Trigger* TriggerUnknown = &TriggerUnknownObj;

/****************************************************************************
 *                                                                          *
 *                               TRIGGER MODES                              *
 *                                                                          *
 ****************************************************************************/

std::vector<TriggerMode*> TriggerMode::TriggerModes;

TriggerMode TriggerModeContinuousObj("continuous", "Continuous");
TriggerMode* TriggerModeContinuous = &TriggerModeContinuousObj;

TriggerMode TriggerModeOnceObj("once", "Once");
TriggerMode* TriggerModeOnce = &TriggerModeOnceObj;

TriggerMode TriggerModeMomentaryObj("momentary", "Momentary");
TriggerMode* TriggerModeMomentary = &TriggerModeMomentaryObj;

TriggerMode TriggerModeToggleObj("toggle", "Toggle");
TriggerMode* TriggerModeToggle = &TriggerModeToggleObj;

TriggerMode TriggerModeXYObj("xy", "X,Y");
TriggerMode* TriggerModeXY = &TriggerModeXYObj;


TriggerMode::TriggerMode(const char* name, const char* display) :
    SystemConstant(name, display)
{
    TriggerModes.push_back(this);
}

TriggerMode* TriggerMode::get(const char* name) 
{
	TriggerMode* found = nullptr;
	if (name != nullptr) {
		for (int i = 0 ; i < TriggerModes.size() ; i++) {
			TriggerMode* t = TriggerModes[i];
			if (!strcmp(t->getName(), name)) {
				found = t;
				break;
			}
		}
	}
	return found;
}

/****************************************************************************
 *                                                                          *
 *   							   TARGETS                                  *
 *                                                                          *
 ****************************************************************************/

std::vector<Target*> Target::Targets;

Target TargetFunctionObj("function", "Function", true);
Target* TargetFunction = &TargetFunctionObj;

Target TargetParameterObj("parameter", "Parameter", true);
Target* TargetParameter = &TargetParameterObj;

Target TargetSetupObj("setup", "Setup", true);
Target* TargetSetup = &TargetSetupObj;

Target TargetPresetObj("preset", "Preset", true);
Target* TargetPreset = &TargetPresetObj;

Target TargetBindingsObj("bindings", "Bindings", true);
Target* TargetBindings = &TargetBindingsObj;

//Target TargetUIControlObj("uiControl", "UI Control", true);
//Target* TargetUIControl = &TargetUIControlObj;

Target TargetUIConfigObj("uiConfig", "UI Config", true);
Target* TargetUIConfig = &TargetUIConfigObj;

// this is for internal use, can't be used in bindings
Target TargetScriptObj("script", "Script", false);
Target* TargetScript = &TargetScriptObj;

Target::Target(const char* name, const char* display, bool argBindable) :
    SystemConstant(name, display)
{
    Targets.push_back(this);
    bindable = argBindable;
}

Target* Target::getBindable(const char* name) 
{
	Target* found = nullptr;

    // auto upgrade old bindings
    if (StringEqual(name, "control"))
      name = "parameter";

	if (name != nullptr) {
		for (int i = 0 ; i < Targets.size() ; i++) {
			Target* t = Targets[i];
			if (t->bindable && !strcmp(t->getName(), name)) {
				found = t;
				break;
			}
		}
	}
	return found;
}

/****************************************************************************
 *                                                                          *
 *                                 UI CONTROL                               *
 *                                                                          *
 ****************************************************************************/

#if 0
UIControl::UIControl()
{
    init();
}

UIControl::UIControl(const char* name, int key) :
    SystemConstant(name, key)
{
    init();
}

void UIControl::init() {
}
#endif

/****************************************************************************
 *                                                                          *
 *                                UI PARAMETER                              *
 *                                                                          *
 ****************************************************************************/

UIParameter::UIParameter(const char* name, int key) :
    SystemConstant(name, key)
{
}

/****************************************************************************
 *                                                                          *
 *   							   BINDING                                  *
 *                                                                          *
 ****************************************************************************/

void Binding::init()
{
	mNext = nullptr;

	// trigger
	mTrigger = nullptr;
    mTriggerMode = nullptr;
    mTriggerPath = nullptr;
	mValue = 0;
	mChannel = 0;

	// target
    mTargetPath = nullptr;
	mTarget = nullptr;
	mName = nullptr;

	// scope
    mScope = nullptr;
	mTrack = 0;
	mGroup = 0;

    // arguments
	mArgs = nullptr;
}

Binding::Binding()
{
	init();
}

/**
 * Hacked this up for BindingTable, make sure it's complete
 */
Binding::Binding(Binding* src)
{
    init();
    
    // trigger
    setTrigger(src->getTrigger());
    setTriggerMode(src->getTriggerMode());
    setValue(src->getValue());
    setChannel(src->getChannel());
    // target
    setTarget(src->getTarget());
    setName(src->getName());
    setArgs(src->getArgs());
    // todo: triggerPath for OSC

    // scope
    // need more for track/group scopes?
    setScope(src->getScope());
}

Binding::~Binding()
{
	Binding *el, *next;

    delete mTriggerPath;
    delete mTargetPath;
	delete mName;
    delete mScope;
	delete mArgs;

	for (el = mNext ; el != nullptr ; el = next) {
		next = el->getNext();
		el->setNext(nullptr);
		delete el;
	}

}

void Binding::setNext(Binding* c)
{
	mNext = c;
}

Binding* Binding::getNext()
{
	return mNext;
}

//
// Trigger
//

void Binding::setTrigger(Trigger* t) 
{
	mTrigger = t;
}

Trigger* Binding::getTrigger()
{
	return mTrigger;
}

void Binding::setValue(int v) 
{
	mValue = v;
}

int Binding::getValue()
{
	return mValue;
}

void Binding::setChannel(int c) 
{
	mChannel = c;
}

int Binding::getChannel()
{
	return mChannel;
}

bool Binding::isMidi()
{
	return (mTrigger == TriggerNote ||
			mTrigger == TriggerProgram ||
			mTrigger == TriggerControl ||
			mTrigger == TriggerPitch);
}

void Binding::setTriggerPath(const char* s)
{
    delete mTriggerPath;
    mTriggerPath = CopyString(s);
}

const char* Binding::getTriggerPath()
{
    return mTriggerPath;
}

void Binding::setTriggerMode(TriggerMode* t)
{
    mTriggerMode = t;
}

TriggerMode* Binding::getTriggerMode()
{
    return mTriggerMode;
}

//
// Target
//

void Binding::setTargetPath(const char* s)
{
    delete mTargetPath;
    mTargetPath = CopyString(s);
}

const char* Binding::getTargetPath()
{
    return mTargetPath;
}

void Binding::setTarget(Target* t) 
{
	mTarget = t;
}

Target* Binding::getTarget()
{
	return mTarget;
}

void Binding::setName(const char *name) 
{
	delete mName;
	mName = CopyString(name);
}

const char* Binding::getName()
{
	return mName;
}

// 
// Scope
//

void Binding::setScope(const char* s)
{
    delete mScope;
    mScope = CopyString(s);
    parseScope();
}

const char* Binding::getScope() 
{
    return mScope;
}

/**
 * Parse a scope into track an group numbers.
 * Tracks are expected to be identified with integers starting
 * from 1.  Groups are identified with upper case letters A-Z.
 */
void Binding::parseScope()
{
    mTrack = 0;
    mGroup = 0;

    if (mScope != nullptr) {
        int len = strlen(mScope);
        if (len > 1) {
            // must be a number 
            mTrack = atoi(mScope);
        }
        else if (len == 1) {
            char ch = mScope[0];
            if (ch >= 'A') {
                mGroup = (ch - 'A') + 1;
            }
            else {
                // normally an integer, anything else
                // collapses to zero
                mTrack = atoi(mScope);
            }
        }
    }
}

void Binding::setTrack(int t) 
{
    if (t > 0) {
        char buffer[32];
        sprintf(buffer, "%d", t);
        setScope(buffer);
    }
}

int Binding::getTrack()
{
	return mTrack;
}

void Binding::setGroup(int t) 
{
    if (t > 0) {
        char buffer[32];
        sprintf(buffer, "%c", (char)('A' + (t - 1)));
        setScope(buffer);
    }
}

int Binding::getGroup()
{
	return mGroup;
}

//
// Arguments
//

void Binding::setArgs(const char* args) 
{
	delete mArgs;
	mArgs = CopyString(args);
}

const char* Binding::getArgs() 
{
	return mArgs;
}

//
// Utilities
//

void Binding::getSummary(char* buffer)
{
	strcpy(buffer, "");

	// we display channel consistenly everywhere as 1-16
	int channel = mChannel + 1;

	if (mTrigger == TriggerNote) {
		char note[128];
		MidiNoteName(mValue, note);
		sprintf(buffer, "%d:%s", channel, note);
	}
	else if (mTrigger == TriggerProgram) {
		sprintf(buffer, "%d:Program %d", channel, mValue);
	}
	else if (mTrigger == TriggerControl) {
		sprintf(buffer, "%d:Control %d", channel, mValue);
	}
	else if (mTrigger == TriggerKey) {
		// UI should actually overload this with a smarter key 
		// rendering utility
		sprintf(buffer, "Key %d", mValue);
	}
    else if (mTrigger == TriggerOsc) {
        sprintf(buffer, "OSC %s", mTriggerPath);
    }

}

void Binding::getMidiString(char* buffer, bool includeChannel)
{
	strcpy(buffer, "");

	// we display channel consistenly everywhere as 1-16
	int channel = mChannel + 1;

	if (mTrigger == TriggerControl) {
		int value = getValue();
		if (value >= 0 && value < 128)
		  sprintf(buffer, "%d:Control %d", channel, value);
	}
	else if (mTrigger == TriggerNote) {
		char note[128];
		int value = getValue();
		if (value >= 0 && value < 128) {
			MidiNoteName(value, note);
			sprintf(buffer, "%d:%s", channel, note);
		}
	}
	else if (mTrigger == TriggerProgram) {
		int value = getValue();
		if (value >= 0 && value < 128)
		  sprintf(buffer, "%d:Program %d", channel, value);
	}
}


/**
 * Render a TriggerKey value as a readable string.
 */
void Binding::getKeyString(char* buffer, int max)
{
    strcpy(buffer, "");

    if (mValue == 0) {
        // this can't be bound  
    }
    else {
        GetKeyString(mValue, buffer);
        if (strlen(buffer) == 0)
          sprintf(buffer, "%d", mValue);
    }
}

/****************************************************************************
 *                                                                          *
 *   							 BINDING XML                                *
 *                                                                          *
 ****************************************************************************/

#define EL_BINDING "Binding"
#define ATT_DISPLAY_NAME "displayName"
#define ATT_TRIGGER "trigger"
#define ATT_VALUE "value"
#define ATT_CHANNEL "channel"
#define ATT_TRIGGER_VALUE "triggerValue"
#define ATT_TRIGGER_PATH "triggerPath"
#define ATT_TRIGGER_TYPE "triggerType"
#define ATT_TARGET_PATH "targetPath"
#define ATT_TARGET "target"
#define ATT_ARGS "args"
#define ATT_SCOPE "scope"
#define ATT_TRACK "track"
#define ATT_GROUP "group"

/**
 * Check to see if this object represents a valid binding.
 * Used during serialization to filter partially constructed bindings
 * that were created by the dialog.
 */
bool Binding::isValid()
{
	bool valid = false;

	if (mTrigger != nullptr && mTarget != nullptr && mName != nullptr) {
		if (mTrigger == TriggerKey) {
			// key must have a non-zero value
			valid = (mValue > 0);
		}
		else if (mTrigger == TriggerNote ||
				 mTrigger == TriggerProgram ||
				 mTrigger == TriggerControl) {

			// hmm, zero is a valid value so no way to detect if
			// they didn't enter anything unless the UI uses negative
			// must have a midi status
			valid = (mValue >= 0);
		}
        else if (mTrigger == TriggerPitch) {
            // doesn't need a value
            valid = true;
        }
		else if (mTrigger == TriggerHost) {
			valid = true;
		}
        else if (mTrigger == TriggerOsc) {
            // anythign?
            valid = true;
        }
        else if (mTrigger == TriggerUI) {
            valid = true;
        }
		else {
			// not sure about mouse, wheel yet
		}
	}

	return valid;
}

/****************************************************************************
 *                                                                          *
 *   							BINDING CONFIG                              *
 *                                                                          *
 ****************************************************************************/

BindingConfig::BindingConfig()
{
	init();
}

void BindingConfig::init()
{
	mNext = nullptr;
	mName = nullptr;
	mBindings = nullptr;
}

BindingConfig::~BindingConfig()
{
	BindingConfig *el, *next;

	delete mBindings;

	for (el = mNext ; el != nullptr ; el = next) {
		next = el->getNext();
		el->setNext(nullptr);
		delete el;
	}
}

Target* BindingConfig::getTarget()
{
	return TargetBindings;
}

void BindingConfig::setNext(BindingConfig* c)
{
	mNext = c;
}

BindingConfig* BindingConfig::getNext()
{
	return mNext;
}

Bindable* BindingConfig::getNextBindable()
{
	return mNext;
}

Binding* BindingConfig::getBindings()
{
	return mBindings;
}

void BindingConfig::setBindings(Binding* b)
{
	mBindings = b;
}

void BindingConfig::addBinding(Binding* b) 
{
    if (b != nullptr) {
        // keep them ordered
        Binding *prev;
        for (prev = mBindings ; prev != nullptr && prev->getNext() != nullptr ; 
             prev = prev->getNext());
        if (prev == nullptr)
          mBindings = b;
        else
          prev->setNext(b);
    }
}

void BindingConfig::removeBinding(Binding* b)
{
    if (b != nullptr) {
        Binding *prev = nullptr;
        Binding* el = mBindings;
    
        for ( ; el != nullptr && el != b ; el = el->getNext())
          prev = el;

        if (el == b) {
            if (prev == nullptr)
              mBindings = b->getNext();
            else
              prev->setNext(b->getNext());
        }
        else {
            // not on the list, should we still nullptr out the next pointer?
            Trace(1, "BindingConfig::removeBinding binding not found!\n");
        }

        b->setNext(nullptr);
    }
}

/**
 * Search for a binding for a given trigger and value.
 * This is intended for upgrading old KeyBinding objects, once that
 * has passed we can delete this.
 */
Binding* BindingConfig::getBinding(Trigger* trigger, int value)
{
	Binding* found = nullptr;

	for (Binding* b = mBindings ; b != nullptr ; b = b->getNext()) {
		if (b->getTrigger() == trigger && b->getValue() == value) {
            found = b;
            break;
		}
	}

	return found;
}

// need to support this eventually
#if 0
BindingConfig* BindingConfig::clone()
{
	BindingConfig* clone = new BindingConfig();

	XmlBuffer* b = new XmlBuffer();
	toXml(b);
	char* xml = b->stealString();
	delete b;
	XomParser* p = new XomParser();
	XmlDocument* d = p->parse(xml);
	if (d != nullptr) {
		XmlElement* e = d->getChildElement();
		clone = new BindingConfig(e);
		delete d;
	}
    else {
        // must have been a parser error, not supposed
        // to happen
        Trace(1, "Parse error while cloning BindingConfig!!\n");
    }
	delete p;
	delete xml;

	return clone;
}
#endif

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

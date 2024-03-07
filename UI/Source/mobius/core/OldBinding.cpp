
// for atoi
#include <stdlib.h>

#include "../../util/Util.h"
#include "../../util/KeyCode.h"
#include "MidiUtil.h"
#include "OldBinding.h"

/****************************************************************************
 *                                                                          *
 *   							   BINDABLE                                 *
 *                                                                          *
 ****************************************************************************/

OldBindable::OldBindable()
{
	mNumber	= 0;
	mName	= NULL;
}

OldBindable::~OldBindable()
{
	delete mName;
}

void OldBindable::setNumber(int i)
{
	mNumber = i;
}

int OldBindable::getNumber()
{
	return mNumber;
}

void OldBindable::setName(const char* s)
{
	delete mName;
	mName = CopyString(s);
}

const char* OldBindable::getName()
{
	return mName;
}

void OldBindable::clone(OldBindable* src)
{
	setName(src->mName);
	mNumber = src->mNumber;
}

/****************************************************************************
 *                                                                          *
 *   							   TRIGGERS                                 *
 *                                                                          *
 ****************************************************************************/

OldTrigger* OldTriggerKey = new OldTrigger("key", "Key", true);
OldTrigger* OldTriggerMidi = new OldTrigger("midi", "MIDI", false);
OldTrigger* OldTriggerNote = new OldTrigger("note", "Note", true);
OldTrigger* OldTriggerProgram = new OldTrigger("program", "Program", true);
OldTrigger* OldTriggerControl = new OldTrigger("control", "Control", true);
OldTrigger* OldTriggerPitch = new OldTrigger("pitch", "Pitch Bend", true);
OldTrigger* OldTriggerHost = new OldTrigger("host", "Host", true);
OldTrigger* OldTriggerOsc = new OldTrigger("osc", "OSC", false);
OldTrigger* OldTriggerUI = new OldTrigger("ui", "UI", true);
OldTrigger* OldTriggerScript = new OldTrigger("script", "Script", false);
OldTrigger* OldTriggerAlert = new OldTrigger("alert", "Alert", false);
OldTrigger* OldTriggerEvent = new OldTrigger("event", "Event", false);
OldTrigger* OldTriggerThread = new OldTrigger("thread", "Mobius Thread", false);
OldTrigger* OldTriggerUnknown = new OldTrigger("unknown", "unknown", false);

/**
 * Array of all triggers for resolving references in XML.
 * Only bindable triggers should be here
 */
OldTrigger* OldTriggers[] = {
	OldTriggerKey,
	OldTriggerNote,
	OldTriggerProgram,
	OldTriggerControl,
	OldTriggerPitch,
	OldTriggerHost,
	OldTriggerOsc,
    OldTriggerUI,
	NULL
};

OldTrigger::OldTrigger(const char* name, const char* display, bool bindable) :
    SystemConstant(name, display)
{
    mBindable = bindable;
}

bool OldTrigger::isBindable()
{
    return mBindable;
}

/**
 * Lookup a bindable trigger by name.
 */
OldTrigger* OldTrigger::get(const char* name) 
{
	OldTrigger* found = NULL;
	if (name != NULL) {
		for (int i = 0 ; OldTriggers[i] != NULL ; i++) {
			OldTrigger* t = OldTriggers[i];
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
 *                               TRIGGER MODES                              *
 *                                                                          *
 ****************************************************************************/

OldTriggerMode* OldTriggerModeContinuous = new OldTriggerMode("continuous", "Continuous");
OldTriggerMode* OldTriggerModeOnce = new OldTriggerMode("once", "Once");
OldTriggerMode* OldTriggerModeMomentary = new OldTriggerMode("momentary", "Momentary");
OldTriggerMode* OldTriggerModeToggle = new OldTriggerMode("toggle", "Toggle");
OldTriggerMode* OldTriggerModeXY = new OldTriggerMode("xy", "X,Y");

/**
 * Array of all triggers for resolving references in XML.
 */
OldTriggerMode* OldTriggerModes[] = {
    OldTriggerModeContinuous,
    OldTriggerModeOnce,
    OldTriggerModeMomentary,
    OldTriggerModeToggle,
    OldTriggerModeXY,
	NULL
};

OldTriggerMode::OldTriggerMode(const char* name, const char* display) :
    SystemConstant(name, display)
{
}

OldTriggerMode* OldTriggerMode::get(const char* name) 
{
	OldTriggerMode* found = NULL;
	if (name != NULL) {
		for (int i = 0 ; OldTriggerModes[i] != NULL ; i++) {
			OldTriggerMode* t = OldTriggerModes[i];
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

OldTarget* OldTargetFunction = new OldTarget("function", "Function");
OldTarget* OldTargetParameter = new OldTarget("parameter", "Parameter");
OldTarget* OldTargetSetup = new OldTarget("setup", "Setup");
OldTarget* OldTargetPreset = new OldTarget("preset", "Preset");
OldTarget* OldTargetBindings = new OldTarget("bindings", "Bindings");
OldTarget* OldTargetUIControl = new OldTarget("uiControl", "UI Control");
OldTarget* OldTargetUIConfig = new OldTarget("uiConfig", "UI Config");
OldTarget* OldTargetScript = new OldTarget("script", "Script");

OldTarget* OldTargets[] = {
	OldTargetFunction,
	OldTargetParameter,
	OldTargetSetup,
	OldTargetPreset,
	OldTargetBindings,
	OldTargetUIControl,
	OldTargetUIConfig,
	OldTargetScript,
	NULL
};
 
OldTarget::OldTarget(const char* name, const char* display) :
    SystemConstant(name, display)
{
}

OldTarget* OldTarget::get(const char* name) 
{
	OldTarget* found = NULL;

    // auto upgrade old bindings
    if (StringEqual(name, "control"))
      name = "parameter";

	if (name != NULL) {
		for (int i = 0 ; OldTargets[i] != NULL ; i++) {
			OldTarget* t = OldTargets[i];
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
 *                                 UI CONTROL                               *
 *                                                                          *
 ****************************************************************************/

OldUIControl::OldUIControl()
{
    init();
}

OldUIControl::OldUIControl(const char* name, int key) :
    SystemConstant(name, key)
{
    init();
}

void OldUIControl::init() {
}

/****************************************************************************
 *                                                                          *
 *                                UI PARAMETER                              *
 *                                                                          *
 ****************************************************************************/

OldUIParameter::OldUIParameter(const char* name, int key) :
    SystemConstant(name, key)
{
}

/****************************************************************************
 *                                                                          *
 *   							   BINDING                                  *
 *                                                                          *
 ****************************************************************************/

void OldBinding::init()
{
	mNext = NULL;

	// trigger
	mTrigger = NULL;
    mTriggerMode = NULL;
    mTriggerPath = NULL;
 	mValue = 0;
	mChannel = 0;

	// target
    mTargetPath = NULL;
	mTarget = NULL;
	mName = NULL;

	// scope
    mScope = NULL;
	mTrack = 0;
	mGroup = 0;

    // arguments
	mArgs = NULL;
}

OldBinding::OldBinding()
{
	init();
}

OldBinding::~OldBinding()
{
	OldBinding *el, *next;

    delete mTriggerPath;
    delete mTargetPath;
	delete mName;
    delete mScope;
	delete mArgs;

	for (el = mNext ; el != NULL ; el = next) {
		next = el->getNext();
		el->setNext(NULL);
		delete el;
	}

}

void OldBinding::setNext(OldBinding* c)
{
	mNext = c;
}

OldBinding* OldBinding::getNext()
{
	return mNext;
}

//
// Trigger
//

void OldBinding::setTrigger(OldTrigger* t) 
{
	mTrigger = t;
}

OldTrigger* OldBinding::getTrigger()
{
	return mTrigger;
}

void OldBinding::setValue(int v) 
{
	mValue = v;
}

int OldBinding::getValue()
{
	return mValue;
}

void OldBinding::setChannel(int c) 
{
	mChannel = c;
}

int OldBinding::getChannel()
{
	return mChannel;
}

bool OldBinding::isMidi()
{
	return (mTrigger == OldTriggerNote ||
			mTrigger == OldTriggerProgram ||
			mTrigger == OldTriggerControl ||
			mTrigger == OldTriggerPitch);
}

void OldBinding::setTriggerPath(const char* s)
{
    delete mTriggerPath;
    mTriggerPath = CopyString(s);
}

const char* OldBinding::getTriggerPath()
{
    return mTriggerPath;
}

void OldBinding::setTriggerMode(OldTriggerMode* t)
{
    mTriggerMode = t;
}

OldTriggerMode* OldBinding::getTriggerMode()
{
    return mTriggerMode;
}

//
// Target
//

void OldBinding::setTargetPath(const char* s)
{
    delete mTargetPath;
    mTargetPath = CopyString(s);
}

const char* OldBinding::getTargetPath()
{
    return mTargetPath;
}

void OldBinding::setTarget(OldTarget* t) 
{
	mTarget = t;
}

OldTarget* OldBinding::getTarget()
{
	return mTarget;
}

void OldBinding::setName(const char *name) 
{
	delete mName;
	mName = CopyString(name);
}

const char* OldBinding::getName()
{
	return mName;
}

// 
// Scope
//

void OldBinding::setScope(const char* s)
{
    delete mScope;
    mScope = CopyString(s);
    parseScope();
}

const char* OldBinding::getScope() 
{
    return mScope;
}

/**
 * Parse a scope into track an group numbers.
 * Tracks are expected to be identified with integers starting
 * from 1.  Groups are identified with upper case letters A-Z.
 */
void OldBinding::parseScope()
{
    mTrack = 0;
    mGroup = 0;

    if (mScope != NULL) {
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

void OldBinding::setTrack(int t) 
{
    if (t > 0) {
        char buffer[32];
        sprintf(buffer, "%d", t);
        setScope(buffer);
    }
}

int OldBinding::getTrack()
{
	return mTrack;
}

void OldBinding::setGroup(int t) 
{
    if (t > 0) {
        char buffer[32];
        sprintf(buffer, "%c", (char)('A' + (t - 1)));
        setScope(buffer);
    }
}

int OldBinding::getGroup()
{
	return mGroup;
}

//
// Arguments
//

void OldBinding::setArgs(const char* args) 
{
	delete mArgs;
	mArgs = CopyString(args);
}

const char* OldBinding::getArgs() 
{
	return mArgs;
}

//
// Utilities
//

void OldBinding::getSummary(char* buffer)
{
	strcpy(buffer, "");

	// we display channel consistenly everywhere as 1-16
	int channel = mChannel + 1;

	if (mTrigger == OldTriggerNote) {
		char note[128];
		MidiNoteName(mValue, note);
		sprintf(buffer, "%d:%s", channel, note);
	}
	else if (mTrigger == OldTriggerProgram) {
		sprintf(buffer, "%d:Program %d", channel, mValue);
	}
	else if (mTrigger == OldTriggerControl) {
		sprintf(buffer, "%d:Control %d", channel, mValue);
	}
	else if (mTrigger == OldTriggerKey) {
		// UI should actually overload this with a smarter key 
		// rendering utility
		sprintf(buffer, "Key %d", mValue);
	}
    else if (mTrigger == OldTriggerOsc) {
        sprintf(buffer, "OSC %s", mTriggerPath);
    }

}

void OldBinding::getMidiString(char* buffer, bool includeChannel)
{
	strcpy(buffer, "");

	// we display channel consistenly everywhere as 1-16
	int channel = mChannel + 1;

	if (mTrigger == OldTriggerControl) {
		int value = getValue();
		if (value >= 0 && value < 128)
		  sprintf(buffer, "%d:Control %d", channel, value);
	}
	else if (mTrigger == OldTriggerNote) {
		char note[128];
		int value = getValue();
		if (value >= 0 && value < 128) {
			MidiNoteName(value, note);
			sprintf(buffer, "%d:%s", channel, note);
		}
	}
	else if (mTrigger == OldTriggerProgram) {
		int value = getValue();
		if (value >= 0 && value < 128)
		  sprintf(buffer, "%d:Program %d", channel, value);
	}
}


/**
 * Render a TriggerKey value as a readable string.
 */
void OldBinding::getKeyString(char* buffer, int max)
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
 *   							BINDING CONFIG                              *
 *                                                                          *
 ****************************************************************************/

OldBindingConfig::OldBindingConfig()
{
	init();
}

void OldBindingConfig::init()
{
	mNext = NULL;
	mName = NULL;
	mBindings = NULL;
}

OldBindingConfig::~OldBindingConfig()
{
	OldBindingConfig *el, *next;

	delete mBindings;

	for (el = mNext ; el != NULL ; el = next) {
		next = el->getNext();
		el->setNext(NULL);
		delete el;
	}
}

OldTarget* OldBindingConfig::getTarget()
{
	return OldTargetBindings;
}

void OldBindingConfig::setNext(OldBindingConfig* c)
{
	mNext = c;
}

OldBindingConfig* OldBindingConfig::getNext()
{
	return mNext;
}

OldBindable* OldBindingConfig::getNextBindable()
{
	return mNext;
}

OldBinding* OldBindingConfig::getBindings()
{
	return mBindings;
}

void OldBindingConfig::setBindings(OldBinding* b)
{
	mBindings = b;
}

void OldBindingConfig::addBinding(OldBinding* b) 
{
    if (b != NULL) {
        // keep them ordered
        OldBinding *prev;
        for (prev = mBindings ; prev != NULL && prev->getNext() != NULL ; 
             prev = prev->getNext());
        if (prev == NULL)
          mBindings = b;
        else
          prev->setNext(b);
    }
}

void OldBindingConfig::removeBinding(OldBinding* b)
{
    if (b != NULL) {
        OldBinding *prev = NULL;
        OldBinding* el = mBindings;
    
        for ( ; el != NULL && el != b ; el = el->getNext())
          prev = el;

        if (el == b) {
            if (prev == NULL)
              mBindings = b->getNext();
            else
              prev->setNext(b->getNext());
        }
        else {
            // not on the list, should we still NULL out the next pointer?
            Trace(1, "OldBindingConfig::removeBinding binding not found!\n");
        }

        b->setNext(NULL);
    }
}

/**
 * Search for a binding for a given trigger and value.
 * This is intended for upgrading old KeyBinding objects, once that
 * has passed we can delete this.
 */
OldBinding* OldBindingConfig::getBinding(OldTrigger* trigger, int value)
{
	OldBinding* found = NULL;

	for (OldBinding* b = mBindings ; b != NULL ; b = b->getNext()) {
		if (b->getTrigger() == trigger && b->getValue() == value) {
            found = b;
            break;
		}
	}

	return found;
}

OldBindingConfig* OldBindingConfig::clone()
{
    // formerly used XML
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


//
// Temporary backward compatibility for things that want the old binding model
// This is what used to be in Binding.h but with "Old" prefixed on all the names
// so it doesn't conflict with the new Binding model.  Only bring things inthat
// are necessary for compiling, we'll rip all this out at some point.
//

#pragma once

#include "../../model/SystemConstant.h"

/****************************************************************************
 *                                                                          *
 *                                  TRIGGER                                 *
 *                                                                          *
 ****************************************************************************/

class OldTrigger : public SystemConstant {
  public:

	static OldTrigger* get(const char* name);

    OldTrigger(const char* name, const char* display, bool bindable);

    bool isBindable();

  private:

   // true if this can be dynamically bound with a Binding object.
   bool mBindable;

};

extern OldTrigger* OldTriggerKey;
extern OldTrigger* OldTriggerMidi;
extern OldTrigger* OldTriggerHost;
extern OldTrigger* OldTriggerOsc;
extern OldTrigger* OldTriggerUI;

// these are used only for binding definitions, not for actions
extern OldTrigger* OldTriggerNote;
extern OldTrigger* OldTriggerProgram;
extern OldTrigger* OldTriggerControl;
extern OldTrigger* OldTriggerPitch;

// internal triggers not used in bindings
extern OldTrigger* OldTriggerScript;
extern OldTrigger* OldTriggerThread;
extern OldTrigger* OldTriggerAlert;
extern OldTrigger* OldTriggerEvent;
extern OldTrigger* OldTriggerUnknown;

/****************************************************************************
 *                                                                          *
 *                                TRIGGER MODE                              *
 *                                                                          *
 ****************************************************************************/

class OldTriggerMode : public SystemConstant {
  public:

	static OldTriggerMode* get(const char* name);

    OldTriggerMode(const char* name, const char* display);

  private:

};

extern OldTriggerMode* OldTriggerModeContinuous;
extern OldTriggerMode* OldTriggerModeOnce;
extern OldTriggerMode* OldTriggerModeMomentary;
extern OldTriggerMode* OldTriggerModeToggle;
extern OldTriggerMode* OldTriggerModeXY;
extern OldTriggerMode* OldTriggerModes[];

/****************************************************************************
 *                                                                          *
 *   							   TARGETS                                  *
 *                                                                          *
 ****************************************************************************/

class OldTarget : public SystemConstant {
  public:

	static OldTarget* get(const char* name);

	OldTarget(const char* name, const char* display);

  private:

};

extern OldTarget* OldTargetFunction;
extern OldTarget* OldTargetParameter;
extern OldTarget* OldTargetSetup;
extern OldTarget* OldTargetPreset;
extern OldTarget* OldTargetBindings;
extern OldTarget* OldTargetUIControl;
extern OldTarget* OldTargetUIConfig;

// internal targets, can't be used in bindings
extern OldTarget* OldTargetScript;

extern OldTarget* OldTargets[];

/****************************************************************************
 *                                                                          *
 *                                 UI CONTROL                               *
 *                                                                          *
 ****************************************************************************/

class OldUIControl : public SystemConstant {

  public:

	OldUIControl();
	OldUIControl(const char* name, int key);

  private:

    void init();

};

/****************************************************************************
 *                                                                          *
 *                                UI PARAMETER                              *
 *                                                                          *
 ****************************************************************************/

class OldUIParameter : public SystemConstant {

  public:

    OldUIParameter(const char* name, int key);

    // never existed...
	//static UIParameter** getParameters();
	//static UIParameter* getParameter(const char* name);
	//static void localizeAll(class MessageCatalog* cat);

  private:

};

/****************************************************************************
 *                                                                          *
 *   							   BINDABLE                                 *
 *                                                                          *
 ****************************************************************************/

class OldBindable {

  public:

	OldBindable();
	~OldBindable();
	void clone(OldBindable* src);

	void setNumber(int i);
	int getNumber();

	void setName(const char* name);
	const char* getName();

    virtual OldBindable* getNextBindable() = 0;
	virtual class OldTarget* getTarget() = 0;

  protected:

	/**
	 * A non-persistent internal number.
	 * Used to uniquely identity presets that may not have names
	 * or have ambiguous names.
	 */
	int mNumber;

	/**
	 * Let configurations be named.
	 */
	char* mName;

};

/****************************************************************************
 *                                                                          *
 *   							   BINDING                                  *
 *                                                                          *
 ****************************************************************************/

class OldBinding {
	
  public:
	
	OldBinding();
	virtual ~OldBinding();

	void setNext(OldBinding* c);
	OldBinding* getNext();

	bool isValid();
	bool isMidi();

	//
	// trigger
	//

	void setTrigger(OldTrigger* t);
	OldTrigger* getTrigger();

    // for MIDI, key, and host parameter triggers
	void setValue(int v);
	int getValue();

	// only for MIDI triggers
	void setChannel(int c);
	int getChannel();

    // only for OSC triggers
    void setTriggerPath(const char* s);
    const char* getTriggerPath();

    // only for OSC triggers
    void setTriggerMode(OldTriggerMode* tt);
    OldTriggerMode* getTriggerMode();

	//
	// target
	//

    void setTargetPath(const char* s);
    const char* getTargetPath();

	void setTarget(OldTarget* t);
	OldTarget* getTarget();

	void setName(const char* s);
	const char* getName();

	void setArgs(const char* c);
	const char* getArgs();

	//
	// scope
	//

    const char* getScope();
    void setScope(const char* s);

    // parsed scopes
	void setTrack(int t);
	int getTrack();
	void setGroup(int g);
	int getGroup();

	//
	// Utils
	//

	void getSummary(char* buffer);
    void getMidiString(char* buffer, bool includeChannel);
    void getKeyString(char* buffer, int max);

  private:

	void init();
    void parseScope();

	OldBinding* mNext;

	// trigger
	OldTrigger* mTrigger;
    OldTriggerMode* mTriggerMode;
    char* mTriggerPath;
	int mValue;
	int mChannel;

	// target
    char* mTargetPath;
	OldTarget* mTarget;
	char* mName;

	// scope, tracks and groups are both numberd from 1
    // both zero means "currently selected track"
    char* mScope;
	int mTrack;
	int mGroup;

    // arguments
	char* mArgs;

};

/****************************************************************************
 *                                                                          *
 *   							BINDING CONFIG                              *
 *                                                                          *
 ****************************************************************************/

class OldBindingConfig : public OldBindable {

  public:

	OldBindingConfig();
	~OldBindingConfig();
	OldBindingConfig* clone();

    OldBindable* getNextBindable();
	OldTarget* getTarget();
	
	void setNext(OldBindingConfig* c);
	OldBindingConfig* getNext();

	void addBinding(OldBinding* c);
	void removeBinding(OldBinding* c);

	OldBinding* getBindings();
	void setBindings(OldBinding* b);

    OldBinding* getBinding(OldTrigger* trig, int value);

  private:

	void init();

	OldBindingConfig* mNext;
	OldBinding* mBindings;
	
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

//
// Temporary backward compatibility for things that want the old binding model
// This is what used to be in Binding.h but with "Old" prefixed on all the names
// so it doesn't conflict with the new Binding model.  Only bring things inthat
// are necessary for compiling, we'll rip all this out at some point.
//

#pragma once

#include "../../model/SystemConstant.h"
#include "../../model/Trigger.h"
#include "../../model/ActionType.h"

/****************************************************************************
 *                                                                          *
 *                                 UI CONTROL                               *
 *                                                                          *
 ****************************************************************************/

#if 0
class OldUIControl : public SystemConstant {

  public:

	OldUIControl();
	OldUIControl(const char* name, int key);

  private:

    void init();

};
#endif

/****************************************************************************
 *                                                                          *
 *                                UI PARAMETER                              *
 *                                                                          *
 ****************************************************************************/
#if 0
class OldUIParameter : public SystemConstant {

  public:

    OldUIParameter(const char* name, int key);

    // never existed...
	//static UIParameter** getParameters();
	//static UIParameter* getParameter(const char* name);
	//static void localizeAll(class MessageCatalog* cat);

  private:

};
#endif

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
	virtual class ActionType* getTarget() = 0;

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

	void setTrigger(Trigger* t);
	Trigger* getTrigger();

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
    void setTriggerMode(TriggerMode* tt);
    TriggerMode* getTriggerMode();

	//
	// target
	//

    void setTargetPath(const char* s);
    const char* getTargetPath();

	void setTarget(ActionType* t);
	ActionType* getTarget();

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
	Trigger* mTrigger;
    TriggerMode* mTriggerMode;
    char* mTriggerPath;
	int mValue;
	int mChannel;

	// target
    char* mTargetPath;
	ActionType* mTarget;
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
	ActionType* getTarget();
	
	void setNext(OldBindingConfig* c);
	OldBindingConfig* getNext();

	void addBinding(OldBinding* c);
	void removeBinding(OldBinding* c);

	OldBinding* getBindings();
	void setBindings(OldBinding* b);

    OldBinding* getBinding(Trigger* trig, int value);

  private:

	void init();

	OldBindingConfig* mNext;
	OldBinding* mBindings;
	
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

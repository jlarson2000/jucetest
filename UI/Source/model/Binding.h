/*
 * A model for defining associations between external stimuli like MIDI,
 * or keyboard events, internal responses like calling a function or changing
 * a parameter.
 *
 * See model-final.txt for more complete notes.
 *
 * Logically a binding consnts of three things: trigger, operation, destination
 *
 * Triggers are things that cause something to happen.
 * There are several fundamantal trigger types defined by a set of system constant
 * objects.
 *
 *    TriggerMidi   - a MIDI event
 *    TriggerKey    - a computer keyboard key
 *    TriggerUI     - something originating in the user interface
 *    TriggerHost   - a VST or AU plugin parameter
 *    TriggerOSC    - the Open Sound Control interface
 *    TriggerScript - a Mobius script
 *
 * Each trigger type may store additional information about the trigger in the binding
 * for example the MIDI event type (note, program, control) and event value (key number,
 * program number, controller number).
 *
 * Operations are what you want the trigger to do.  They are defined
 * by these system constant objects.
 *
 *    OpFunction       - call a built-in function
 *    OpScript         - call a user defined function (script)
 *    OpParameter      - set the value of a parameter
 *    OpActivation     - apply a collection of parameters (preset, setup, binding set)
 *
 * Destinations are where you want the operation to go.
 *
 * These are more abstract than triggers and operations but fundamentally think
 * of them as "paths" that identify something within the system
 * that can receive a request to perform an operation.  Some examples:
 *
 *    the active track
 *    the specific track 4
 *    the collection of tracks 1,2,3
 *    a group of gracks named "Background"
 *    loop number 2 in track 3
 *    a graphical item in the user interface
 *
 * There are two broad categories of destinations, those in the user interface and
 * those in the looping engine.  Most destinations are in the engine.  
 *
 * Scopes are a simplified representation of a destination that are represented
 * in the binding model as a string of text.
 * 
 * Because most destinations are in the engine which has a fixed internal structure.
 * We define a set of value conventions for the scope property.
 *
 *    digit - a track number
 *    name  - a track group name
 *    "ui"  - the user interface
 *
 * If a scope is not defined the operation is assumed to be targted at the
 * active track in the engine.
 * Scope specifiers may become more flexible than this in the future.
 */

#pragma once

#include <vector>

#include "Structure.h"
#include "SystemConstant.h"

//////////////////////////////////////////////////////////////////////
//
// Trigger
//
//////////////////////////////////////////////////////////////////////

/**
 * Triggers are the "who" of a binding.
 * They define where the trigger came from which in turn may
 * imply things about the way the action should be processed.
 *
 * Do we realy need display names for these?  
 */
class Trigger : public SystemConstant {
  public:

    Trigger(const char* name, const char* display);

    static std::vector<Trigger*> Triggers;
    static Trigger* find(const char* name);

    static bool isMidi(Trigger* t);
    
};

// these have historically been global constants
// think about moving them inside Trigger

extern Trigger* TriggerKey;
extern Trigger* TriggerMidi;
extern Trigger* TriggerHost;
extern Trigger* TriggerOsc;
extern Trigger* TriggerUI;

// these are used only for binding definitions, not for actions
extern Trigger* TriggerNote;
extern Trigger* TriggerProgram;
extern Trigger* TriggerControl;
extern Trigger* TriggerPitch;

// internal triggers not used in bindings
// not all of these may be used when we finish porting
// weed them out
extern Trigger* TriggerScript;
extern Trigger* TriggerThread;
extern Trigger* TriggerAlert;
extern Trigger* TriggerEvent;
extern Trigger* TriggerUnknown;

//////////////////////////////////////////////////////////////////////
//
// Trigger Mode
//
//////////////////////////////////////////////////////////////////////

/**
 * Defines the behavior of the trigger over time.
 * 
 * Triggers can behave in several ways, the most common are
 * as momentary buttons and as continuous controls.
 *
 * Some trigger constants imply their mode, TriggerNote
 * for example can be assumed to behave like a momentary button.
 * Others like TriggerOsc and TriggerUI are more generic.  They
 * may have several behaviors.  
 *
 * If a Binding is created with an ambiguous Trigger, a TriggerMode
 * must also be specified.  If not then TriggerTypeOnce is assumed.
 *
 */
class TriggerMode : public SystemConstant {
  public:

    static std::vector<TriggerMode*> TriggerModes;
    static TriggerMode* find(const char* name);

    TriggerMode(const char* name, const char* display);
};

// the trigger happens a single time
extern TriggerMode* TriggerModeOnce;

// the trigger has both on/pressed and up/released transitions
extern TriggerMode* TriggerModeMomentary;

// the trigger sweeps through a range of values
extern TriggerMode* TriggerModeContinuous;

// the trigger is momentary, but sustains for an indefinite period of time
extern TriggerMode* TriggerModeToggle;

// the trigger is continues but sweeps through two ranges of values (never used)
extern TriggerMode* TriggerModeXY;

//////////////////////////////////////////////////////////////////////
//
// Operations
//
//////////////////////////////////////////////////////////////////////

/**
 * An operation defines what you would like the binding to do.
 */
class Operation : public SystemConstant {
  public:

    static std::vector<Operation*> Operations;
	static Operation* find(const char* name);

	Operation(const char* name, const char* display);
};

extern Operation* OpFunction;
extern Operation* OpParameter;
extern Operation* OpActivation;
extern Operation* OpScript;

// until we can refactor all the old uses of TargetPreset
// and decide on the right concrete model, define these
// here just so we have a place to store the names
// they aren't really Operations

extern Operation* OpSetup;
extern Operation* OpPreset;
extern Operation* OpBindings;

//////////////////////////////////////////////////////////////////////
//
// Binding
//
//////////////////////////////////////////////////////////////////////

/**
 * Defines a binding between a trigger, operation, and destination.
 * A list of these is contained in a BindingSet
 *
 * Formerly had OSC support in here but I removed it and we can revisit
 * the design later to keep things clean.
 *
 * OSC bindings were added later, need to revisit the design and consider
 * using "paths" consistently everywhere rather than breaking it down
 * into discrete target/name/scope/args, at least not in the persistent model.
 *
 * Scope started out as a pair of track and group numbers but now
 * it is string whose value is a scope name with these conventions:
 *
 *    null - a global binding (current track, focused tracks, group tracks)
 *    digit - track number
 *    letter - group identifier (A,B,C...)
 *
 * In theory we can add other scopes like "muted", simliar to the
 * track identifiers we have with the "for" statement in scripts.
 * 
 * There is usually a number associated with the trigger.
 *   Note - note number
 *   Program - program number 
 *   Control - control number
 *   Key - key code
 *   Wheel - NA
 *   Mouse - ?
 *   Host - host parameter number
 *
 * MIDI triggers may have an additional channel number.
 *
 * Some operations may have an additional numeric argument.
 *   - only functions like Track ?
 *
 * OSC bindings use targetPath instead of discrete target, name, scope, 
 * and arguments values.  This will be parsed into the discrete values.
 * OSC bindings also have a string value (the source method) rather than
 * a simple integer like MIDI notes.  This is only used when Bindings
 * are inside an OscBindingSet.
 * 
 */
class Binding {
	
  public:
	
	Binding();
	Binding(Binding* src);
	virtual ~Binding();

    // keep these on a linked list for now, convert to vector later
	void setNext(Binding* c);
	Binding* getNext();

    // true if the binding is filled out enough to be useful
	bool isValid();

    // true if this represents a MIDI binding
	bool isMidi();

	// trigger

    Trigger *trigger;
    TriggerMode* triggerMode;

    // key number, midi note number
    int triggerValue;

    // for TriggerMidi, the MIDI channel
    // todo: eventually want the note/program/control types as seperate
    // values rather than overloading Trigger
    int midiChannel;

    // Operation
    
    // todo: eventually get rid of OpPreset and instead just
    // use OpActivation and store activationType?
    Operation *op;
    
    void setOperationName(const char* s);
	const char* getOperationName();

	void setArguments(const char* c);
	const char* getArguments();

	// scope

    void setScope(const char* s);
    const char* getScope();

    // parsed scopes
    // !! won't be able to do this automatically once we allow
    // groups to have user defined names
    int trackNumber;
    int groupOrdinal;


    // utilities to take a number and convert it to a scope string
    void setTrack(int t);
    void setGroup(int g);

  private:

    void parseScope();

	Binding* mNext;
	char* mOperationName;
    char* mArguments;
    char* mScope;

};

//////////////////////////////////////////////////////////////////////
//
// BindingSet
//
//////////////////////////////////////////////////////////////////////

/**
 * An object managing a named collection of Bindings, with convenience
 * methods for searching them.
 */
class BindingSet : public Structure {

  public:

	BindingSet();
	BindingSet(BindingSet* src);
	~BindingSet();

    Structure* clone();
    
    Binding* getBindings();
	void setBindings(Binding* b);

	void addBinding(Binding* c);
	void removeBinding(Binding* c);

  private:

	Binding* mBindings;
	
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

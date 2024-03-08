/**
 * A collection of static objects that define the types of things
 * that can cause something to happen.  They are part of both
 * the Action and Binding models, but factored out because they
 * need to be used are used at various levels that don't need to
 * understand where they came from.
 *
 * They self initialize during static initialization and will
 * self destruct.
 */

#pragma once

#include <vector>

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

    static std::vector<Trigger*> Instances;
    static Trigger* find(const char* name);
    static bool isMidi(Trigger* t);

    Trigger(const char* name, const char* display);
    
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

    static std::vector<TriggerMode*> Instances;
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

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


#include <JuceHeader.h>

#include "util/Trace.h"
#include "model/MobiusConfig.h"
#include "model/UIAction.h"
#include "model/FunctionDefinition.h"
#include "model/Binding.h"

#include "KeyTracker.h"
#include "Supervisor.h"

#include "Binderator.h"

Binderator::Binderator(Supervisor * super)
{
    supervisor = super;
}

Binderator::~Binderator()
{
    KeyTracker::removeListener(this);
    supervisor->getMidiManager()->removeListener(this);
}

void Binderator::start()
{
    if (!started) {
        KeyTracker::addListener(this);
        supervisor->getMidiManager()->addListener(this);
        started = true;
    }
}

void Binderator::stop()
{
    if (started) {
        KeyTracker::removeListener(this);
        supervisor->getMidiManager()->removeListener(this);
        started = false;
    }
}

void Binderator::configure(MobiusConfig* config)
{
    // if we're reconfiguring, initialize the binding array
    keyActions.clear();
    noteActions.clear();
    // juce::Array is fucking annoying, even if you call ensureStorageAllocated
    // you can't just index into sparse arrays without setting something
    // there first, even if it is nullptr, if not it appends
    // if we do this then don't really need to call clear()
    for (int i = 0 ; i < 256 ; i++) {
        keyActions.set(i, nullptr);
        noteActions.set(i, nullptr);
    }
    
    // only pay attention to the base set for now
    BindingSet* baseBindings = config->getBindingSets();
    if (baseBindings != nullptr) {
        Binding* bindings = baseBindings->getBindings();
        while (bindings != nullptr) {
            installAction(bindings);
            bindings = bindings->getNext();
        }
    }
}

void Binderator::installAction(Binding* b)
{
    Trigger* trigger = b->trigger;

    const char* name = b->getActionName();
    if (name == nullptr) {
        trace("Binderator: Ignoring Binding with no name\n");
    }
    else if (trigger == TriggerKey) {
        int code = b->triggerValue;
        // could check the upper range too
        if (code <= 0) {
            trace("Binderator: Ignoring Binding with invalid value %s\n", name);
        }
        else {
            // mask off all but the bottom byte to get rid of bit 17 for extended chars
            code &= 0xFF;

            ActionType* type = b->action;
            if (type == nullptr) {
                trace("Binderator: Ignoring Binding with no action type %s\n", name);
            }
            else if (type != ActionFunction) {
                // only support Functions for awhile
                trace("Binderator: Ignoring Binding for non-function action %s\n", name);
            }
            else {
                FunctionDefinition* func = FunctionDefinition::find(name);
                if (func == nullptr) {
                    trace("Binderator: Ignoring Binding for invalid function %s\n", name);
                }
                else {
                    UIAction* action = new UIAction();
                    action->trigger = trigger;
                    action->type = type;
                    action->implementation.function = func;
                    // todo: scope and args
                    keyActions.set(code, action);
                }
            }
        }
    }
    else if (trigger == TriggerNote) {
        // todo: channels, do we really need 16 arrays for all this or should
        // we pile them into one and have a list?
        int note = b->triggerValue;
        if (note < 0 || note > 0xFF) {
            trace("Binderator: Invalid MIDI note %d\n", note);
        }
        else {
            ActionType* type = b->action;
            if (type == nullptr) {
                trace("Binderator: Ignoring Binding with no action type %s\n", name);
            }
            else if (type != ActionFunction) {
                // only support Functions for awhile
                trace("Binderator: Ignoring Binding for non-function action %s\n", name);
            }
            else {
                FunctionDefinition* func = FunctionDefinition::find(name);
                if (func == nullptr) {
                    trace("Binderator: Ignoring Binding for invalid function %s\n", name);
                }
                else {
                    UIAction* action = new UIAction();
                    action->trigger = trigger;
                    action->type = type;
                    action->implementation.function = func;
                    // todo: scope and args
                    noteActions.set(note, action);
                }
            }
        }
    }
}

void Binderator::keyTrackerDown(int code, int modifiers)
{
    if (started) {
        int index = code & 0xFF;
        UIAction* action = keyActions[index];
        if (action != nullptr) {
            supervisor->doAction(action);
        }
    }
}

// no up yet

void Binderator::keyTrackerUp(int code, int modifiers)
{
}

void Binderator::midiMessage(const juce::MidiMessage& message, juce::String& source)
{
    if (started) {
        if (message.isNoteOn()) {
            int index = message.getNoteNumber();
            UIAction* action = noteActions[index];
            if (action != nullptr) {
                supervisor->doAction(action);
            }
        }
    }
}

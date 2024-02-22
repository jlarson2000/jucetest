/*
 *  having some trouble with F12, wants to load ntdll.pdb
*
 * Tracking press/release with modifiers will be hard.
 * keyPressed has the key code and modifiers
 * but isCurrentlyDown only tests codes.
 *
 * ModifierKeys has bitmask constants for various modifiers
 *   shiftModifier, ctrlModifier, altModifier
 *
 * Press/release of just modifier keys sends keyStateChanged but you
 * do not get keyPressed and can't tell which modifier it was.
 * okay for my purposes.
 *
 * For simple up transitions when down put the key code in an array.
 * On each state change iterate the pressed array and if the key is
 * no longer pressed it went up.  
 *
 * For modifiers, save both the code and all modifiers in the tracking array
 * and generate down with modifiers.  On up for each key in the array
 * that is no longer down gets an up event WITH the modifiers that were
 * down at the time the key was pressed.
 *
 * For bindings, the easiest is to just ignore modifiers and look
 * for down/up.  Only generate actions for targets that are sustainable.
 * Check function->sustainable
 *
 * If you allow modifier bindings at all then the modifiers
 * be part of the jump tables.  e.g. S is different than ctrl-S
 *
 * key codes for function keys are large
 *   F10 = 65657  0x10079
 *
 * so that is not a simple 8-bit number
 *
 * Note that shift-9 comes in as 9 with shift modifier so you do not
 * get the ascii code for (
 *
 * Not all of my old name mappings work.  slash 47 maps to "Help"
 * Backquote 96 shows Num0
 *
 * So will need to recapture all these, or use Juce tools.
 * F12 generates a break in VisualStudio for some reason
 * the code is 65659 which is displayed correctly as  F12 but it seems
 * to be interpreted by the OS.
 * ASCII charts shows ` as 96 so my tables must be wrong.
 *
 * key codes ignore case so to do full ascii bindings with upper/lower
 * will have to save both key code and shift modifier, and interpret it.
 * Sadkly the Juce text char mapper isn't static but you can build
 * a KeyCode object.
 *
 * For Mobius modifiers might just be confusing.  No one besides me will
 * take the time to use shift keys while playing.  key values in the Binding
 * couldn't be the current numbers, they could be chars like a and A
 * for simple shifts which is better than nothing.  But there is no ASCII
 * for F10 so we would have to store the symbolic names and parse them.
 *
 * 
 */


#include <JuceHeader.h>

#include "util/Keycode.h"
#include "util/Trace.h"

#include "KeyTracker.h"

KeyTracker::KeyTracker()
{
}

KeyTracker::~KeyTracker()
{
}

void KeyTracker::addListener(Listener* l)
{
    listeners.add(l);
}

/**
 * Return true to indiciate that the key has been consumed.
 * getKeyCode returns
 *  "This will either be one of the special constants defined in this class, or an 8-bit character code"
 *
 * Constants include things like spaceKey, backspaceKey, numberPad9, etc.
 
 */
bool KeyTracker::keyPressed(const juce::KeyPress& key, juce::Component* originator)
{
    trace("keyPressed: %d %s\n", key.getKeyCode(), key.getTextDescription().toUTF8());

    char buf[32];
    GetKeyString(key.getKeyCode(), buf);
    trace("%s\n", buf);
    
    return false;
}

bool KeyTracker::keyStateChanged(bool isKeyDown, juce::Component* originator)
{
    trace("keyStateChanged: %s\n", (isKeyDown) ? "down" : "up");
    return false;
}


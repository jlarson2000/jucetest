// having some trouble with F12, wants to load ntdll.pdb

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

/**
 * Return true to indiciate that the key has been consumed.
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


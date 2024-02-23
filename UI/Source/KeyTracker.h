/*
 * Singleton that tracks application keyboard events
 * and does the usual transformations for detecting up transitions.
 * It is only useful to have one of these so it is maintained
 * as a static singleton.
 *
 * Since this can only be touched from the Juce main application thread
 * it should be thread safe.
 */

#pragma once

#include <JuceHeader.h>

class KeyTracker : public juce::KeyListener
{
  public:

    static KeyTracker Instance;

    class Listener {
      public:
        virtual void keyTrackerDown(int code, int modifiers) = 0;
        virtual void keyTrackerUp(int code, int modifiers) = 0;
    };
    
    static void addListener(Listener* l);
    static void removeListener(Listener* l);

    // utilieis for the Listeners
    static juce::String getKeyText(int code, int modifiers);
    static int parseKeyText(juce::String& text);
    static void parseKeyText(juce::String& text, int* code, int* modidifers);
    static void dumpCodes();

    KeyTracker();
    ~KeyTracker();

    // juce::KeyListener
    bool keyPressed(const juce::KeyPress& key, juce::Component* originator);
    bool keyStateChanged(bool isKeyDown, juce::Component* originator);

  private:

    juce::Array<Listener*> listeners;
    juce::Array<int> pressedCodes;
    juce::Array<int> pressedModifiers;
     
    void traceKeyPressed(const juce::KeyPress& key, juce::Component* originator);
    void traceKeyStateChanged(bool isKeyDown, juce::Component* originator);
    void traceTrackerDown(int code, int modifiers);
    void traceTrackerUp(int code, int modifiers);

    void addLocalListener(Listener* l);
    void removeLocalListener(Listener* l);
    
};

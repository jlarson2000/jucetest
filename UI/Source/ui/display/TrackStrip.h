/**
 * A component that displays a configurable set of track
 * status elements and controls for one track.
 *
 * Found at the bottom of MobiusDisplay.
 * Mainted in a set by TrackStrips
 */

#pragma once

#include <JuceHeader.h>

class TrackStrip : public juce::Component
{
  public:
    
    TrackStrip(class TrackStrips*);
    TrackStrip(class FloatingStripElement*);
    ~TrackStrip();

    void setFloatingConfig(int i);
    void setFollowTrack(int i);
    
    int getTrackNumber();

    void configure(class UIConfig* config);
    void layout(juce::Rectangle<int>);
    void update(class MobiusState* state);

    void resized() override;
    void paint(juce::Graphics& g) override;

    void doAction(class UIAction* action);

  private:

    // parent when we're in the docked strips
    class TrackStrips* strips;

    // parent when we're in a floating status element
    class FloatingStripElement* floater;
    
    // taking a different approach than StatusArea and
    // allocating these dynamicall since you don't usually
    // want that many of them
    juce::OwnedArray<class StripElement> elements;

    // the track to follow, -1 means the active track
    int followTrack = -1;

    // the floating configuration to use, 0 is the first
    int floatingConfig = 0;

    // if we're a floating strip, update needs to remember the
    // selected track here
    // if we're a docked strip, this controls the border highlighting
    int activeTrack = 0;

};


    

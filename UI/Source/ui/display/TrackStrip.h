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
    ~TrackStrip();

    void setFloatingConfig(int i);
    void setFollowTrack(int i);
    
    int getTrackNumber();

    void configure(class UIConfig* config);
    void layout(juce::Rectangle<int>);
    void update(class MobiusState* state);

    void resized() override;
    void paint(juce::Graphics& g) override;

  private:

    // parent, shouldn't be needed for much
    // but this is how we tell if we're floating or docked
    class TrackStrips* strips;

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


    

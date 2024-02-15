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
    
    TrackStrip(TrackStrips*);
    ~TrackStrip();

    void configure(UIConfig* config);

    void resized() override;
    void paint(juce::Graphics& g) override;

  private:

    class TrackStrips* strips;
    
    // list of owned sub-components 

};


    

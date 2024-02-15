/**
 * A component that displays a list of track controls for all tracks.
 *
 * Found at the bottom of MobiusDisplay.
 */

#pragma once

#include <JuceHeader.h>

class TrackStrips : public juce::Component
{
  public:
    
    TrackStrips(class MobiusDisplay*);
    ~TrackStrips();

    void configure(class UIConfig* config);
    void layout(juce::Rectangle<int>);
    
    void resized() override;
    void paint(juce::Graphics& g) override;

  private:

    class MobiusDisplay* display;
    juce::OwnedArray<class TrackStrip> tracks;
    
};


    

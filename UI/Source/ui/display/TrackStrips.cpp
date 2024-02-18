
#include <JuceHeader.h>

#include "../../model/UIConfig.h"
#include "../../model/MobiusState.h"
#include "MobiusDisplay.h"
#include "TrackStrip.h"
#include "TrackStrips.h"

TrackStrips::TrackStrips(MobiusDisplay* parent)
{
    setName("TrackStrips");
    display = parent;
}

TrackStrips::~TrackStrips()
{
}

void TrackStrips::configure(UIConfig* config)
{
    // todo: max tracks comes from MobiusConfig
    // should be able to handle a number change without restarting
    int maxTracks = 8;

    if (tracks.size() == 0) {
        // first time in
        for (int i = 0 ; i < maxTracks ; i++) {
            TrackStrip* strip = new TrackStrip(this);
            strip->setFollowTrack(i);
            tracks.add(strip);
            addAndMakeVisible(strip);
        }
    }

    for (int i = 0 ; i < tracks.size() ; i++) {
        TrackStrip* strip = tracks[i];
        strip->configure(config);
    }
}

void TrackStrips::update(MobiusState* state)
{
    for (int i = 0 ; i < tracks.size() ; i++) {
        TrackStrip* strip = tracks[i];
        strip->update(state);
    }
}

void TrackStrips::layout(juce::Rectangle<int> parentBounds)
{
    // layout each strip
    for (int i = 0 ; i < tracks.size() ; i++) {
        TrackStrip* strip = tracks[i];
        strip->layout(parentBounds);
        // it will have been shoved all the way to the left, move it
        // since they're all the same width, can just use the one we just layed out
        strip->setTopLeftPosition(i * strip->getWidth(), strip->getY());
    }

    // get the first one
    int totalWidth = 0;
    int maxHeight = 0;
    
    if (tracks.size() > 0) {
        TrackStrip* strip = tracks[0];
        totalWidth = strip->getWidth() * tracks.size();
        maxHeight = strip->getHeight();
    }
    setSize(totalWidth, maxHeight);
}

void TrackStrips::resized()
{
}

void TrackStrips::paint(juce::Graphics& g)
{
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

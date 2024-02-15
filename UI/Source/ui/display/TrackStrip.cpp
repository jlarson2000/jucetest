
#include <JuceHeader.h>

#include "TrackStrips.h"
#include "TrackStrip.h"

TrackStrip::TrackStrip(TrackStrips* parent)
{
    setName("TrackStrip");
    strips = parent;
}

TrackStrip::~TrackStrip()
{
}

void TrackStrip::configure(UIConfig* config)
{
}

void TrackStrip::resized()
{
}

void TrackStrip::paint(juce::Graphics& g)
{
}



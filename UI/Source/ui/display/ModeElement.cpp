

#include <JuceHeader.h>

#include "../../model/UIConfig.h"
#include "../../model/MobiusState.h"
#include "../../model/ModeDefinition.h"

#include "Colors.h"
#include "StatusArea.h"
#include "ModeElement.h"

ModeElement::ModeElement(StatusArea* area) :
    StatusElement(area, "ModeElement")
{
}

ModeElement::~ModeElement()
{
}

void ModeElement::configure(UIConfig* config)
{
}

void ModeElement::update(MobiusState* state)
{
    MobiusTrackState* track = &(state->tracks[state->activeTrack]);
    MobiusLoopState* loop = &(track->loops[track->activeLoop]);
    ModeDefinition* mode = loop->mode;

    if (mode == nullptr) {
        // could interpret this to mean Reset?
    }
    else if (!StringEqual(mode->getName(), current.toUTF8())) {
        current = mode->getName();
        repaint();
    }
}

int ModeElement::getPreferredHeight()
{
    return 30;
}

int ModeElement::getPreferredWidth()
{
    return 100;
}

void ModeElement::resized()
{
}

void ModeElement::paint(juce::Graphics& g)
{
    // borders, labels, etc.
    StatusElement::paint(g);

    juce::Font font = juce::Font(getHeight() * 0.8f);
    g.drawText(current, 0, 0, getWidth(), getHeight(), juce::Justification::left);
}

    

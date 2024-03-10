/**
 * Simple layer list.
 * 
 */

#include <JuceHeader.h>

#include "../../util/Trace.h"
#include "../../model/UIConfig.h"
#include "../../model/MobiusState.h"
#include "../../model/ModeDefinition.h"

#include "Colors.h"
#include "StatusArea.h"
#include "LayerElement.h"

const int LayerPreferredWidth = 200;
const int LayerPreferredHeight = 40;

LayerElement::LayerElement(StatusArea* area) :
    StatusElement(area, "LayerElement")
{
}

LayerElement::~LayerElement()
{
}

void LayerElement::configure(UIConfig* config)
{
    // todo: could adjust the diameter
}

int LayerElement::getPreferredHeight()
{
    return LayerPreferredHeight;
}

int LayerElement::getPreferredWidth()
{
    return LayerPreferredWidth;
}

void LayerElement::resized()
{
}

const int LayerInset = 2;

void LayerElement::update(MobiusState* state)
{
    MobiusTrackState* track = &(state->tracks[state->activeTrack]);
    MobiusLoopState* loop = &(track->loops[track->activeLoop]);

}

/**
 * If we override paint, does that mean we control painting
 * the children, or is that going to cascade?
 */
void LayerElement::paint(juce::Graphics& g)
{
    // borders, labels, etc.
    StatusElement::paint(g);

    g.setColour(juce::Colour(MobiusRed));
    
    g.drawRect(LayerInset,
               LayerInset,
               getWidth() - (LayerInset * 2) - savedLevel,
               getHeight() - (LayerInset * 2));
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/**
 * Simple audio level meter for track input.
 *
 * From the old AudioMeter code
 *
 *  // this seems to be too sensitive, need a trim control?
 * 	//mMeter->setRange((1024 * 32) - 1);
 * 	setRange((1024 * 8) - 1);
 *
 * mRange seems to set the granularity or sensitivity.
 *
 * This decides what level to display:
 *
 * 	if (mValue != i && i >= 0 && i <= mRange) {
 * 		mValue = i;
 * 
 * 		// typically get a lot of low level noise which flutters
 * 		// the value but is not actually visible, remember the last
 * 		// level and don't repaint unless it changes
 * 		int width = mBounds.width - 4;
 *         int level = (int)(((float)width / (float)mRange) * mValue);
 * 		if (level != mLevel) {
 * 			mLevel = level;
 * 			if (isEnabled())
 * 			  invalidate();
 * 		}
 * 	}
 *
 * And this paints it:
 *
 * 			mLevel = (int)(((float)b.width / (float)mRange) * mValue);
 * 
 * 			if (mLevel > 0) {
 * 				g->setColor(mMeterColor);
 * 				g->fillRect(b.x, b.y, mLevel, b.height);
 * 			}
 *
 * So mLevel is the width of the level rectangle to draw taken as a
 * raction of the actual width times the value
 *
 * The value in MobiusState is calculated down in Stream as:
 *  int OutputStream::getMonitorLevel()
 *  {
 *   	// convert to 16 bit integer
 *   	return (int)(mMaxSample * 32767.0f);
 *  }
 *
 * And mMaxSample comes directly from the audio stream
 * Not following the old math here, but we can make the same assumption.
 *
 * And finally the value is set here:
 *  
 *     mMeter->update(tstate->inputMonitorLevel);
 *
 * So don't trigger a repaint just when the raw sample changes because
 * that would be way too bouncy.
 * 
 */

#include <JuceHeader.h>

#include "../../util/Trace.h"
#include "../../model/UIConfig.h"
#include "../../model/MobiusState.h"
#include "../../model/ModeDefinition.h"

#include "Colors.h"
#include "StatusArea.h"
#include "AudioMeterElement.h"

const int AudioMeterPreferredWidth = 200;
const int AudioMeterPreferredHeight = 20;

AudioMeterElement::AudioMeterElement(StatusArea* area) :
    StatusElement(area, "AudioMeterElement")
{
    // didn't seem to change in old chde
    range = (1024 * 8) - 1;
}

AudioMeterElement::~AudioMeterElement()
{
}

void AudioMeterElement::configure(UIConfig* config)
{
    // todo: could adjust the diameter
}

int AudioMeterElement::getPreferredHeight()
{
    return AudioMeterPreferredHeight;
}

int AudioMeterElement::getPreferredWidth()
{
    return AudioMeterPreferredWidth;
}

void AudioMeterElement::resized()
{
}

const int AudioMeterInset = 2;

void AudioMeterElement::update(MobiusState* state)
{
    MobiusTrackState* track = &(state->tracks[state->activeTrack]);
    int value = track->inputMonitorLevel;
    
	if (savedValue != value && value >= 0 && value <= range) {
		savedValue = value;

        // the old way, I don't like how we're making assumptions
        // about repait based on the current width what if width
        // isn't set yet, or changes between now and paint?
        
		// typically get a lot of low level noise which flutters
		// the value but is not actually visible, remember the last
		// level and don't repaint unless it changes
		int width = getWidth() - (AudioMeterInset * 2);
        int level = (int)(((float)width / (float)range) * value);
		if (level != savedLevel) {
			savedLevel = level;
            repaint();
		}
	}
}

/**
 * If we override paint, does that mean we control painting
 * the children, or is that going to cascade?
 */
void AudioMeterElement::paint(juce::Graphics& g)
{
    // borders, labels, etc.
    StatusElement::paint(g);

    g.setColour(juce::Colour(MobiusRed));
    
    g.drawRect(AudioMeterInset,
               AudioMeterInset,
               getWidth() - (AudioMeterInset * 2) - savedLevel,
               getHeight() - (AudioMeterInset * 2));
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/**
 * Status element to display the playback beats.
 */

#pragma once

#include <JuceHeader.h>

#include "StatusElement.h"

#define BEAT_DECAY 150

/**
 * Internal component maingained by Beaeters
 * Not a full StatusElement
 */
class Beater : public juce::Component
{
  public:
    
    Beater() {
        setName("Beater");
    }

    ~Beater() {
    }
    
	int decayCounter = 0;

    bool start();
    bool tick();
    bool reset();

    void paintBeater(juce::Graphics& g, bool on);

  private:
    
};

class BeatersElement : public StatusElement
{
  public:
    
    BeatersElement(class StatusArea* area);
    ~BeatersElement();

    void configure(class UIConfig* config) override;
    void update(class MobiusState* state) override;
    int getPreferredWidth() override;
    int getPreferredHeight() override;

    void resized() override;
    void paint(juce::Graphics& g) override;

  private:

    Beater loopBeater;
    Beater cycleBeater;
    Beater subcycleBeater;

    bool update(Beater* b, bool* hit);

};

    

/**
 * Status element to display a floating strip of track controls.
 */

#pragma once

#include <JuceHeader.h>

#include "StatusElement.h"
#include "TrackStrip.h"

class FloatingStripElement : public StatusElement
{
  public:
    
    FloatingStripElement(class StatusArea* area);
    ~FloatingStripElement();

    void configure(class UIConfig* config) override;
    // void configure(class MobiusConfig* config) override;
    void update(class MobiusState* state) override;
    int getPreferredWidth() override;
    int getPreferredHeight() override;

    void resized() override;
    void paint(juce::Graphics& g) override;

    void doAction(class UIAction* action);
    
  private:

    TrackStrip strip {this};
    
};

    

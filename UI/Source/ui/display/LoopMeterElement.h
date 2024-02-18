/**
 * Status element to display the current loop's mode.
 */

#pragma once

#include <JuceHeader.h>

#include "StatusElement.h"

class LoopMeterElement : public StatusElement
{
  public:
    
    LoopMeterElement(class StatusArea* area);
    ~LoopMeterElement();

    void configure(class UIConfig* config) override;
    // void configure(class MobiusConfig* config) override;
    void update(class MobiusState* state) override;
    int getPreferredWidth() override;
    int getPreferredHeight() override;

    void resized() override;
    void paint(juce::Graphics& g) override;

  private:

    juce::String current;
};

    

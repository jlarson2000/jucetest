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

    long savedFrames = 0;
    long savedFrame = 0;

    // for events just maintain a pointer directly to MobiusState
    // which is known to live between udpate and paint
    // can't easily do difference detection on this but we can
    // for the frame pointer which triggers a full refresh
    class MobiusLoopState* loop = nullptr;

    int getMeterOffset(long frame);

};

    

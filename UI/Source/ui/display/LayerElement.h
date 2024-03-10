/**
 * Basic level meter
 */

#pragma once

#include <JuceHeader.h>

#include "StatusElement.h"

class LayerElement : public StatusElement
{
  public:
    
    LayerElement(class StatusArea* area);
    ~LayerElement();

    void configure(class UIConfig* config) override;
    void update(class MobiusState* state) override;
    int getPreferredWidth() override;
    int getPreferredHeight() override;

    void resized() override;
    void paint(juce::Graphics& g) override;

  private:

    int range = 0;
    int savedValue = 0;
    int savedLevel = 0;

};

    

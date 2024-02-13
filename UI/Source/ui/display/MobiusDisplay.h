/*
 * Component containing the main Mobius display.
 * This is what you see when you are not using configuration
 * popup editors.
 *
 * It contains a row of buttons at the top, a row of track strips
 * on the bottom, and configurable status items in the center.
 */

#pragma once

#include <JuceHeader.h>

#include "BaseItem.h"

class MobiusDisplay : public juce::Component
{
  public:

    MobiusDisplay();
    ~MobiusDisplay();

    void configure(class MobiusConfig* mainConfig, class UIConfig* uiConfig);

  private:

    ButtonsItem buttons;
    TracksItem tracks;

    juce::OwnedArray<BaseItem> items;

};

    

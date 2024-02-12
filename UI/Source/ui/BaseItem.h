/*
 * The main Mobius display (not configuration) is made
 * up of "items" that can be configured and often paint
 * themselves as with simple geometic shapes rather than
 * Juce Components.
 *
 * Let's start by calling these "items" for now to distinguish
 * them from other support Components used in the configuration UI
 * which are full Juce Components.
 */

#pragma once

#include <JuceHeader.h>

class BaseItem : public juce::Component
{
  public:
    
    BaseItem(const char* type) {
        type = argType;
    }

    ~BaseItem() {
    }

  pritected:

    const char* type;

};

   

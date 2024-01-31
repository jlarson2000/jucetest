/**
 * ConfigPanel to edit track setups
 */

#pragma once

#include <JuceHeader.h>

#include "ConfigPanel.h"
#include "Form.h"

class GlobalPanel : public ConfigPanel 
{
  public:
    GlobalPanel();
    ~GlobalPanel();

    // Button::Listener
    void buttonClicked(juce::Button* b);

  private:

    FieldSet fields;
    
};

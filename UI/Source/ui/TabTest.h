/**
 * Simple component to contain random tests.
 */

#pragma once

#include <JuceHeader.h>

#include "common/SimpleTabPanel.h"
#include "common/ButtonBar.h"
#include "config/BindingTargetPanel.h"

class TabTest : public juce::Component, public ButtonBar::Listener
{
  public:

    TabTest();
    ~TabTest() override;

    void show();
    void center();

    // Component
    void resized() override;
    void paint (juce::Graphics& g) override;

    void buttonClicked(juce::String name);

  private:

    SimpleTabPanel tabs;
    BindingTargetPanel targets;
    ButtonBar buttons;
    
};

/**
 * Simple component to contain random tests.
 */

#pragma once

#include <JuceHeader.h>

#include "SimpleTabPanel.h"
#include "BindingTargetPanel.h"
#include "ButtonBar.h"

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

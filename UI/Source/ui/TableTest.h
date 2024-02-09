/**
 * Simple component to contain random tests.
 */

#pragma once

#include <JuceHeader.h>

#include "JLabel.h"
#include "SimpleTable.h"

class TableTest : public juce::Component
{
  public:

    TableTest();
    ~TableTest() override;

    void show();
    void center();

    // Component
    void resized() override;
    void paint (juce::Graphics& g) override;

  private:

    SimpleTable table;
    
};

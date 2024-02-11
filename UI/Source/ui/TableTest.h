/**
 * Simple component to contain random tests.
 */

#pragma once

#include <JuceHeader.h>

#include "JLabel.h"
#include "SimpleTable.h"

class TableTest : public juce::Component, public SimpleTable::Listener
{
  public:

    TableTest();
    ~TableTest() override;

    void show();
    void center();

    // Component
    void resized() override;
    void paint (juce::Graphics& g) override;

    void tableTouched(SimpleTable* t);
    
  private:

    SimpleTable table;
    
};

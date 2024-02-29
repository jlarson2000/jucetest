/**
 * ConfigPanel to edit scripts
 */

#pragma once

#include <JuceHeader.h>

#include "ConfigPanel.h"
#include "SampleTable.h"

class SamplePanel : public ConfigPanel 
{
  public:
    SamplePanel(class ConfigEditor*);
    ~SamplePanel();

    // overloads called by ConfigPanel
    void load();
    void save();
    void cancel();

    void resized() override;

  private:

    SampleTable table;
    juce::String lastFolder;

};

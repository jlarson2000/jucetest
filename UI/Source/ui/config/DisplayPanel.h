/**
 * ConfigPanel to edit scripts
 */

#pragma once

#include <JuceHeader.h>

#include "../common/Form.h"
#include "ConfigPanel.h"

class DisplayPanel : public ConfigPanel 
{
  public:
    DisplayPanel(class ConfigEditor*);
    ~DisplayPanel();

    // overloads called by ConfigPanel
    void load();
    void save();
    void cancel();

  private:

    juce::Label label {"Displays"};

};

/**
 * ConfigPanel to edit scripts
 */

#pragma once

#include <JuceHeader.h>

#include "../common/Form.h"
#include "ConfigPanel.h"

class ScriptPanel : public ConfigPanel 
{
  public:
    ScriptPanel(class ConfigEditor*);
    ~ScriptPanel();

    // overloads called by ConfigPanel
    void load();
    void save();
    void cancel();

  private:

    juce::Label label {"Scripts"};

};

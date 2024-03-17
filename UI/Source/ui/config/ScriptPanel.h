/**
 * ConfigPanel to edit scripts
 */

#pragma once

#include <JuceHeader.h>

#include "ConfigPanel.h"
#include "ScriptTable.h"

class ScriptPanel : public ConfigPanel 
{
  public:
    ScriptPanel(class ConfigEditor*);
    ~ScriptPanel();

    // overloads called by ConfigPanel
    void load();
    void save();
    void cancel();

    void resized() override;

  private:

    ScriptTable table;
    juce::String lastFolder;

};

/**
 * ConfigPanel to edit scripts
 */

#pragma once

#include <JuceHeader.h>

#include "../common/Form.h"
#include "ConfigPanel.h"

class SamplePanel : public ConfigPanel 
{
  public:
    SamplePanel(class ConfigEditor*);
    ~SamplePanel();

    // overloads called by ConfigPanel
    void load();
    void save();
    void cancel();

  private:

};

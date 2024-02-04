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
    GlobalPanel(class ConfigEditor*);
    ~GlobalPanel();

    // overloads called by ConfigPanel
    void load();
    void save();
    void cancel();

  private:

    FieldGrid fields;
    
};

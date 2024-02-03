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

    void show();
    void save();
    void cancel();
    void revert();
    bool isActive();
    
  private:

    FieldSet fields;
    
};

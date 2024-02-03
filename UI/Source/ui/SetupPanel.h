/**
 * ConfigPanel to edit track setups
 */

#pragma once

#include <JuceHeader.h>

#include "ConfigPanel.h"
#include "Form.h"

class SetupPanel : public ConfigPanel 
{
  public:
    SetupPanel(class ConfigEditor *);
    ~SetupPanel();

    void show();
    void save();
    void cancel();
    void revert();
    bool isActive();

  private:

    FieldSet fields;
    
};

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
    GlobalPanel();
    ~GlobalPanel();

  private:

    FieldSet fields;
    
};

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
    SetupPanel();
    ~SetupPanel();

  private:

    Form form;

};

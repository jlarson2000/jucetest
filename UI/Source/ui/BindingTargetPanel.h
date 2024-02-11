
#pragma once

#include <JuceHeader.h>

#include "SimpleTabPanel.h"
#include "SimpleListBox.h"

class BindingTargetPanel : public SimpleTabPanel
{
  public:

    BindingTargetPanel();
    ~BindingTargetPanel();
    void init();
    
  private:

    SimpleListBox functions;


};
    

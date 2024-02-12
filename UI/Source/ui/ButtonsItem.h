/*
 * Main display item that shows command buttons.
 */


#pragma once

#include <JuceHeader.h>

#include "Panel.h"
#include "BaseItem.h"

class ButtonsItem : public BaseItem
{
  public:
    
    ButtonsItem();
    ~ButtonsItem();

    void add(class UIButton* src);

  private:
    
    Panel panel;
};

    

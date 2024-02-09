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

    void render();
    void initForm();
    
    void loadGlobal(class MobiusConfig* c);
    void saveGlobal(class MobiusConfig* c);
    
    Form form;
        
};

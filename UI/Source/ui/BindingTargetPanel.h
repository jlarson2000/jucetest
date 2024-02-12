
#pragma once

#include <JuceHeader.h>

#include "SimpleTabPanel.h"
#include "SimpleListBox.h"

class BindingTargetPanel : public SimpleTabPanel, public SimpleListBox::Listener
{
  public:

    BindingTargetPanel();
    ~BindingTargetPanel();
    void init(class MobiusConfig* config);
    
    bool isValidTarget(juce::String name);
    bool isTargetSelected();
    juce::String getSelectedTarget();
    void showSelectedTarget(juce::String target);
    void deselectTarget();
    
    void selectedRowsChanged(SimpleListBox* box, int lastRow);
        
  private:

    void initBox(SimpleListBox* box);
    void deselectOtherTargets(SimpleListBox* active);
    
    SimpleListBox functions;
    SimpleListBox controls;
    SimpleListBox scripts;
    SimpleListBox parameters;
    SimpleListBox configurations;

    // array of pointers to the above for convenient iteration
    juce::Array<SimpleListBox*> boxes;
};
    

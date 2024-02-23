
#pragma once

#include <JuceHeader.h>

#include "../common/SimpleTabPanel.h"
#include "../common/SimpleListBox.h"

class BindingTargetPanel : public SimpleTabPanel, public SimpleListBox::Listener
{
  public:

    BindingTargetPanel();
    ~BindingTargetPanel();
    
    void configure(class MobiusConfig* config);

    void capture(class Binding* b);
    void select(class Binding* b);
    void reset();
    
    bool isTargetSelected();
    class Target* getSelectedTargetType();
    juce::String getSelectedTargetName();

    // temporary: used only for Button since it doesn't use Binding yet
    bool isValidTarget(juce::String name);
    juce::String getSelectedTarget();
    void showSelectedTarget(juce::String target);
    
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
    

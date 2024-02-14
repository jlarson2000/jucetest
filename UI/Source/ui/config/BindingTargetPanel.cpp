
#include <JuceHeader.h>

#include "../../model/FunctionDef.h"
#include "../../model/Parameter.h"
#include "../../model/MobiusConfig.h"
#include "../../model/Preset.h"
#include "../../model/Setup.h"

#include "BindingTargetPanel.h"

BindingTargetPanel::BindingTargetPanel()
{
    setName("BindingTargetPanel");
}

BindingTargetPanel::~BindingTargetPanel()
{
}

void BindingTargetPanel::init(MobiusConfig* config)
{
    initBox(&functions);
    addTab(juce::String("Functions"), &functions);
    for (int i = 0 ; i < FunctionDefinition::Functions.size() ; i++) {
        // todo, may want a deprecation or bindable flag
        FunctionDefinition* f = FunctionDefinition::Functions[i];
        // no display name for these yet
        functions.add(f->getName());
    }

    initBox(&scripts);
    addTab(juce::String("Scripts"), &scripts);
    // todo: need a registry of loaded scripts
    
    initBox(&controls);
    addTab(juce::String("Controls"), &controls);
    for (int i = 0 ; i < Parameter::Parameters.size() ; i++) {
        // todo, may want a deprecation or bindable flag
        Parameter* p = Parameter::Parameters[i];
        if (p->control)
          controls.add(p->getDisplayableName());
    }
    
    initBox(&parameters);
    addTab(juce::String("Parameters"), &parameters);
    for (int i = 0 ; i < Parameter::Parameters.size() ; i++) {
        // todo, may want a deprecation or bindable flag
        Parameter* p = Parameter::Parameters[i];
        if (!p->control)
          parameters.add(p->getDisplayableName());
    }
    
    initBox(&configurations);
    addTab(juce::String("Configurations"), &configurations);
    
    Preset* presets = config->getPresets();
    while (presets != nullptr) {
        // need to find a better way to do string concatenation
        juce::String item = juce::String("Preset:") + juce::String(presets->getName());
        configurations.add(item);
        presets = presets->getNext();
    }
    
    Setup* setups = config->getSetups();
    while (setups != nullptr) {
        // need to find a better way to do string concatenation
        juce::String item = juce::String("Setup:") + juce::String(setups->getName());
        configurations.add(item);
        setups = setups->getNext();
    }
}

void BindingTargetPanel::initBox(SimpleListBox* box)
{
    box->setMultipleSelectionEnabled(false);
    box->addListener(this);
    boxes.add(box);
}

bool BindingTargetPanel::isTargetSelected()
{
    bool selected = false;
    int tab = tabs.getCurrentTabIndex();
    if (tab >= 0) {
        SimpleListBox* box = boxes[tab];
        selected = (box->getSelectedRow() >= 0);
    }
    return selected;
}

juce::String BindingTargetPanel::getSelectedTarget()
{
    juce::String target;
    
    int tab = tabs.getCurrentTabIndex();
    if (tab >= 0) {
        SimpleListBox* box = boxes[tab];
        int row = box->getSelectedRow();
        if (row >= 0) {
            // cells won't have qualified names
            juce::String cellName = box->getSelectedValue();
            if (tab > 0) {
                // not a function, add a qualifier
            }
            target = cellName;
        }
    }
    return target;
}

void BindingTargetPanel::selectedRowsChanged(SimpleListBox* box, int lastRow)
{
    deselectOtherTargets(box);
    // notify a listener
}

void BindingTargetPanel::deselectOtherTargets(SimpleListBox* active)
{
    for (int i = 0 ; i < boxes.size() ; i++) {
        SimpleListBox* other = boxes[i];
        if (other != active)
          other->deselectAll();
    }
}

void BindingTargetPanel::deselectTarget()
{
    deselectOtherTargets(nullptr);
    showTab(0);
}

/**
 * Adjust the tabs and list boxes to display the
 * desired target.  The format of the name must
 * match what is returned by getSelectedTarget so
 * if you add prefixes there you need to parse them here.
 */
void BindingTargetPanel::showSelectedTarget(juce::String name)
{
    bool found = false;

    // getting some weird lingering state that prevents
    // setSelectedRow after showing the selected tab from
    // highlighting, start by full deselection?
    // yes, this works
    // maybe if the row had been selected previously,
    // we moved to a different tab, then back again it
    // won't show it?  weird and I'm too tired to figure it out
    deselectTarget();
    
    for (int tab = 0 ; tab < tabs.getNumTabs() ; tab++) {
        SimpleListBox* box = boxes[tab];
        
        int numRows = box->getNumRows();
        for (int row = 0 ; row < numRows ; row++) {
            juce::String value = box->getRowValue(row);
            if (name == value) {
                // this is the one
                showTab(tab);
                box->setSelectedRow(row);
                found = true;
                break;
            }
        }
    }

    if (!found) {
        // must have had an invalid name in the config file
        // clear any lingering target
        deselectTarget();
    }
}

/**
 * Tests to see if a target name is valid.
 * Used by binding panels to filter out stale data from
 * the config file.
 * Does the same awkward walk as showSelectedTarget but
 * not worth deifning an iterator at the moment.
 *
 * Could support unqualified names here and add the right
 * qualification?  
 */
bool BindingTargetPanel::isValidTarget(juce::String name)
{
    bool valid = false;

    for (int tab = 0 ; tab < tabs.getNumTabs() ; tab++) {
        SimpleListBox* box = boxes[tab];
        
        int numRows = box->getNumRows();
        for (int row = 0 ; row < numRows ; row++) {
            juce::String value = box->getRowValue(row);
            if (name == value) {
                valid = true;
                break;
            }
        }
    }

    return valid;
}

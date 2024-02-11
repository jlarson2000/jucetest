
#include <JuceHeader.h>

#include "BindingTargetPanel.h"

BindingTargetPanel::BindingTargetPanel()
{
    setName("BindingTargetPanel");
    init();
}

BindingTargetPanel::~BindingTargetPanel()
{
}


void BindingTargetPanel::init()
{
    addTab(juce::String("Functions"), &functions);

    functions.add("Reset");
    functions.add("Record");
    functions.add("Overdub");
}


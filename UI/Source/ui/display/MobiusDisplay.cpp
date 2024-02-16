/*
 * The Mobius status display area.
 * This will be inside MainWindow under the menu bar.
 */

#pragma once

#include <JuceHeader.h>

#include "../../model/UIConfig.h"
#include "../JuceUtil.h"
#include "../MainWindow.h"
#include "Colors.h"
#include "ActionButtons.h"

#include "MobiusDisplay.h"

MobiusDisplay::MobiusDisplay(MainWindow* parent)
{
    setName("MobiusDisplay");
    mainWindow = parent;
    
    addAndMakeVisible(buttons);
    addAndMakeVisible(statusArea);
    addAndMakeVisible(strips);

    localButton.setButtonText("local");
    addAndMakeVisible(localButton);

    setSize(500, 500);
}

MobiusDisplay::~MobiusDisplay()
{
}

/**
 * Configure ourselves after a UIConfig change
 */
void MobiusDisplay::configure(UIConfig* config)
{
    buttons.configure(config);
    statusArea.configure(config);
    strips.configure(config);
    
    resized();
}

void MobiusDisplay::update(MobiusState* state)
{
    statusArea.update(state);
    //strips.update(state);
}

void MobiusDisplay::resized()
{
    juce::Rectangle<int> area = getLocalBounds();

    // leave a gap between the MainWindow menu and the top of the buttons
    area.removeFromTop(20);
    
    // we call layout() rather than resized() to auto-calculate
    // the necessary height for all buttons
    // this doesn't work, the buttons display but the 
    buttons.layout(area);

    int bheight = buttons.getPreferredHeight(area);
    buttons.setBounds(area.removeFromTop(bheight));
    //localButton.setBounds(area.removeFromTop(30));
    
    // TrackStrips will have configurable content
    strips.layout(area);

    area.removeFromBottom(strips.getHeight());

    // what remains is for the status area
    // todo: it's going to be easy for this to overflow
    // think about maximum heights with a viewport or smart
    // truncation?
    statusArea.setBounds(area);
    
    JuceUtil::dumpComponent(this);
}

void MobiusDisplay::paint(juce::Graphics& g)
{
}

//////////////////////////////////////////////////////////////////////
//
// Actions
//
//////////////////////////////////////////////////////////////////////

/**
 * Propagate an action from a subcomponent.
 * This can be from an ActionButton, a TrackStrip control
 * or one of the few StatusArea elements that support actions
 *
 * We have nothing to intercept along the way up.
 */
void MobiusDisplay::doAction(UIAction* action)
{
    mainWindow->doAction(action);
}

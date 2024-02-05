/**
 * A very old experiment that should no longer be interesting
 * Base class for all configuration and information popup dialogs.
 */

#include <JuceHeader.h>

#include "ConfigPopup.h"

ConfigPopup::ConfigPopup()
{
// todo: this is one where I would like the size to be determined
    // by the child components, not the other way around
    setSize (500, 500);
    setAlwaysOnTop(true);
}

ConfigPopup::~ConfigPopup()
{
}

void ConfigPopup::setListener(Listener* l)
{
    listener = l;
}

void ConfigPopup::startup()
{
}

void ConfigPopup::shutdown()
{
}

/**
 * ConfigPanel::Listener
 * Cascaide to our listener.
 * Don't like this
 */
void ConfigPopup::configPanelClosed()
{
    if (listener != nullptr)
      listener->configPopupClosed(this);
}

void ConfigPopup::paint (juce::Graphics& g)
{
    // make it different to watch panel painting for awhile
    g.fillAll (juce::Colours::red);
}

void ConfigPopup::resized()
{
    // nothing todo, inner panel handles it
}

/**
 * Called by MainComponent to center this within the available space
 * We should be able to do this ourselves
 */
void ConfigPopup::center()
{
    int pwidth = getParentWidth();
    int pheight = getParentHeight();
    int mywidth = getWidth();
    int myheight = getHeight();
    
    if (mywidth > pwidth) mywidth = pwidth;
    if (myheight > pheight) myheight = pheight;

    int left = (pwidth - mywidth) / 2;
    int top = (pheight - myheight) / 2;
    
    setTopLeftPosition(left, top);
}

//////////////////////////////////////////////////////////////////////
//
// PresetPopup
//
//////////////////////////////////////////////////////////////////////

PresetPopup::PresetPopup()
{
    addAndMakeVisible(panel);
    // who should do this, us or the base class?
    setSize(500, 500);

    // ask the panel to tell us when it is done
    panel.setListener(this);
}

PresetPopup::~PresetPopup()
{
}

//////////////////////////////////////////////////////////////////////
//
// SetupPopup
//
//////////////////////////////////////////////////////////////////////

SetupPopup::SetupPopup()
{
    addAndMakeVisible(panel);
    // who should do this, us or the base class?
    setSize(500, 500);

    // ask the panel to tell us when it is done
    panel.setListener(this);
}

SetupPopup::~SetupPopup()
{
}

//////////////////////////////////////////////////////////////////////
//
// GlobalPopup
//
//////////////////////////////////////////////////////////////////////

GlobalPopup::GlobalPopup()
{
    addAndMakeVisible(panel);
    // who should do this, us or the base class?
    setSize(500, 500);

    // ask the panel to tell us when it is done
    panel.setListener(this);
}

GlobalPopup::~GlobalPopup()
{
}

void GlobalPopup::startup()
{
}

void GlobalPopup::shutdown()
{
}

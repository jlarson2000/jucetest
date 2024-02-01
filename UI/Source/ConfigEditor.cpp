/**
 * Base class for all configuration and information popup editors.
 */

#include <JuceHeader.h>

#include "ConfigEditor.h"

ConfigEditor::ConfigEditor(juce::Component* argOwner)
{
    owner = argOwner;
}

ConfigEditor::~ConfigEditor()
{
}

void ConfigEditor::resized()
{
    // we don't change our size, but we will
    // center relative to the parent

    juce::Component* panel = getPanel();
    if (panel != nullptr) {
        int pwidth = owner->getWidth();
        int pheight = owner->getHeight();
        int mywidth = panel->getWidth();
        int myheight = panel->getHeight();
    
        if (mywidth > pwidth) mywidth = pwidth;
        if (myheight > pheight) myheight = pheight;

        int left = (pwidth - mywidth) / 2;
        int top = (pheight - myheight) / 2;
    
        panel->setTopLeftPosition(left, top);
    }
}
   
void ConfigEditor::open()
{
    ConfigPanel* panel = getPanel();
    if (panel != nullptr) {
        if (!initialized ) {
            // add directly to the owner
            owner->addAndMakeVisible(panel);

            panel->setSize(500, 500);

            panel->setAlwaysOnTop(true);

            // start off centered
            resized();

            // called by ConfigPanel when a button is clicked
            panel->setListener(this);
            initialized = true;
        }
    
        panel->setVisible(true);
        resized();
    }
}

void ConfigEditor::close()
{
    if (initialized) {
        // don't remove it but make it invisible
        juce::Component* panel = getPanel();
        if (panel != nullptr) {
            // todo: ask the panel to flush state
            panel->setVisible(false);
        }
        // leave initialized
    }
}

/**
 * Panel button callback
 */
void ConfigEditor::configPanelClosed(ConfigPanelButton button)
{
    close();
}

//////////////////////////////////////////////////////////////////////
//
// GlobalEditor
//
//////////////////////////////////////////////////////////////////////

GlobalEditor::GlobalEditor(juce::Component* argOwner) :
    ConfigEditor(argOwner)
{
}

GlobalEditor::~GlobalEditor()
{
}

//////////////////////////////////////////////////////////////////////
//
// PresetEditor
//
//////////////////////////////////////////////////////////////////////

PresetEditor::PresetEditor(juce::Component* argOwner) :
    ConfigEditor(argOwner)
{
}

PresetEditor::~PresetEditor()
{
}

//////////////////////////////////////////////////////////////////////
//
// SetupEditor
//
//////////////////////////////////////////////////////////////////////

SetupEditor::SetupEditor(juce::Component* argOwner) :
    ConfigEditor(argOwner)
{
}

SetupEditor::~SetupEditor()
{
}

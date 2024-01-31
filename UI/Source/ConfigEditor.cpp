/**
 * Base class for all configuration and information popup editors.
 */

#include <JuceHeader.h>

#include "ConfigEditor.h"

ConfigEditor::ConfigEditor(juce::Component argOwner)
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
    if (!initialized) {
        ConfigPanel* panel = getPanel();
        if (panel != nullptr) {
            // add directly to the owner
            owner->addAndMakeVisible(panel);

            panel->setSize(500, 500);

            // ask the panel to tell us when it is done
            panel.setListener(this);

            // start off centered
            resized();

            // called by ConfigPanel when a button is clicked
            panel->setListener(this);
        }
        else {
            panel.setVisible(true);
            resized();
        }
    }
    initialized = true;
}

void GlobalEditor::close()
{
    if (initialized) {
        // don't remove it but make it invisible
        juce::Component* panel = getPanel();
        if (panel != nullptr) {
            // todo: ask the panel to flush state
            panel->setVisible(false);
        }
    }
}

/**
 * Panel button callback
 */
void ConfigEditor::configPanelClosed(ConfigPanelButton button)
{
}

//////////////////////////////////////////////////////////////////////
//
// GlobalEditor
//
//////////////////////////////////////////////////////////////////////

GlobalEditor::GlobalEditor(jucce::Component argOwner) :
    ConfigEditor(argOwner)
{
}

GlobalEditor::~GlobalEditor()
{
}

juce::Component GlobalEditor::getPanel()
{
    return &panel;
}

//////////////////////////////////////////////////////////////////////
//
// PresetEditor
//
//////////////////////////////////////////////////////////////////////

PresetEditor::PresetEditor(jucce::Component argOwner) :
    ConfigEditor(argOwner)
{
}

PresetEditor::~PresetEditor()
{
}

juce::Component* PresetEditor::getPanel()
{
    return &panel;
}

//////////////////////////////////////////////////////////////////////
//
// SetupEditor
//
//////////////////////////////////////////////////////////////////////

SetupEditor::SetupEditor(jucce::Component argOwner) :
    ConfigEditor(argOwner)
{
}

SetupEditor::~SetupEditor()
{
}

juce::Component* SetupEditor::getPanel()
{
    return &panel;
}

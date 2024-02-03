/**
 * Base class for all configuration and information popup dialogs.
 */

#include <JuceHeader.h>

#include "../util/FileUtil.h"
#include "../model/MobiusConfig.h"
#include "../model/XmlRenderer.h"

#include "ConfigEditor.h"
#include "ConfigPanel.h"

ConfigPanel::ConfigPanel(ConfigEditor* argEditor, const char* titleText, int buttons)
    : header{titleText}, footer{this,buttons}
{
    editor = argEditor;

    addAndMakeVisible(header);
    addAndMakeVisible(footer);
    addAndMakeVisible(content);
    
    // todo: this is one where I would like the size to be determined
    // by the child components, not the other way around
    // this is initial size, the parent component may change it
    setSize (500, 500);
}

ConfigPanel::~ConfigPanel()
{
}

/**
 * Called by the footer when a button is clicked
 */
void ConfigPanel::buttonClicked(ConfigPanelButton button)
{
    switch (button) {
        case (ConfigiPanelButton::Ok):
        case (ConfigiPanelButton::Save): {
            save();
        }
        break;
        case (ConfigPanelButton::Cancel): {
            cancel();
        }
        break;
        case (ConfigPanelButton::Revert): {
            revert();
        }
        break;
    }

    editor->close(this);
}

void ConfigPanel::resized()
{
    auto area = getLocalBounds();
    
    header.setBounds(area.removeFromTop(ConfigPanelHeader::PreferredHeight));
    footer.setBounds(area.removeFromBottom(ConfigPanelFooter::PreferredHeight));

    content.setBounds(area);
}

void ConfigPanel::center()
{
    // we don't change our size, but we will
    // center relative to the parent
// copied from ConfigEditor when it was a component
/*
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
*/
}

void ConfigPanel::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::yellow);
}

//////////////////////////////////////////////////////////////////////
//
// Header
//
//////////////////////////////////////////////////////////////////////

ConfigPanelHeader::ConfigPanelHeader(const char* titleText)
{
    addAndMakeVisible (titleLabel);
    titleLabel.setFont (juce::Font (16.0f, juce::Font::bold));
    titleLabel.setText (titleText, juce::dontSendNotification);
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::black);
    titleLabel.setJustificationType (juce::Justification::centred);
}


ConfigPanelHeader::~ConfigPanelHeader()
{
}

void ConfigPanelHeader::resized()
{
    // do we need to adjust bounds here?  just let it oriented to upper left
    titleLabel.setBounds(getLocalBounds());
}

void ConfigPanelHeader::paint(juce::Graphics& g)
{
    g.fillAll (juce::Colours::blue);
}

//////////////////////////////////////////////////////////////////////
//
// Footer
//
//////////////////////////////////////////////////////////////////////

ConfigPanelFooter::ConfigPanelFooter(ConfigPanel* parentPanel, int buttons)
{
    if (buttons & ConfigPanelButton::Ok) {
        addButton(&okButton, "Ok");
    }
    
    if (buttons & ConfigPanelButton::Save) {
        addButton(&saveButton, "Save");
    }
    
    if (buttons & ConfigPanelButton::Cancel) {
        addButton(&cancelButton, "Cancel");
    }

    panel = parentPanel;
    buttonList = buttons;
}

ConfigPanelFooter::~ConfigPanelFooter()
{
}

void ConfigPanelFooter::addButton(juce::TextButton* button, const char* text)
{
    addAndMakeVisible(button);
    button->setButtonText(text);
    button->addListener(this);
}

void ConfigPanelFooter::resized()
{
    auto area = getLocalBounds();
    const int buttonWidth = 100;

    // seems like centering should be easier...
    int numButtons = 0;
    if (buttonList & ConfigPanelButton::Ok) numButtons++;
    if (buttonList & ConfigPanelButton::Save) numButtons++;
    if (buttonList & ConfigPanelButton::Cancel) numButtons++;

    int totalWidth = area.getWidth();
    int buttonsWidth = buttonWidth * numButtons;
    int leftOffset = (totalWidth - buttonsWidth) / 2;
    area.removeFromLeft(leftOffset);
    
    if (buttonList & ConfigPanelButton::Ok) {
        okButton.setBounds(area.removeFromLeft(buttonWidth));
    }
    
    if (buttonList & ConfigPanelButton::Save) {
        saveButton.setBounds(area.removeFromLeft(buttonWidth));
    }
    
    if (buttonList & ConfigPanelButton::Cancel) {
        cancelButton.setBounds(area.removeFromLeft(buttonWidth));
    }
}

void ConfigPanelFooter::paint(juce::Graphics& g)
{
    // buttons will draw themselves in whatever the default color is
    g.fillAll (juce::Colours::white);
}

/**
 * juce::Button::Listener
 * Forward to the parent
 */
void ConfigPanelFooter::buttonClicked(juce::Button* b)
{
    // find a better way to do this, maybe subclassing Button
    if (b == &okButton) {
        panel->buttonClicked(ConfigPanelButton::Ok);
    }
    else if (b == &saveButton) {
        panel->buttonClicked(ConfigPanelButton::Save);
    }
    else if (b == &cancelButton) {
        panel->buttonClicked(ConfigPanelButton::Cancel);
    }
}

//////////////////////////////////////////////////////////////////////
//
// Content
//
//////////////////////////////////////////////////////////////////////

ContentPanel::ContentPanel()
{
}

ContentPanel::~ContentPanel()
{
}

void ContentPanel::resized()
{
}

void ContentPanel::paint(juce::Graphics& g)
{
}


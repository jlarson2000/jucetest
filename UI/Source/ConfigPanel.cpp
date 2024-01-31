/**
 * Base class for all configuration and information popup dialogs.
 */

#include <JuceHeader.h>

#include "ConfigPanel.h"

ConfigPanel::ConfigPanel(const char* titleText, int buttons)
    : header{titleText}, footer{this,buttons}
{
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

void ConfigPanel::setListener(Listener* l)
{
    listener = l;
}

/**
 * Called by the footer when a button is clicked
 */
void ConfigPanel::buttonClicked(ConfigPanelButton button)
{
    if (listener != nullptr)
      listener->configPanelClosed(button);
}

void ConfigPanel::resized()
{
    auto area = getLocalBounds();
    
    header.setBounds(area.removeFromTop(ConfigPanelHeader::PreferredHeight));
    footer.setBounds(area.removeFromBottom(ConfigPanelFooter::PreferredHeight));

    content.setBounds(area);
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

ContentPanel::ContentPanel() : tabs {juce::TabbedButtonBar::Orientation::TabsAtTop}
{
}

ContentPanel::~ContentPanel()
{
}

void ContentPanel::resized()
{
    //group.setBounds(getLocalBounds());

    // from the demo
    // if (tabBarIndent > 0)
    // getTabbedButtonBar().setBounds (getTabbedButtonBar().getBounds().withTrimmedLeft (tabBarIndent));
    tabs.setBounds(getLocalBounds());

    // subclasses can toss whatever they want in here
    // hmm, might be better to subclass the content panel?
    // no, at the point of initial sizing, there will be no children
    // have to do this later, doesn't Juce iterate over children automatically?
    /*
    const juce::Array <Component * > & children = getChildren();
    for (int i = 0 ; i < children.size() ; i++) {
        children[i]->setBounds(getLocalBounds());
    }
    */
    
}

void ContentPanel::paint(juce::Graphics& g)
{
}

void ContentPanel::addTab(const char* name)
{
    if (numTabs == 0) {
        addAndMakeVisible(tabs);
        tabs.setTabBarDepth (40);
    }
    numTabs++;
        
    tabs.addTab(name, juce::Colours::black, nullptr, false);
}
//////////////////////////////////////////////////////////////////////
//
// Subclass Builders
//
//////////////////////////////////////////////////////////////////////

void ConfigPanel::addTab(const char* name)
{
    content.addTab(name);
}


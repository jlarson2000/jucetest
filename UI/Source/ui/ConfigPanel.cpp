/**
 * Base class for all configuration and information popup dialogs.
 */

#include <JuceHeader.h>

#include "ConfigEditor.h"
#include "ConfigPanel.h"

ConfigPanel::ConfigPanel(ConfigEditor* argEditor, const char* titleText, int buttons, bool multi)
    : header{titleText}, footer{this,buttons}, objectSelector{this}
{
    setName("ConfigPanel");
    editor = argEditor;

    addAndMakeVisible(header);
    addAndMakeVisible(footer);
    addAndMakeVisible(content);

    if (multi) {
        hasObjectSelector = true;
        addAndMakeVisible(objectSelector);
    }
    
    // size has to be deferred to the subclass after it has finished rendering
    //setSize (500, 500);
}

ConfigPanel::~ConfigPanel()
{
}

/**
 * Called by the footer when a button is clicked
 */
void ConfigPanel::footerButtonClicked(ConfigPanelButton button)
{
    switch (button) {
        case (ConfigPanelButton::Ok):
        case (ConfigPanelButton::Save): {
            save();
        }
        break;
        case (ConfigPanelButton::Cancel): {
            cancel();
        }
        break;
    }

    // ConfigEditor will decide whether to save the
    // MobiusConfig if it has nothing else active
    editor->close(this, (button == ConfigPanelButton::Cancel));
}

void ConfigPanel::resized()
{
    auto area = getLocalBounds();
    
    header.setBounds(area.removeFromTop(header.getPreferredHeight()));
    if (hasObjectSelector) {
        objectSelector.setBounds(area.removeFromTop(objectSelector.getPreferredHeight()));
    }
    footer.setBounds(area.removeFromBottom(footer.getPreferredHeight()));

    content.setBounds(area);
}

/**
 * ConfigPanels are not at the moment resizeable, but they
 * can auto-center within the parent.
 */
void ConfigPanel::center()
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

void ConfigPanel::paint (juce::Graphics& g)
{
    // temporary, give it an obvious background while we play with positioning
    g.fillAll (juce::Colours::yellow);
}

//////////////////////////////////////////////////////////////////////
//
// Header
//
//////////////////////////////////////////////////////////////////////

ConfigPanelHeader::ConfigPanelHeader(const char* titleText)
{
    setName("ConfigPanelHeader");
    addAndMakeVisible (titleLabel);
    titleLabel.setFont (juce::Font (12.0f, juce::Font::bold));
    titleLabel.setText (titleText, juce::dontSendNotification);
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::black);
    titleLabel.setJustificationType (juce::Justification::centred);
}


ConfigPanelHeader::~ConfigPanelHeader()
{
}

int ConfigPanelHeader::getPreferredHeight()
{
    // todo: ask the title font
    return 30;
}

void ConfigPanelHeader::resized()
{
    // let it fill the entire area
    titleLabel.setBounds(getLocalBounds());
}

void ConfigPanelHeader::paint(juce::Graphics& g)
{
    // give it an obvious background
    // need to work out  borders
    g.fillAll (juce::Colours::blue);
}

//////////////////////////////////////////////////////////////////////
//
// Footer
//
//////////////////////////////////////////////////////////////////////

ConfigPanelFooter::ConfigPanelFooter(ConfigPanel* parent, int buttons)
{
    setName("ConfigPanelFooter");
    parentPanel = parent;
    buttonList = buttons;

    if (buttons & ConfigPanelButton::Ok) {
        addButton(&okButton, "Ok");
    }
    
    if (buttons & ConfigPanelButton::Save) {
        addButton(&saveButton, "Save");
    }
    
    if (buttons & ConfigPanelButton::Cancel) {
        addButton(&cancelButton, "Cancel");
    }

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

int ConfigPanelFooter::getPreferredHeight()
{
    // todo: more control over the internal button sizes
    return 36;
}

void ConfigPanelFooter::resized()
{
    auto area = getLocalBounds();
    const int buttonWidth = 100;

    // seems like centering should be easier...
    // don't really need to deal with having both Ok and Save there
    // will only ever be two
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
        parentPanel->footerButtonClicked(ConfigPanelButton::Ok);
    }
    else if (b == &saveButton) {
        parentPanel->footerButtonClicked(ConfigPanelButton::Save);
    }
    else if (b == &cancelButton) {
        parentPanel->footerButtonClicked(ConfigPanelButton::Cancel);
    }
}

//////////////////////////////////////////////////////////////////////
//
// Content
//
// Nothing really to do here.  If all subclasses just have a single
// component could do away with this, but it is a nice spot to leave
// the available area.
//
//////////////////////////////////////////////////////////////////////

ContentPanel::ContentPanel()
{
    setName("ContentPanel");
}

ContentPanel::~ContentPanel()
{
}

void ContentPanel::resized()
{
    // assume subclass added a single child
    Component* child = getChildComponent(0);
    child->setSize(getWidth(), getHeight());
}

void ContentPanel::paint(juce::Graphics& g)
{
    g.fillAll (juce::Colours::beige);
}

//////////////////////////////////////////////////////////////////////
//
// ObjectSelector  TODO
//
//////////////////////////////////////////////////////////////////////

ObjectSelector::ObjectSelector(ConfigPanel* parent)
{
    setName("ObjectSelector");
    parentPanel = parent;
}

ObjectSelector::~ObjectSelector()
{
}

int ObjectSelector::getPreferredHeight()
{
    return 0;
}

void ObjectSelector::resized()
{
}

void ObjectSelector::paint(juce::Graphics& g)
{
}

/**
 * Called by the ConfigPanel subclass to set the names
 * to display in the combobox.
 * When the combobox is changed we call the selectObject overload.
 * This also auto-selects the first name in the list.
 */
void ObjectSelector::setObjectNames(juce::Array<juce::String> names)
{
}

/**
 * Juce listener for the object management buttons.
 */
void ObjectSelector::buttonClicked(juce::Button* b)
{
    if (b == &newButton) {
        parentPanel->newObject();
    }
    else if (b == &deleteButton) {
        parentPanel->deleteObject();
    }
    else if (b == &copyButton) {
        parentPanel->copyObject();
    }
    else if (b == &revertButton) {
        parentPanel->revertObject();
    }
}

// TODO: give the name label a listener to call renameObject
// TODO: give the combobox a listener to call selectObject

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

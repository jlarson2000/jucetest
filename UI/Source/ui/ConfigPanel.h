/**
 * Base component class for all configuration editors.
 * Managed by the ConfigEditor which receives instructions from the MainComponent
 * about what to display.
 *
 * Initially these will be simple Juce components with visibility
 * controlled by ConfigEditor.  I might want to make these true
 * dialog windows someday, but there is strong advice against that
 * and this is easier.
 *
 * A config panel contrains a header area at the top with a title, and a footer
 * area at the bottom with a set of configurable buttons.
 *
 * The content area is in the middle where the subclasses places
 * appropriate editing fields.
 *
 * For config object types that support multiple objects (presets, setups, bindinds)
 * there may be an optional object selector that displays the names of the available
 * objects and buttons for operating on them.
 */

#pragma once

#include <JuceHeader.h>

/**
 * Types of buttons the popup may display at the bottom.
 * These are a bitmask that can be ORd to define the desired buttons.
 * I tried encapsulating putting this inside ConfigPanel but since it needs to be
 * visible to both ConfigPanel and ConfigPanelFooter you get into a typical circular
 * dependency nightmare.
 *
 * There must be a better way to do this, probably with inner classes.
 */
enum ConfigPanelButton {
    // read-only informational panels will have an Ok rather than a Save button
    Ok = 1,
    Save = 2,
    Cancel = 4
};

class ConfigPanelHeader : public juce::Component
{
  public:

    ConfigPanelHeader(const char* titleText);
    ~ConfigPanelHeader() override;
    
    void resized() override;
    void paint (juce::Graphics& g) override;
    
    // these can have variable height, but we'll assume they are as wide
    // as the parent
    int getPreferredHeight();
    
  private:
        
    juce::Label titleLabel;
};

class ConfigPanelFooter : public juce::Component, public juce::Button::Listener
{
  public:
        
    ConfigPanelFooter(class ConfigPanel* parentPanel, int buttons);
    ~ConfigPanelFooter() override;
    
    void resized() override;
    void paint (juce::Graphics& g) override;
    
    int getPreferredHeight();

    // Button::Listener
    virtual void buttonClicked(juce::Button* b);
    
  private:
        
    // find a better way to redirect button listeners without needing
    // a back pointer to the parent
    class ConfigPanel* parentPanel;
    int buttonList;
    juce::TextButton okButton;
    juce::TextButton saveButton;
    juce::TextButton cancelButton;
    juce::TextButton revertButton;
    
    void addButton(juce::TextButton* button, const char* text);
};

class ContentPanel : public juce::Component
{
  public:

    ContentPanel();
    ~ContentPanel();

    void resized() override;
    void paint (juce::Graphics& g) override;

  private:

};

/**
 * The object selector presents a combobox to select one of a list
 * of objects.  It also displays the name of the selected object
 * for editing.  Is there such a thing as a combo with editable items?
 * There is a set of buttons for acting on the object list.
 */
class ObjectSelector : public juce::Component
{
  public:

    // should we put revert here or in the footer?
    enum ButtonType {
        New,
        Delete,
        Copy,
        Revert
    };

    ObjectSelector(class ConfigPanel* parent);
    ~ObjectSelector() override;

    void resized() override;
    void paint (juce::Graphics& g) override;
    void buttonClicked(juce::Button* b);

    int getPreferredHeight();

    // set the names to display in the combo box
    // currently reserving "[New]" to mean an object that
    // does not yet have a name
    void setObjectNames(juce::Array<juce::String> names);

  private:

    class ConfigPanel* parentPanel;
    
    juce::ComboBox combobox;
    juce::Label nameEditor;

    juce::TextButton newButton;
    juce::TextButton deleteButton;
    juce::TextButton copyButton;
    juce::TextButton revertButton;
};

/**
 * ConfigPanel arranges the previous generic components and
 * holds object-specific component within the content panel.
 * 
 * It is subclassed by the various configuration panels.
 */
class ConfigPanel : public juce::Component
{
  public:


    ConfigPanel(class ConfigEditor* argEditor, const char* titleText, int buttons, bool multi);
    ~ConfigPanel() override;

    void center();

    // Component
    void resized() override;
    void paint (juce::Graphics& g) override;

    // callback from the footer buttons
    void footerButtonClicked(ConfigPanelButton button);
    
    // callbacks from the object selector
    // could these be pure virtual?
    void selectorButtonClicked(ObjectSelector::ButtonType button);
    void objectSelected(juce::String name);

    bool isLoaded() {
        return loaded;
    }

    bool isChanged() {
        return changed;
    }
    
    // Subclass overloads

    // prepare for this panel to be shown
    virtual void load() = 0;

    // save the all edited objects and prepare to close
    virtual void save() = 0;

    // throw away any changes and prepare to close
    // is this necessary?  could just implement this as revert
    // for all of them
    virtual void cancel() = 0;

    // respond to ObjectSelector buttons if it supports multiple
    virtual void selectObject(int /*ordinal*/) {};
    virtual void newObject() {};
    virtual void deleteObject() {};
    virtual void copyObject() {};
    virtual void revertObject() {};
    virtual void renameObject(juce::String /*newName*/) {};

  protected:
    
    class ConfigEditor* editor;
    ContentPanel content;
    ObjectSelector objectSelector;

    // set by the subclass if state has been loaded
    bool loaded = false;
    
    // set by the subclass if it was shown and there are pending changes
    bool changed = false;
    
  private:

    bool hasObjectSelector = false;;
    ConfigPanelHeader header;
    ConfigPanelFooter footer;
};

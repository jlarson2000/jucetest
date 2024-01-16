/**
 * Base component class for all configuration and information "popup" dialogs.
 * Used inside a component that controls how the content is shown.
 * Initially these will just be simple components with visibility controlled
 * by the parent.  Might want to make these true dialog windows someday but
 * I'm liking having them as components for now.
 *
 * The panel contains a header area at the top with a title, background color and
 * a border.
 *
 * The bottom has a footer with one or more action buttons.
 *
 * The center will have one or more useage specific value components.
 */

#pragma once

#include <JuceHeader.h>

/**
 * Types of buttons the popup may display at the bottom.
 * These are a bitmask that can be ORd to define the desired buttons.
 * I tried encapsulating putting this inside ConfigPanel but since it needs to be
 * visible to both ConfigPanel and ConfigPanelFooter you get into a typical circular
 * dependency nightmare.
 */
enum ConfigPanelButton {
    Ok = 1,
    Save = 2,
    Cancel = 4
};

class ConfigPanelHeader : public juce::Component
{
  public:

    // todo: tutorial had this hard coded, figure out a way to get true font height
    static const int PreferredHeight = 36;
    
    ConfigPanelHeader(const char* titleText);
    ~ConfigPanelHeader() override;
    
    void resized() override;
    void paint (juce::Graphics& g) override;
    
  private:
        
    juce::Label titleLabel;
};

// circular dependency between panel and footer
class ConfigPanel;

class ConfigPanelFooter : public juce::Component, public juce::Button::Listener
{
  public:
        
    // todo: tutorial had this hard coded, figure out a way to get true font height
    static const int PreferredHeight = 36;
    
    ConfigPanelFooter(ConfigPanel* parentPanel, int buttons);

    ~ConfigPanelFooter() override;
    
    void resized() override;
    void paint (juce::Graphics& g) override;
    
    // Button::Listener
    void buttonClicked(juce::Button* b);
    
  private:
        
    // find a better way to redirect button listeners without needing
    // a back pointer to the parent
    ConfigPanel* panel;
    int buttonList;
    juce::TextButton okButton;
    juce::TextButton saveButton;
    juce::TextButton cancelButton;

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

    juce::GroupComponent group;
};

class ConfigPanel : public juce::Component
{
  public:


    /**
     * Interface of the parent that controls opening and
     * closing of the popup.  Tells the parent we are done.
     */
    class Listener {
      public:
        virtual void configPanelClosed() = 0;
    };
    ConfigPanel(const char* titleText, int buttons);
    ~ConfigPanel() override;

    // Component
    void resized() override;
    void paint (juce::Graphics& g) override;

    // local
    void setListener(Listener* listener);

    // callback from the footer buttons
    void buttonClicked(ConfigPanelButton button);
    
  private:

    ConfigPanelHeader header;
    ConfigPanelFooter footer;
    ContentPanel content;
    
    Listener* listener;
    
};
    

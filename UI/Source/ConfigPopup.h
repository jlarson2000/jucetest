/**
 * Base class for all configuration and information "popup" dialogs.
 * Initially these will just be simple components with visibility controlled
 * by the parent.  Might want to make these true dialog windows someday but
 * I'm liking having them as components for now.
 *
 * The guts of what this does is defined by ConfigPanel, we just
 * determine the container to display the panel in.
 *
 * Not liking the nested listener levels but trying to maintain encapsulation
 * Think more about this.
 * 
 */

#pragma once

#include <JuceHeader.h>

#include "ConfigPanel.h"
#include "PresetPanel.h"

class ConfigPopup : public juce::Component, public ConfigPanel::Listener
{
  public:

    class Listener {
      public:
        virtual void configPopupClosed(ConfigPopup* p) = 0;
    };

    ConfigPopup();
    ~ConfigPopup() override;

    void setListener(Listener *l);

    // Component
    void resized() override;
    void paint (juce::Graphics& g) override;

    // local
    void center();

    void configPanelClosed();
    
  private:

    Listener* listener;
    
};

class PresetPopup : public ConfigPopup
{
  public:

    PresetPopup();
    ~PresetPopup();

  private:

    PresetPanel panel;

};

class SetupPopup : public ConfigPopup
{
  public:

    SetupPopup();
    ~SetupPopup();

  private:

    SetupPanel panel;

};

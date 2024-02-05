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
 * The owner component calls show() when it wants the dialog to show.
 * If this is implemented as a child Component it will be added and made visible.
 * If this is implemented as a window the window is created and shown.
 *
 * The owner assumes this an async dialog and will close itself
 *
 * The owner may call close() to force the dialog to cancel whatever it was
 * doing and 
 */

#pragma once

#include <JuceHeader.h>

#include "ConfigPanel.h"
#include "PresetPanel.h"
#include "SetupPanel.h"
#include "GlobalPanel.h"

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
    
    // initialize before showing
    void startup();
    void shutdown();

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

class GlobalPopup : public ConfigPopup
{
  public:

    GlobalPopup();
    ~GlobalPopup();

    void startup();

  private:

    GlobalPanel panel;

};

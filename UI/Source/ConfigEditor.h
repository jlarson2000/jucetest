/**
 * The base class of an object responsible for editing some portion of the
 * Mobius configuration.  The implementation of the editor is up to the editor,
 * the owner should not care.
 *
 * It may either be a true popup dialog window, or simply a Component that is added
 * to the owning component.
 *
 * ConfigEditor::open
 *
 * Called by the owning component to display the dialog.
 * If the editor is not currently visible, it is expected to load fresh state from Mobius.
 * If the editor is already visible could just let it continue or force a reload.
 *
 * ConfigEditor::close
 *
 * Called by the owning component to force the editor to close and abandon whatever
 * it was doing.  Editors typically close themselves when a Save/Cancel button is pressed.
 * close() may be called to enforce non-concurency or simply when shutting down.
 *
 * ConfigEditor::Listener
 *
 * Interface that may be registered, usually by the owning component to take
 * action when the editor is closed.  Not sure this is necessary.
 *
 * Concurrency can't easily be handled by the editor since they won't know about each
 * other.  Could have a ConfigEditor::allowConcurrent flag to let the owner
 * know what to do.
 *
 */

#pragma once

#include <JuceHeader.h>

#include "ConfigPanel.h"
#include "PresetPanel.h"
#include "SetupPanel.h"
#include "GlobalPanel.h"

class ConfigEditor : public ConfigPanel::Listener
{
  public:

    // hmm, should we carry this with us from construction or should
    // this be passed in show()?
    ConfigEditor(juce::Component* owner);
    ~ConfigEditor();

    void open();
    void close();

    // called by the owner when it is resized
    void resized();
    
    virtual class ConfigPanel* getPanel() = 0;

    // called by ConfigPanel when a button is clicked
    void configPanelClosed(ConfigPanelButton button);
    
  private:

    juce::Component* owner;
    bool active;  // true if this is currently open
    
};

class PresetEditor : public ConfigEditor
{
  public:

    PresetEditor(juce::Component* owner);
    ~PresetEditor();

  private:

    PresetPanel panel;

};

class SetupEditor : public ConfigEditor
{
  public:

    SetupEditor(juce::Component* owner);
    ~SetupEditor();

  private:

    SetupPanel panel;

};

class GlobalEditor : public ConfigEditor
{
  public:

    GlobalEditor(juce::Component owner);
    ~GlobalEditor();


  private:

    GlobalPanel panel;

};

/**
 * Mobius main menu.
 * A wrapper around Juce MenuBarComponent that tries to simplify some things.
 * This will build out a specific menu it is not configurable.
 * Owners should implmenet our Listener interface which just forwards
 * things from MenuBarModel.
 */

#pragma once

#include <JuceHeader.h>

class MainMenu : public juce::Component, public juce::MenuBarModel
{
  public:

    /**
     * Interface of the thing that wants to receive menu events
     */
    class Listener {
      public:
        virtual void mainMenuSelection(int id) = 0;
    };

    // these are indexes not ids so must start from zero
    enum MenuIndexes {
        menuIndexFile = 0,
        menuIndexSetup,
        menuIndexPreset,
        menuIndexConfig,
        menuIndexHelp
    };
    
    const char* FILE_MENU_NAME = "File";
    const char* SETUP_MENU_NAME = "Track Setups";
    const char* PRESET_MENU_NAME = "Presets";
    const char* CONFIG_MENU_NAME = "Configuration";
    const char* HELP_MENU_NAME = "Help";

    /**
     * Offset of menu item ids for the generated track setup items
     * Figure out a better way to do this so we don't overflow
     */
    const int MenuSetupOffset = 100;
    
    /**
     * Offset of menu item ids for the generated preset items
     */
    const int MenuPresetOffset = 200;

    /**
     * These are menu item model ids which must begin from 1
     */
    enum MenuItems {
        
        // File
        OpenLoop = 1,
        OpenProject,
        SaveLoop,
        SaveProject,
        QuickSave,
        ReloadScripts,
        //ReloadOSC,
        Exit,

        // Configuration
        GlobalParameters,
        Presets,
        TrackSetups,
        MidiControl,
        KeyboardControl,
        PluginParamters,
        Buttons,
        DisplayComponents,
        Scripts,
        Samples,
        MidiDevices,
        AudioDevices,
        InstallSamples,

        // Help
        KeyBindings,
        MidiBindings,
        DiagnosticWindow,
        About
        };
    
    MainMenu();
    ~MainMenu() override;

    void resized() override;

    void setListener(Listener* l) {
        listener = l;
    }

    int getPreferredHeight();
    
    // 
    // MenuBarModel
    // 

    juce::StringArray getMenuBarNames() override;
    juce::PopupMenu getMenuForIndex (int menuIndex, const juce::String& /*menuName*/) override;
    void menuItemSelected (int /*menuItemID*/, int /*topLevelMenuIndex*/) override;

  private:

    // demo did it this way not sure why
    //std::unique_ptr<juce::MenuBarComponent> menuBar;
    juce::MenuBarComponent menuBar;

    Listener* listener = nullptr;
    
    juce::StringArray menuNames {FILE_MENU_NAME, SETUP_MENU_NAME, PRESET_MENU_NAME, CONFIG_MENU_NAME, HELP_MENU_NAME};

    int presetCounter = 1;
    
};

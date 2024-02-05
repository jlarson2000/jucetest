/**
 * Main menu simplified from the complicated demo with command handlers
 */

#pragma once

#include <JuceHeader.h>

class SimpleMenu : public juce::Component,
                   public juce::MenuBarModel
{
  public:

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

    enum MenuItems
    {
        // File
        menuOpenLoop = 1,
        menuOpenProject,
        menuSaveLoop,
        menuSaveProject,
        menuQuickSave,
        menuReloadScripts,
        menuReloadOSC,
        menuExit,

        // Configuration
        menuPresets,
        menuTrackSetups,
        menuGlobalParameters,
        menuMIDIControl,
        menuKeyboardControl,
        menuPluginParamters,
        menuButtons,
        menuDisplayComponents,
        menuPalette,
        menuScripts,
        menuSamples,
        menuMIDIDevices,
        menuAudioDevices,

        // Help
        menuKeyBindings,
        menuMIDIBindings,
        menuRefreshUI,
        menuAbout
    };
    
    SimpleMenu();
    ~SimpleMenu() override;
    void resized() override;

    void setMainComponent(class MainComponent* main);
    
    // 
    // MenuBarModel
    // 

    juce::StringArray getMenuBarNames() override;
    juce::PopupMenu getMenuForIndex (int menuIndex, const juce::String& /*menuName*/) override;
    void menuItemSelected (int /*menuItemID*/, int /*topLevelMenuIndex*/) override;

  private:
    
    std::unique_ptr<juce::MenuBarComponent> menuBar;

    int presetCounter = 1;

    class MainComponent* mainComponent;

    
};

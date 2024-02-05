/**
 * A main menu example
 */

#pragma once

#include <JuceHeader.h>

class MainMenu : public juce::Component,
                 public juce::ApplicationCommandTarget,
                 public juce::MenuBarModel
{
  public:

    // A list of the commands that this demo responds to
    enum CommandIDs
    {
        menuPositionInsideWindow = 1,
        menuPositionGlobalMenuBar,
        menuPositionBurgerMenu,
        outerColourRed,
        outerColourGreen,
        outerColourBlue,
        innerColourRed,
        innerColourGreen,
        innerColourBlue
    };

    // Represents the possible menu positions
    enum class MenuBarPosition
    {
        window,
        global,
        burger
    };

    MainMenu();
    ~MainMenu() override;
    void resized() override;
    
    // 
    // MenuBarModel
    // 

    juce::StringArray getMenuBarNames() override;
    juce::PopupMenu getMenuForIndex (int menuIndex, const juce::String& /*menuName*/) override;
    void menuItemSelected (int /*menuItemID*/, int /*topLevelMenuIndex*/) override;

    //
    // ApplicationCommandTarget Interface
    //
    
    juce::ApplicationCommandTarget* getNextCommandTarget() override;
    void getAllCommands (juce::Array<juce::CommandID>& c) override;
    void getCommandInfo (juce::CommandID commandID, juce::ApplicationCommandInfo& result) override;
    bool perform (const juce::ApplicationCommandTarget::InvocationInfo& info) override;
    
  private:
    
    juce::ApplicationCommandManager commandManager;
    std::unique_ptr<juce::MenuBarComponent> menuBar;
};

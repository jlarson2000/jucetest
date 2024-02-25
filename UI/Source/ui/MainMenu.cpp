/**
 * The Mobius main menu.
 */

#include <JuceHeader.h>

#include "../util/trace.h"
#include "MainMenu.h"

/**
 * Construction of the MenuBarComponent is weird.
 * Juce demos did not alloccate this with member objects
 * they used std::unique_ptr and new to make it during
 * the class constructor.  It has something to do with the component
 * wanting to use the model right away and if we implement the model
 * we're not done initializing yet or something.  I tried passing
 * {this} as a member initialization list and got an exception.
 * It works to call setModel in the constructor.
 * I think because MenuComponent starts trying to use the model right away
 * and if you set it to {this} the object isn't done being constructed yet.
 */
MainMenu::MainMenu()
{
    setName("MainMenu");
    
    // demo used a std::unique_ptr instead of just having a local object
    //menuBar.reset (new juce::MenuBarComponent (this));
    //addAndMakeVisible (menuBar.get());

    menuBar.setModel(this);
    addAndMakeVisible(menuBar);

    setSize(500, getPreferredHeight());
}

MainMenu::~MainMenu()
{
}

void MainMenu::resized()
{
    menuBar.setBounds(getLocalBounds());
}

int MainMenu::getPreferredHeight()
{
    return juce::LookAndFeel::getDefaultLookAndFeel().getDefaultMenuBarHeight();
}

//////////////////////////////////////////////////////////////////////
// 
// MenuBarModel
// 
//////////////////////////////////////////////////////////////////////

/**
 * the top level menu bar names
 * there are a variety of constructors for StringArray
 * this one is probaly
 *  	StringArray (const char *const *strings)
 *      Creates a copy of a null-terminated array of string literals.
 */
juce::StringArray MainMenu::getMenuBarNames()
{
    // return {FILE_MENU_NAME, SETUP_MENU_NAME, PRESET_MENU_NAME, CONFIG_MENU_NAME, HELP_MENU_NAME };
    return menuNames;
}

/**
 * first arg is topLevelMenuIndex
 * second is menuName which I guess does not have to be used
 * maybe if you want to have dynamically defined bar names and won't have a fixed index?
 *
 * this is weird because I don't get how resource allocation works
 * I guess it is an automatic local, then it is copy constructed on return?
 *
 * web on object returning
 * https://www.bogotobogo.com/cplusplus/object_returning.php#:~:text=If%20a%20method%20or%20function,a%20reference%20to%20an%20object.
 *
 * If a method or function returns a local object, it should return an object, not a reference.
 * If a method or function returns an object of a class for which there is no public copy constructor, such as ostream class, it must return a reference to an object.
 *
 * it is an error to return a pointer to a local object, so why does returning a reference work?
 * the stack is going to unwind, and without a copy constrctor, where does it go?
 * whatever the answer, PopupMenu does have a copy constrctor
 * 
 * the use of object passing rather than pointers is interesting here
 * you will pay for copying in order to get automatic memory reclamation
 * seems reasonable for smaller objects
 *
 * ahh, it's making sense now
 * popup menus are shown with the show() method which returns the command id
 * of the item selected.  Since MenuBar does it's own show internally, we won't receive
 * the item id.  Can use a callback, surely there is a listener?
 *
 * Yes, experiement with MenuBarModel.menuItemActivated
 */
juce::PopupMenu MainMenu::getMenuForIndex (int menuIndex, const juce::String& menuName)
{
    juce::PopupMenu menu;
    int numPresets = 10;
    int numSetups = 5;
    
    if (menuIndex == menuIndexFile)
    {
        menu.addItem(OpenLoop, "Open Loop...");
        menu.addItem(OpenProject, "Open Project...");
        menu.addItem(SaveLoop, "Save Loop...");
        menu.addItem(SaveProject, "Save Project...");
        menu.addItem(QuickSave, "Quick Save");
        menu.addItem(ReloadScripts, "Reload Scripts");
        //menu.addItem(ReloadOSC, "Reload OSC");
        menu.addItem(Exit, "Exit");
    }
    else if (menuIndex == menuIndexSetup)
    {
        // find a better c++ or juce::String way to do this
        char buf[128];
        for (int i = 0 ; i < numSetups ; i++) {
            sprintf(buf, "Setup %d", i);
            menu.addItem(MenuSetupOffset + i, buf);
        }
    }
    else if (menuIndex == menuIndexPreset)
    {
        char buf[128];
        for (int i = 0 ; i < numPresets ; i++) {
            sprintf(buf, "Preset %d", i + presetCounter);
            juce::PopupMenu::Item item = juce::PopupMenu::Item(juce::String(buf)).setID(MenuPresetOffset + i);
            if ((i % 2) == 0) item.setTicked(true);
            menu.addItem(item);
        }
        presetCounter += numPresets;
    }
    else if (menuIndex == menuIndexConfig)
    {
        menu.addItem(GlobalParameters, "Global Parameters");
        menu.addItem(Presets, "Presets");
        menu.addItem(TrackSetups, "Track Setups");
        menu.addItem(MidiControl, "MIDI Control");
        menu.addItem(KeyboardControl, "Keyboard Control");
        //menu.addItem(PluginParamters, "Plugin Parameters");
        menu.addItem(Buttons, "UI Buttons");
        menu.addItem(DisplayComponents, "Display Configuration");
        // menu.addItem(Palette, "Palette");
        menu.addItem(Scripts, "Scripts");
        menu.addItem(Samples, "Samples");
        menu.addItem(MidiDevices, "MIDI Devices");
        menu.addItem(AudioDevices, "Audio Devices");
    }
    else if (menuIndex == menuIndexHelp)
    {
        menu.addItem(KeyBindings, "Key Bindings");
        menu.addItem(MidiBindings, "MIDI Bindings");
        menu.addItem(DiagnosticWindow, "Diagnostic Window");
        menu.addItem(About, "About");
    }

    return menu;
}

/**
 * MenuBarModel tells us something happened
 * Our listener provides a slightly simpler interface by
 * dispensing with the menuId.
 */
void MainMenu::menuItemSelected (int itemId, int menuId)
{
    if (itemId >= MenuPresetOffset)
    {
        int preset = itemId - MenuPresetOffset;
        // todo: select a preset
        // this needs more than just the Listener we have now
        trace("Preset %d\n", preset);
    }
    else if (itemId >= MenuSetupOffset)
    {
        int setup = itemId - MenuSetupOffset;
        trace("Setup %d\n", setup);
    }
    else {
        if (listener != nullptr)
          listener->mainMenuSelection(itemId);
    }
}



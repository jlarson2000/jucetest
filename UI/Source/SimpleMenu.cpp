/**
 * The main menu.
 */

#include <JuceHeader.h>


#include "SimpleMenu.h"
#include "MainComponent.h"

SimpleMenu::SimpleMenu()
{
    // jsl - interesting, it has to use reset() and new() because the
    // consttor needs to be passed this which isn't known at compile time?
    menuBar.reset (new juce::MenuBarComponent (this));
    addAndMakeVisible (menuBar.get());

    // initial size, this will adjust in resized() for the window size?
    setSize (500, 300);
}

SimpleMenu::~SimpleMenu()
{
}

void SimpleMenu:: resized()
{
    auto b = getLocalBounds();

    // in guess in case the local bounds is taller than it needs to be we shorten it
    menuBar->setBounds (b.removeFromTop (juce::LookAndFeel::getDefaultLookAndFeel()
                                         .getDefaultMenuBarHeight()));
}

/**
 * Should be a better way to do this in the constructor.
 */
void SimpleMenu::setMainComponent(MainComponent* main)
{
    mainComponent = main;
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
juce::StringArray SimpleMenu::getMenuBarNames()
{
    return {FILE_MENU_NAME, SETUP_MENU_NAME, PRESET_MENU_NAME, CONFIG_MENU_NAME, HELP_MENU_NAME };
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
juce::PopupMenu SimpleMenu::getMenuForIndex (int menuIndex, const juce::String& menuName)
{
    juce::PopupMenu menu;
    int numPresets = 10;
    int numSetups = 5;
    
    if (menuIndex == menuIndexFile)
    {
        menu.addItem(menuOpenLoop, "Open Loop...");
        menu.addItem(menuOpenProject, "Open Project...");
        menu.addItem(menuSaveLoop, "Save Loop...");
        menu.addItem(menuSaveProject, "Save Project...");
        menu.addItem(menuQuickSave, "Quick Save");
        menu.addItem(menuReloadScripts, "Reload Scripts");
        menu.addItem(menuReloadOSC, "Reload OSC");
        menu.addItem(menuExit, "Exit");
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
        menu.addItem(menuPresets, "Presets");
        menu.addItem(menuTrackSetups, "Track Setups");
        menu.addItem(menuGlobalParameters, "Global Parameters");
        menu.addItem(menuMIDIControl, "MIDI Control");
        menu.addItem(menuKeyboardControl, "Keyboard Control");
        menu.addItem(menuPluginParamters, "Plugin Parameters");
        menu.addItem(menuButtons, "Buttons");
        menu.addItem(menuDisplayComponents, "Display Components");
        menu.addItem(menuPalette, "Palette");
        menu.addItem(menuScripts, "Scripts");
        menu.addItem(menuSamples, "Samples");
        menu.addItem(menuMIDIDevices, "MIDI Devices");
        menu.addItem(menuAudioDevices, "Audio Devices");
    }
    else if (menuIndex == menuIndexHelp)
    {
        menu.addItem(menuKeyBindings, "Key Bindings");
        menu.addItem(menuMIDIBindings, "MIDI Bindings");
        menu.addItem(menuRefreshUI, "Refresh UI");
        menu.addItem(menuAbout, "About");
    }

    return menu;
}

/**
 * This I guess is a pure virtual we need to override
 * But we ignore it since we're using the command manager for dispatching actions
 *
 * Yes, this would be a lot simpler if you don't need keyboard activation
 */
#include "Trace.h"
void SimpleMenu::menuItemSelected (int itemId, int menuId)
{
    char buf[100];
    sprintf(buf, "Menu %d Item %d\n", menuId, itemId);
    // DBG(buf);
    Trace(buf);

    if (itemId >= MenuPresetOffset)
    {
        int preset = itemId - MenuPresetOffset;
        sprintf(buf, "Preset %d\n", preset);
        Trace(buf);
    }
    else if (itemId >= MenuSetupOffset)
    {
        int setup = itemId - MenuSetupOffset;
        sprintf(buf, "Setup %d\n", setup);
        Trace(buf);
    }
    else
    {
        switch (itemId)
        {
            case menuOpenLoop:
            {
            }
            break;
            case menuOpenProject:
            {
            }
            break;
            case menuSaveLoop: break;
            case menuSaveProject: break;
            case menuQuickSave: break;
            case menuReloadScripts: break;
            case menuReloadOSC: break;
            case menuExit: break;

            case menuPresets: {
                mainComponent->showPresets();
            }
            break;
            
            case menuTrackSetups: {
                mainComponent->showSetups();
            }
            break;
                
            case menuGlobalParameters: break;
            case menuMIDIControl: break;
            case menuKeyboardControl: break;
            case menuPluginParamters: break;
            case menuButtons: break;
            case menuDisplayComponents: break;
            case menuPalette: break;
            case menuScripts: break;
            case menuSamples: break;
            case menuMIDIDevices: break;
            case menuAudioDevices: break;

            case menuKeyBindings: break;
            case menuMIDIBindings: break;
            case menuRefreshUI: break;
                //case menuAbout: break;
            default: {
                Trace("Unknown menu item\n");
            }
            break;
        }
    }
}



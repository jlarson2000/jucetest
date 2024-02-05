 /**
  * A main menu.
  *
  * This was derived from the Juce menu example and/or some simplified
  * version I found on the webz, I think "MenusDemoFlash" which tried to extract the
  * rather complicated Juce demo into a more standalone thing that doesn't deal
  * with side panels and burger menus like the Juce demo.
  *
  * It retains the complexity of ApplicationCommandManager which I don't want
  * just yet so keep this around for reference.
  *
  * This is not used by Mobius
  * 
  */

#include <JuceHeader.h>

#include "MainMenu.h"

MainMenu::MainMenu()
{
    // jsl - interesting, it has to use reset() and new() because the
    // consttor needs to be passed this which isn't known at compile time?
    menuBar.reset (new juce::MenuBarComponent (this));
    addAndMakeVisible (menuBar.get());

    // this is part of MenuBarModel
    // it tells the menu bar to listen to this command manager and to update
    // itself when the commands change
    // also allows it to flash itself when a command from that menu is invoked using a key
    setApplicationCommandManagerToWatch (&commandManager);

    // An ApplicationCommandManager holds a list of all the commands your app can perform,
    // and despatches these commands when needed.  Since this is defined in MainMenu it's
    // scope is only within this menu, I guess you can have these with higher scope
    // and more than one?  
    //
    // from the docs
    // Adds all the commands that this target publishes to the manager's list.
    // This will use ApplicationCommandTarget::getAllCommands() and ApplicationCommandTarget::getCommandInfo()
    // to get details about all the commands that this target can do, and will call registerCommand() to
    // add each one to the manger's list.
    //
    // jsl - basically a very fancy form of callback, that is not dependent on a single event source?
    commandManager.registerAllCommandsForTarget (this);

    // from the demo comments:
    // this ensures that commands invoked on the DemoRunner application are correctly
    // forwarded to this demo
    //
    // from the docs
    // Commands are despatched to ApplicationCommandTarget objects which can choose which events they want
    // to handle
    //
    // jsl - I think this just reorders the target list to make sure that this target
    // is first, may be unnecessary unless more than one thing is using the same manager?
    //
    // also docs
    // If you use this to set a target, make sure you call setFirstCommandTarget (nullptr) before
    // deleting the target object.
    //
    // jsl - so what associates the target with the command manager, registerAll or setFirst?
    // seems redundant
    commandManager.setFirstCommandTarget (this);

    // from the demo
    // this lets the command manager use keypresses that arrive in our window to send out commands
    //
    // this is defined on Component
    // getKeyMappings returns a KeyPressMappingSet
    // KeyPressMappingSet implements KeyListener
    //
    // you appear to be able to have nested key listeners, who gets it probably has to do with focus
    addKeyListener (commandManager.getKeyMappings());

    // defined on Component
    // flag to indiccate component is interested in keyboard focus
    // see grabKeyboardFocus for the way a component is chosen to receive focus
    // docs: When the user clicks on a component or its grabKeyboardFocus() method is called
    // you probably need to click on it, not just mouse over?
    setWantsKeyboardFocus (true);

    // initial size, this will adjust in resized() for the window size?
    setSize (500, 300);
}

MainMenu::~MainMenu()
{
    // yes, this was mentioned in the docs for setFirstCommandTarget
    // not sure why
    commandManager.setFirstCommandTarget (nullptr);
}

void MainMenu:: resized()
{
    auto b = getLocalBounds();

    // in guess in case the local bounds is taller than it needs to be we shorten it
    menuBar->setBounds (b.removeFromTop (juce::LookAndFeel::getDefaultLookAndFeel()
                                         .getDefaultMenuBarHeight()));
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
    return { "Menu Position", "Outer Colour", "Inner Colour" };
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
 */
juce::PopupMenu MainMenu::getMenuForIndex (int menuIndex, const juce::String& menuName)
{
    juce::PopupMenu menu;

    // the demo defines items through a command manager, which I guess is more flexible
    // since it can do both menu selection and keyboard selection
    // you can also just add Item objects which have a string name and a callback function
    // I'm guessing you can skip the callback function and have the containing
    // menu be notified and look up the action by item name
    // 
    // for Mobius, I don't necessarily need keyboard commands for menus so this could be
    // simplified, it may make the dynamic Preset menu harder too since CommandIDs won't be
    // constant
    //
    // the use of object passing rather than pointers is interesting here
    // you will pay for copying in order to get automatic memory reclamation
    // seems reasonable for smaller objects
    //
    // for items using command ids, it gets the name of the item from the getCommandInfo call below 
    
    if (menuIndex == 0)
    {
        menu.addCommandItem (&commandManager, CommandIDs::menuPositionInsideWindow);
#if JUCE_MAC
        menu.addCommandItem (&commandManager, CommandIDs::menuPositionGlobalMenuBar);
#endif
        menu.addCommandItem (&commandManager, CommandIDs::menuPositionBurgerMenu);
    }
    else if (menuIndex == 1)
    {
        menu.addCommandItem (&commandManager, CommandIDs::outerColourRed);
        menu.addCommandItem (&commandManager, CommandIDs::outerColourGreen);
        menu.addCommandItem (&commandManager, CommandIDs::outerColourBlue);
    }
    else if (menuIndex == 2)
    {
        menu.addCommandItem (&commandManager, CommandIDs::innerColourRed);
        menu.addCommandItem (&commandManager, CommandIDs::innerColourGreen);
        menu.addCommandItem (&commandManager, CommandIDs::innerColourBlue);
    }

    return menu;
}

/**
 * This I guess is a pure virtual we need to override
 * But we ignore it since we're using the command manager for dispatching actions
 *
 * Yes, this would be a lot simpler if you don't need keyboard activation
 */
void MainMenu::menuItemSelected (int /*menuItemID*/, int /*topLevelMenuIndex*/)
{
}

//////////////////////////////////////////////////////////////////////
// 
// ApplicationCommandTarget
// 
// from the demo
// The following methods implement the ApplicationCommandTarget interface, allowing
// this window to publish a set of actions it can perform, and which can be mapped
// onto menus, keypresses, etc.
//
// jsl - when I first simplified this I didn't get the outer/inner command targets
// from the demo.  YES command targets can be nested
// and if the code was running inside the demo runner it added things to the
// global command manager, otherwise it created a local one
//
// It used nested components each with it's own command targets to respond
// to the colors
//
// sweet jesus, this was a complicated way to introduce menus
// but I guess it's nice if you want to have dynamic sub components that manage
// their own menus
//
// how do you make the top-level menu bar names varaiable then?
//
// So this didn't work because when MenuBarModel asked for the PopupMenu
// with a top-level bar index, it returned a menu with items that
// used addCommandItem.  Since that manager wasn't initialized
// with commands for the item ids, there was nothing to show.
// something like that...
// 
//////////////////////////////////////////////////////////////////////

/**
 * There can only be one
 * Still not clear on why setFirstCommandTarget was necessary unless
 * that's how the list is built.
 * Apparently registerAllCommandsForTarget does not add it to the list?
 *
 * What about all the other CommandTarget getters, you seem to be able to
 * do more than just first and next
 *
 * oh no, this is the TARGET not the MANAGER
 * the manager has a list of targets but I guess each target can also contain
 * a sublist of targets?
 *
 */
juce::ApplicationCommandTarget* MainMenu::getNextCommandTarget() 
{
    // from the demo
    // return &outerCommandTarget;
    //
    // I guess this is where you would pass up to a parent hierarchical target
    return nullptr;
}

/**
 * Tells the CommandManager the command ids that this target wants to see
 * juce::Array is interesting
 * "holds a resizeable array of primitive or copy-by-value objects
 *
 * So roughly like a Java List but with templating you don't have to use
 * object wrappers around primitive types
 *
 * Now I'm confused, we have a bunch of commands defined in the enum but
 * we're only returning three of them.  
 */
void MainMenu::getAllCommands (juce::Array<juce::CommandID>& c) 
{
    juce::Array<juce::CommandID> commands {
        CommandIDs::menuPositionInsideWindow,
        CommandIDs::menuPositionGlobalMenuBar,
        CommandIDs::menuPositionBurgerMenu
    };
    c.addArray (commands);
}

/**
 * Return information about a command id, presumably one of the
 * ids return by getAllCommands
 */
void MainMenu::getCommandInfo (juce::CommandID commandID, juce::ApplicationCommandInfo& result)
{
    switch (commandID)
    {
        case CommandIDs::menuPositionInsideWindow:
            result.setInfo ("Inside Window", "Places the menu bar inside the application window", "Menu", 0);
            result.setTicked (true);
            result.addDefaultKeypress ('w', juce::ModifierKeys::shiftModifier);
            break;
        case CommandIDs::menuPositionGlobalMenuBar:
            result.setInfo ("Global", "Uses a global menu bar", "Menu", 0);
            result.setTicked (false);
            result.addDefaultKeypress ('g', juce::ModifierKeys::shiftModifier);
            break;
        case CommandIDs::menuPositionBurgerMenu:
            result.setInfo ("Burger Menu", "Uses a burger menu", "Menu", 0);
            result.setTicked (false);
            result.addDefaultKeypress ('b', juce::ModifierKeys::shiftModifier);
            break;
        default:
            break;
    }
}

/**
 * Called by the command manager when one of the commands is performed
 * Demo only handled the ones to change menu style
 */
bool MainMenu::perform (const juce::ApplicationCommandTarget::InvocationInfo& info)
{
    switch (info.commandID)
    {
        case CommandIDs::menuPositionInsideWindow:
            break;
        case CommandIDs::menuPositionGlobalMenuBar:
            break;
        case CommandIDs::menuPositionBurgerMenu:
            break;
        default:
            return false;
    }

    return true;
}


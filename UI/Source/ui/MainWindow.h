/*
 * The main application window, added direcdtly inside
 * the Juce MainComponent.
 *
 * Here we manaage a menu, main display area, and a set of configuration editors.
 * The hiearchy goes
 *
 * MainComponent - Juce interface
 *  Supervisor - overall coordination
 *    DisplayManager - selects from display options
 *      MainWindow
 *        MobiusDisplay - the default display
 *
 * The main purpose of MainWindow vs. MobiusDisplay is to manage the
 * menus and configuration editor panels.  Someday we can swap different
 * implementations of MobiusDisplay but we would always have the same system
 * menu and editor panels.  Root menu might be something we want to push
 * down into MobiusDisplay but I think the editors would always go here
 * unless you also wanted to simplify those for a "light" display vs. "classic"
 * display.
 *
 * This is as low as Supervisor goes for now.  Subcomponents can pass UIActions
 * back up through the ownership chain giving each level a chance to handle
 * actions before passing them to the engine.  Alternately we could make
 * Supervisor figure that out, but then it would have to walk back down to
 * execute them.  And it makes sense not to have Supervisor be aware of
 * UI implementation details.
 */

#pragma once

#include <JuceHeader.h>

#include "MainMenu.h"
#include "display/MobiusDisplay.h"
#include "config/ConfigEditor.h"

#include "TestPanel.h"
#include "TableTest.h"
#include "TabTest.h"

class MainWindow : public juce::Component, public MainMenu::Listener
{
  public:

    MainWindow(class Supervisor* super);
    ~MainWindow();

    class Supervisor* getSupervisor() {
        return supervisor;
    }
        
    void resized() override;

    void configure(class MobiusConfig* config);
    void configure(class UIConfig* config);
    void update(class MobiusState* state);
    bool saveConfiguration(class UIConfig* config);
    
    // MainMenu listener
    void mainMenuSelection(int id);
    
    // actions propagated up from within MobiusDisplay
    void doAction(class UIAction* action);

  private:

    class Supervisor* supervisor;

    // until we support alternate displays, just define
    // the components directly
    
    MainMenu menu;
    MobiusDisplay display {this};
    ConfigEditor configEditor {this};

    // various temporary tests
    TestPanel test;
    TableTest table;
    TabTest tabs;
    
};    

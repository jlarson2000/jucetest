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
 *      MainWindow - the default display
 */

#pragma once

#include <JuceHeader.h>

#include "MainMenu.h"
#include "MainDisplay.h"
#include "config/ConfigEditor.h"

#include "TestPanel.h"
#include "TableTest.h"
#include "TabTest.h"

class MainWindow : public juce::Component, public MainMenu::Listener
{
  public:

    MainWindow(class Supervisor* super);
    ~MainWindow();

    void resized() override;

    void updateConfiguration(class UIConfig* config);
    void updateConfiguration(class MobiusConfig* config);

    // MainMenu listener
    void mainMenuSelection(int id);
    
  private:

    class Supervisor* supervisor;

    // until we support alternate displays, just define
    // the components directly
    
    MainMenu menu;
    MainDisplay display;
    ConfigEditor configEditor {this};

    // various temporary tests
    TestPanel test;
    TableTest table;
    TabTest tabs;
    
};    

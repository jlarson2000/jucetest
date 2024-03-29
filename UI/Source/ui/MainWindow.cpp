/*
 * The default Mobius main window.
 * Contains a main menu, a set of configuration editor popup panels
 * and the MobiusDisplay.
 *
 * Let's put the ConfigEditor under here for now
 * though it probably makes more sense to move that
 * up to DisplayManager since editors won't change but
 * the main display might?  
 */

#include <JuceHeader.h>

#include "../Supervisor.h"
#include "../util/Trace.h"
#include "../model/UIConfig.h"
#include "../model/MobiusState.h"

#include "JuceUtil.h"
#include "MainMenu.h"
#include "config/ConfigEditor.h"
#include "../DiagnosticWindow.h"
#include "../Supervisor.h"

// temporary testing panels
#include "TestPanel.h"
#include "TableTest.h"
#include "TabTest.h"

#include "MainWindow.h"

//#include "../ParameterPuller.h"

MainWindow::MainWindow(Supervisor* super)
{
    setName("MainWindow");
    supervisor = super;

    // using a Listener pattern here but could just
    // pass this to the constructor like we do for the others
    addAndMakeVisible(menu);
    menu.setListener(this);
    
    addAndMakeVisible(display);

    // let the config editor pre-load all its panels
    configEditor.init(supervisor);

    //addChildComponent(test);
    //addChildComponent(table);
    //addChildComponent(tabs);
}

MainWindow::~MainWindow()
{
}

void MainWindow::resized()
{
    juce::Rectangle<int> area = getLocalBounds();
    menu.setBounds(area.removeFromTop(menu.getPreferredHeight()));
    display.setBounds(area);
}

//////////////////////////////////////////////////////////////////////
//
// Menu Callbacks
//
//////////////////////////////////////////////////////////////////////

/**
 * Called for anything other than the dynamic
 * Preset and Setup menus
 */
void MainWindow::mainMenuSelection(int id)
{
    switch (id)
    {
        case MainMenu::OpenLoop: break;
        case MainMenu::OpenProject: break;
        case MainMenu::SaveLoop: break;
        case MainMenu::SaveProject: break;
        case MainMenu::QuickSave: break;
        case MainMenu::ReloadScripts: break;
        case MainMenu::Exit: break;

        case MainMenu::GlobalParameters: {
            configEditor.showGlobal();
        }
        break;
        case MainMenu::Presets: {
            configEditor.showPresets();
        }
        break;
        case MainMenu::TrackSetups: {
            configEditor.showSetups();
        }
        break;
        case MainMenu::MidiControl: {
            configEditor.showMidiBindings();
        }
        break;
        
        case MainMenu::KeyboardControl:  {
            configEditor.showKeyboardBindings();
        }
            break;
            
        case MainMenu::Buttons: {
            configEditor.showButtons();
        }
        break;

        case MainMenu::PluginParamters: break;

        case MainMenu::DisplayComponents: {
            configEditor.showDisplay();
        }
            break;
        case MainMenu::Scripts: {
            configEditor.showScripts();
        }
            break;
        case MainMenu::Samples: {
            configEditor.showSamples();
        }
            break;
        case MainMenu::MidiDevices: {
            configEditor.showMidiDevices();
        }
            break;
        case MainMenu::AudioDevices: {
            configEditor.showAudioDevices();
        }
            break;
        case MainMenu::InstallSamples: {
            supervisor->installSamples();
        }
            break;

        case MainMenu::KeyBindings: break;
        case MainMenu::MidiBindings: break;
        case MainMenu::DiagnosticWindow: {
            DiagnosticWindow::launch();
        }
            break;
        case MainMenu::About: {
            //test.setVisible(true);
            //test.center();
            //JuceUtil::dumpComponent(&test);
            //ParameterPuller* puller = new ParameterPuller();
            //puller->pull();
        }
            break;
                
        default: {
            trace("Unknown menu item: %d\n", id);
        }
        break;
    }
}

//////////////////////////////////////////////////////////////////////
//
// Configuration
//
//////////////////////////////////////////////////////////////////////

/**
 * Called by Supervisor after the initial load of the
 * configuration files, and after any editing.
 * Most of our configuration is in UIConfig.
 * MainDisplay uses this most, old Mobius adjusted
 * the menus for Preset and Setup selection.
 */
void MainWindow::configure(UIConfig* config)
{
    display.configure(config);
}

/**
 * Called by Supervisor after the initial load of the
 * configuration files, and after any editing.
 *
 * Relatively little is of interest in the main config
 * except for the names of the Presets and Setups and other
 * things we might want to select with menus or display elements.
 */
void MainWindow::configure(MobiusConfig* config)
{
    display.configure(config);
}

/**
 * Called during the shutdown process to save any accumulated
 * changes.
 */
bool MainWindow::saveConfiguration(UIConfig* config)
{
    return display.saveConfiguration(config);
}

void MainWindow::update(MobiusState* state)
{
    display.update(state);
}

//////////////////////////////////////////////////////////////////////
//
// Actions
//
//////////////////////////////////////////////////////////////////////

/**
 * Propagate an action sent from down below.
 * We have nothing to add here though I suppose
 * this cold be a place where an action could target
 * a menu selection.  I can't think of a reason for that,
 * maybe saving/loading the project or refreshing the configuration.
 */

void MainWindow::doAction(UIAction* action)
{
    supervisor->doAction(action);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

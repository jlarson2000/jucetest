/**
 * Manages everying related the Mobius user interface.
 * Here is where we might swap in different display styles
 * based on user preference.
 */

#include <JuceHeader.h>

#include "../Supervisor.h"
#include "../model/UIConfig.h"
#include "../model/MobiusConfig.h"

#include "JuceUtil.h"
#include "MainWindow.h"
#include "DisplayManager.h"

DisplayManager::DisplayManager(Supervisor* super, juce::Component* main)
{
    supervisor = super;
    mainComponent = main;

    // select from display options, only one right now

    MainWindow* window = new MainWindow(super);
    main->addAndMakeVisible(window);
    
    mainWindow.reset(window);
}

DisplayManager::~DisplayManager()
{
}

/**
 * Called by Supervisor after the initial load of the
 * configuration files, and after any editing.
 * Most of our configuration is in UIConfig.
 *
 * todo: think about control flow here
 * rather than passing these down everwhere could just
 * call a single updateConfiguration() and have the
 * subcomponents pull out the things they need from Supervisor?
 */
void DisplayManager::updateConfiguration(UIConfig* config)
{
    mainWindow->updateConfiguration(config);
}

/**
 * Called by Supervisor after the initial load of the
 * configuration files, and after any editing.
 *
 * Relatively little is of interest in the main config
 * except for the names of the Presets and Setups and other
 * things we might want to select with menus or display elements.
 */
void DisplayManager::updateConfiguration(MobiusConfig* config)
{
    mainWindow->updateConfiguration(config);
}

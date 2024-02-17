/**
 * Manages everying related the Mobius user interface.
 * Here is where we might swap in different display styles
 * based on user preference.
 */

#include <JuceHeader.h>

#include "../Supervisor.h"
#include "../model/UIConfig.h"
#include "../model/MobiusConfig.h"
#include "../model/MobiusState.h"

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
    
    // store it in the unique_ptr
    mainWindow.reset(window);
}

DisplayManager::~DisplayManager()
{
}

/**
 * Called by Supervisor after the initial load of the
 * configuration files, and after any editing.
 * Most of our configuration is in UIConfig.
 */
void DisplayManager::configure(UIConfig* config)
{
    mainWindow->configure(config);
}

/**
 * Called by Supervisor before we finish shutting down
 * to save any accumulated configuration changes.
 */
bool DisplayManager::saveConfiguration(UIConfig* config)
{
    return mainWindow->saveConfiguration(config);
}

/**
 * Called by Supervisor after the initial load of the
 * configuration files, and after any editing.
 *
 * Relatively little is of interest in the main config
 * except for the names of the Presets and Setups and other
 * things we might want to select with menus or display elements.
 */
void DisplayManager::configure(MobiusConfig* config)
{
    mainWindow->configure(config);
}

void DisplayManager::update(MobiusState* state)
{
    mainWindow->update(state);
}


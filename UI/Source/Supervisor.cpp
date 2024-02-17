/*
 * A singleton object that provides services and coordinates
 * activities between the various sub-components of the Mobius application
 *
 */

#include <JuceHeader.h>

#include "util/Trace.h"
#include "util/FileUtil.h"

#include "model/MobiusConfig.h"
#include "model/UIConfig.h"
#include "model/XmlRenderer.h"
#include "model/UIAction.h"
#include "model/MobiusState.h"

#include "ui/DisplayManager.h"
#include "mobius/MobiusInterface.h"

#include "Supervisor.h"

Supervisor::Supervisor(juce::Component* main)
{
    mainComponent = main;
//    uiThread.setSupervisor(this);
}

Supervisor::~Supervisor()
{
    // todo: check for proper termination?
}

/**
 * Initialize the supervisor, this is where the magic begins.
 */
void Supervisor::start()
{
    MobiusConfig* config = getMobiusConfig();

    MobiusInterface::startup();

    mobius = MobiusInterface::getMobius();
    mobius->configure(config);

    displayManager.reset(new DisplayManager(this, mainComponent));
    
    // load the initial configuration and tell everyone about it
    displayManager->configure(getUIConfig());

    // a few things in the UI are sensitive to global parameters
    displayManager->configure(config);

    // let the UI refresh go
    uiThread.start();

    // initial display update
    MobiusState* state = mobius->getState();
    displayManager->update(state);
}

/**
 * Shut down the supervisor, we're tired, but it's a good kind of tired.
 */
void Supervisor::shutdown()
{
    // stop the UI thread so we don't get any lingering events
    uiThread.stop();

    // stop the Mobius engine, mobius object is a smart pointer
    MobiusInterface::shutdown();
    // this is now invalid
    mobius = nullptr;

    // save any UI configuration changes that were made during use
    // in practice this is only for StatusArea but might have others someday
    UIConfig* config = getUIConfig();
    bool changes = displayManager->saveConfiguration(config);
    if (changes) {
        writeUIConfig(config);
    }
        
    // DisplayManager is in a smart pointer, but should we be doing
    // things before the destructor happens, in particular the audio
    // streams will be closed before delete happens

    // mobiusConfig and uiConfig are smart pointers
}


/**
 * Called by the MobiusThread to process events.
 * The expected cycle time is 10ms or 1/10 second.
 * To make things look real, we need to advance the simulator by an amount
 * that corresponds to the thread notification cycle.  So if the cycle is 100ms
 * or 1/10 second, then the number of audio frames that would have been cousumed
 * in that time is SamplesPerSecond divided by 10.  It doesn't really matter what
 * sample rate is as long as both sides agree, so we'll assume 41.1 which
 * is what old Mobius always used.  So the number of frames per
 * thread cycle is 4110.
 */
void Supervisor::advance()
{
    // tell the simulator to pretend it received some audio
    mobius->simulateInterrupt(nullptr, nullptr, 4110);
    
    // traverse the display components telling then to reflect changes in the engine
    MobiusState* state = mobius->getState();
    displayManager->update(state);
}

//////////////////////////////////////////////////////////////////////
//
// Configuration Files
//
// Still using our old File utiltiies, need to use Juce
//
//////////////////////////////////////////////////////////////////////

const char* MobiusConfigFile = "mobius.xml";
const char* UIConfigFile = "ui.xml";

/**
 * Locate the root of the Mobius installation directory
 * where configuration files are stored.
 */
juce::String Supervisor::findMobiusInstallationPath()
{
    // obviously need to be smarter here
    return juce::String("c:/dev/jucetest/UI/Source/");
}

/**
 * Read the XML for a configuration file.
 */
char* Supervisor::readConfigFile(const char* name)
{
    juce::String root = findMobiusInstallationPath();
    juce::String path = root + name;
    char* xml = ReadFile(path.toUTF8());
    return xml;
}

/**
 * Write an XML configuration file.
 */
void Supervisor::writeConfigFile(const char* name, char* xml)
{
    juce::String root = findMobiusInstallationPath();
    juce::String path = root + name;
    WriteFile(path.toUTF8(), xml);
}

/**
 * Read the MobiusConfig from the file system.
 * The returned object is owned by the caller and must be deleted.
 */
MobiusConfig* Supervisor::readMobiusConfig()
{
    MobiusConfig* config = nullptr;
    
    char* xml = readConfigFile(MobiusConfigFile);
    if (xml != nullptr) {
        XmlRenderer xr;
        config = xr.parseMobiusConfig(xml);
        // todo: capture or trace parse errors
        delete xml;
    }
    return config;
}

/**
 * Write a MobiusConfig back to the file system.
 * Ownership of the config object does not transfer.
 */
void Supervisor::writeMobiusConfig(MobiusConfig* config)
{
    if (config != nullptr) {
        XmlRenderer xr;
        char* xml = xr.render(config);
        writeConfigFile(MobiusConfigFile, xml);
        delete xml;
    }
}

/**
 * Similar read/writer for the UIConfig
 */
UIConfig* Supervisor::readUIConfig()
{
    UIConfig* config = nullptr;
    
    char* xml = readConfigFile(UIConfigFile);
    if (xml != nullptr) {
        XmlRenderer xr;
        config = xr.parseUIConfig(xml);
        // todo: capture or trace parse errors
        delete xml;
    }
    return config;
}

/**
 * Write a UIConfig back to the file system.
 * Ownership of the config object does not transfer.
 */
void Supervisor::writeUIConfig(UIConfig* config)
{
    if (config != nullptr) {
        XmlRenderer xr;
        char* xml = xr.render(config);
        writeConfigFile(UIConfigFile, xml);
        delete xml;
    }
}

/**
 * Called by components to obtain the MobiusConfig file.
 * The object remains owned by the Supervisor and must not be deleted.
 * For now we will allow it to be modified by the caller, but to save
 * it and propagate change it must call updateMobiusConfig.
 * Caller is responsible for copying it if it wants to make temporary
 * changes.
 *
 * todo: the Proper C++ Way seems to be to pass around the std::unique_pointer
 * rather than calling .get on it.  I suppose it helps drive the point
 * home about ownership, but I find it ugly.
 */
MobiusConfig* Supervisor::getMobiusConfig()
{
    // bool operator tests for nullness of the pointer
    if (!mobiusConfig) {
        // haven't loaded it yet
        MobiusConfig* neu = readMobiusConfig();
        if (neu == nullptr) {
            // bootstrap one so we don't have to keep checking
            neu = new MobiusConfig();
        }
        mobiusConfig.reset(neu);
    }
    return mobiusConfig.get();
}

/**
 * Same dance for the UIConfig
 */
UIConfig* Supervisor::getUIConfig()
{
    if (!uiConfig) {
        // haven't loaded it yet
        UIConfig* neu = readUIConfig();
        if (neu == nullptr) {
            neu = new UIConfig();
        }
        uiConfig.reset(neu);
    }
    return uiConfig.get();
}

/**
 * Save a modified MobiusConfig, and propagate changes
 * to the interested components.
 * In practice this should only be called by ConfigEditor.
 *
 * Current assumption is that the object returned by getMobiusConfig
 * has been modified.  I don't think it's worth messing with excessive
 * copying of this and ownership transfers.
 */
void Supervisor::updateMobiusConfig()
{
    if (mobiusConfig) {
        MobiusConfig* config = mobiusConfig.get();

        writeMobiusConfig(config);

        displayManager->configure(config);
        mobius->configure(config);
    }
}

void Supervisor::updateUIConfig()
{
    if (uiConfig) {
        UIConfig* config = uiConfig.get();
        
        writeUIConfig(config);
        // DisplayManager/MainWindow are the primary consumers of this
        displayManager->configure(config);
    }
}

//////////////////////////////////////////////////////////////////////
//
// Actions
//
//////////////////////////////////////////////////////////////////////

/**
 * Propagate an action sent up from one of the display elements that
 * was not handled at a lower level.
 * If we get back here, the action is targeted to the mobius engine.
 * Other options could be to refresh configuration, save projects,
 * or other housekeeping tasks you would normally do from menu items.
 */
void Supervisor::doAction(UIAction* action)
{
    // resolve it if we haven't already
    // should normally be able to do this early but if we're
    // dealing with scripts it will be harder
    action->resolve();

    mobius->doAction(action);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

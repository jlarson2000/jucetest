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
#include "ui/DisplayManager.h"

#include "Supervisor.h"

Supervisor::Supervisor(juce::Component* main)
{
    mainComponent = main;
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
    displayManager.reset(new DisplayManager(this, mainComponent));

    // load the initial configuration and tell everyone about it
    displayManager->configure(getUIConfig());

    // a few things in the UI are sensitive to global parameters
    MobiusConfig* mc = getMobiusConfig();
    displayManager->configure(mc);
}

/**
 * Shut down the supervisor, we're tired, but it's a good kind of tired.
 */
void Supervisor::shutdown()
{
    // DisplayManager is in a smart pointer, but should we be doing
    // things before the destructor happens, in particular the audio
    // streams will be closed before delete happens
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
        writeMobiusConfig(mobiusConfig.get());

        // this is where the engine will want to know about things
        // like the max loop & track counts
        displayManager->configure(mobiusConfig.get());
    }
}

void Supervisor::updateUIConfig()
{
    if (uiConfig) {
        writeUIConfig(uiConfig.get());
        // DisplayManager/MainWindow are the primary consumers of this
        displayManager->configure(uiConfig.get());
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
    // todo: stub out the engine

    trace("Supervisor::doAction %s\n", action->getDescription());
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

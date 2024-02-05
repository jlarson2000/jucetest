/**
 * Class managing most configuration editing dialogs.
 * Old Mobius implemented these with popup windows, we're now doing
 * these with simple Juce components overlayed over the main window.
 * 
 * See ConfigEditor.h for the interface with MainComponent.
 *
 * This wrapper allows for the possible experimentation with popup windows
 * if we ever decide to go there, isolating MainComponent from the details.
 *
 */

#include <JuceHeader.h>

#include "../util/FileUtil.h"
#include "../model/XmlRenderer.h"
#include "../model/MobiusConfig.h"

#include "JuceUtil.h"
#include "ConfigEditor.h"

ConfigEditor::ConfigEditor(juce::Component* argOwner)
{
    owner = argOwner;

    // add the various config panels to the owner but don't
    // make them visible yet
    addPanel(&global);
    addPanel(&presets);
    addPanel(&setups);
}

ConfigEditor::~ConfigEditor()
{
    // RIAA will destruct the various panels

    // this we own
    delete masterConfig;
}

/**
 * Internal method to add the panel to the parent and put
 * it on the panel list for iteration.  The panel is
 * added but not made visible yet.
 */
void ConfigEditor::addPanel(ConfigPanel* p)
{
    owner->addChildComponent(p);
    panels.add(p);
}

void ConfigEditor::showGlobal()
{
    show(&global);
}

void ConfigEditor::showPresets()
{
    show(&presets);
}

void ConfigEditor::showSetups()
{
    show(&setups);
}

void ConfigEditor::closeAll()
{
    // cancel any active editing state
    for (int i = 0 ; i < panels.size() ; i++) {
        ConfigPanel* p = panels[i];
        p->cancel();
    }

    // TODO: should this have the side effect of canceling the current editing session?
    show(nullptr);
}

/**
 * Hide the currently active panel if any and show the desired one.
 */
void ConfigEditor::show(ConfigPanel* selected)
{
    for (int i = 0 ; i < panels.size() ; i++) {
        ConfigPanel* other = panels[i];
        // note that this does not cancel an editing session, it
        // just hides it.  Some might want different behavior?
        other->setVisible(other == selected);
    }

    // weird for Juce but since we defer rendering and don't do it the normal way
    // resize just before showing
    selected->resized();
    selected->center();

    // tell it to load state if it hasn't already
    selected->load();

    JuceUtil::dumpComponent(selected);
}

//////////////////////////////////////////////////////////////////////
//
// ConfigPanel Callbacks
//
//////////////////////////////////////////////////////////////////////

/**
 * Called by the panel when it is done.
 * There are three states a panel can be in:
 *
 *    unloaded: hasn't done anything
 *    loaded: has state loaded from the master config but hasn't changed anything
 *    changed: has unsaved changes
 *      
 * When a panel is closed by one of the buttons we look at the other
 * panels to see if they can be shown.  If any panel has unsaved
 * changes it will be shown.
 *
 * If no panel has unsaved changes but some of them have been loaded
 * we could either close everything, or show a loaded one.  The thinking
 * is that if someone bothered to show a panel, selected another without
 * changing anything, then closed the second panel, we can return to the
 * first one and let them continue.  Alternately, since they didn't bother
 * changing anything in the first one we could just close all of them.
 *
 * The first approach behaves more like a stack of panels which might
 * be nice.  The second is probably more obvious, if you open another without
 * doing anything in the first, you probably don't care about the first.
 * Let's start with the stack.
 *
 * Note though that this isn't actually a stack since we don't maintain
 * an ordered activation list if there are more than two loaded panels.
 */
void ConfigEditor::close(ConfigPanel* closing, bool canceled)
{
    ConfigPanel* nextLoaded = nullptr;
    ConfigPanel* nextChanged = nullptr;

    if (!closing->isVisible()) {
        // callback from something we asked to close
        // that wasn't visible
    }
    else {
        closing->setVisible(false);
    
        for (int i = 0 ; i < panels.size() ; i++) {
            ConfigPanel* p = panels[i];
            if (p->isLoaded())
              nextLoaded = p;
            if (p->isChanged())
              nextChanged = p;
        }

        if (nextChanged != nullptr) {
            nextChanged->setVisible(true);
        }
        else {
            // i don't think this needs to be configurable
            // but I want to try both behaviors easily
            bool showLoaded = true;

            if (showLoaded && nextLoaded != nullptr) {
                nextLoaded->setVisible(true);
            }
            else {
                // all done
                // we don't need to write configuration if the panel canceled
                if (!canceled)
                  saveMobiusConfig();
            }
        }
    }
}

/**
 * Determine the path to the MobiusConfig file.
 * Hard code for now, need to find a good way to do this.
 */
const char* ConfigEditor::getConfigFilePath()
{
    const char* path = "c:/dev/jucetest/UI/Source/mobius.xml";
    return path;
}

/**
 * Called by a panel read the MobiusConfig.
 * The master config object is managed by ConfigEditor
 * the panels are allowed to make modifications to it
 * and ask us to save it.  Each panel must not overlap
 * on the changes it makes to the MobiusConfig.
 *
 * Might be better to have the panel return us just the
 * changes and have us splice it into the master config?
 */
MobiusConfig* ConfigEditor::getMobiusConfig()
{
    if (masterConfig == nullptr) {
        const char* path = getConfigFilePath();
        char* xml = ReadFile(path);
        if (xml != nullptr) {
            XmlRenderer xr;
            masterConfig = xr.parseMobiusConfig(xml);
            // todo: display parse errors
            delete xml;
        }

        // if we couldn't read one, boostrap a new one
        // might want to warn here?
        if (masterConfig == nullptr)
          masterConfig = new MobiusConfig();
    }

    return masterConfig;
}

/**
 * Called by the ConfigPanel after it has made modifications
 * to the MobiusConfig returned by getMobiusConfig.
 */
void ConfigEditor::saveMobiusConfig()
{
    if (masterConfig != nullptr) {
        const char* path = getConfigFilePath();
        XmlRenderer xr;
        char* xml = xr.render(masterConfig);
        WriteFile(path, xml);
        delete xml;
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

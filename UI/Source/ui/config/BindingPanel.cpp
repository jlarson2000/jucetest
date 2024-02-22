/**
 * All binding panels share a common structure
 * They are ConfigPanels so have Save/Cancel buttons in the footer.
 * They have an optional object selector for bindings that have more than one object.
 *
 * On the left is a large scrolling binding table with columns for
 *
 *    Target
 *    Trigger
 *    Scope
 *    Arguments
 * 
 * Under the Target table are buttons New, Update, Delete to manage rows in the table.
 *
 * Under the BindingTargetPanel are Extended Fields to add additional information about
 * the Binding.  At minimum it will have an Arguments field to specifify arbitrary
 * trigger.
 *
 */

#include <JuceHeader.h>

#include <string>
#include <sstream>

#include "../../util/Trace.h"
#include "../../model/UIConfig.h"
#include "../../model/MobiusConfig.h"
#include "../common/Form.h"
#include "../JuceUtil.h"

#include "ConfigEditor.h"
#include "BindingTable.h"
#include "BindingTargetPanel.h"

#include "BindingPanel.h"

BindingPanel::BindingPanel(ConfigEditor* argEditor, const char* title) :
    ConfigPanel{argEditor, title, ConfigPanelButton::Save | ConfigPanelButton::Cancel, false}
{
    setName("BindingPanel");

    bindings.setListener(this);
    content.addAndMakeVisible(bindings);
    
    content.addAndMakeVisible(targets);

    content.addAndMakeVisible(form);
    // because of the use of subclass virtuals to build parts
    // of the form, we have to complete construction before calling initForm
    // subclass must do this instead
    // initForm();
    
    // we can either auto size at this point or try to
    // make all config panels a uniform size
    setSize (900, 600);
}

BindingPanel::~BindingPanel()
{
}

/**
 * ConfigPanel overload to load state.
 */
void BindingPanel::load()
{
    if (!loaded) {
        JuceUtil::dumpComponent(this);
        // good a place as any
        grabKeyboardFocus();
        
        MobiusConfig* config = editor->getMobiusConfig();
        
        // let the target panel know the names of the things it can target
        targets.configure(config);

        // just look at the first one for now
        BindingConfig* bindingConfig = config->getBindingConfigs();
        if (bindingConfig != nullptr) {
            Binding* blist = bindingConfig->getBindings();
            while (blist != nullptr) {
                // subclass overload
                if (isRelevant(blist)) {
                    // table will copy
                    bindings.add(blist);
                }
                blist = blist->getNext();
            }
            bindings.updateContent();
        }

        // force this true for testing
        changed = true;
    }
}

void BindingPanel::save()
{
    if (changed) {
        // harder than buttons, have to remove and replace just
        // the TriggerKey bindings
        //editor->saveUIConfig();
        changed = false;
    }
}

void BindingPanel::cancel()
{
    bindings.clear();
    changed = false;
}

//////////////////////////////////////////////////////////////////////
//
// Trigger/Scope/Arguments Form
//
//////////////////////////////////////////////////////////////////////

/**
 * Build out the form containing scope, subclass specific files
 * binding arguments.
 */
void BindingPanel::initForm()
{
    // todo: scope

    // subclass must override this
    addSubclassFields();

    // don't have an interface that allows static fields not
    // owned by the Form
    arguments = new Field("Arguments", Field::Type::String);
    arguments->setWidthUnits(15);
    form.add(arguments);
    form.render();
}

/**
 * Reset all trigger and target arguments to their initial state
 */
void BindingPanel::resetForm()
{
    targets.reset();
    // todo: scope

    resetSubclassFields();
    
    arguments->setValue(juce::var());
}

/**
 * Refresh form to have values for the selected binding
 */
void BindingPanel::refreshForm(Binding* b)
{
    targets.select(b);
    refreshSubclassFields(b);
    
    juce::var args = juce::var(b->getArgs());
    arguments->setValue(args);
    // used this in old code, now that we're within a form still necessary?
    // arguments.repaint();
}

/**
 * Copy what we have displayed for targets, scopes, and arguments
 * into a Binding
 */
void BindingPanel::captureForm(Binding* b)
{
    targets.capture(b);

    // todo: scope
    captureSubclassFields(b);
    
    juce::var value = arguments->getValue();
    b->setArgs(value.toString().toUTF8());
}

//////////////////////////////////////////////////////////////////////
//
// BindingTable Listener
//
//////////////////////////////////////////////////////////////////////

/**
 * Render the cell that represents the binding trigger.
 */
juce::String BindingPanel::renderTriggerCell(Binding* b)
{
    // subclass must overload this
    return renderSubclassTrigger(b);
}

/**
 * Update the binding info components to show things for the
 * binding selected in the table
 */
void BindingPanel::bindingSelected(Binding* b)
{
    if (bindings.isNew(b)) {
        // uninitialized row, don't modify it but reset the target display
        resetForm();
    }
    else {
        refreshForm(b);
    }
    
}

Binding* BindingPanel::bindingNew()
{
    Binding* neu = nullptr;
    
    // here we have the option of doing an immediate capture
    // of what is in the target selectors, return null
    // to leave a [New] placeholder row
    bool captureCurrentTarget = false;

    if (captureCurrentTarget && targets.isTargetSelected()) {
        neu = new Binding();
        captureForm(neu);
    }
    else {
        // we'l let BindingTable make a placeholder row
        // clear any lingering target selection?
        resetForm();
    }

    return neu;
}

void BindingPanel::bindingUpdate(Binding* b)
{
    if (!targets.isTargetSelected()) {
        // todo: try an alert here?
    }
    else {
        captureForm(b);
    }
}

void BindingPanel::bindingDelete(Binding* b)
{
    resetForm();}

/**
 * Field::Listener callback
 * If there is something selected in the table could actively
 * change it, but we're going with a manual Update button for now
 * now that we have a Form this would have to be a lot more
 * complicated
 */
void BindingPanel::fieldSet(Field* field)
{
}

//////////////////////////////////////////////////////////////////////
//
// Component
//
//////////////////////////////////////////////////////////////////////

void BindingPanel::resized()
{
    ConfigPanel::resized();
    
    juce::Rectangle<int> area = getLocalBounds();

    // leave some space at the top
    area.removeFromTop(20);
    // and on the left
    area.removeFromLeft(20);

    // let's fix the size of the table for now rather
    // adapt to our size
    int width = bindings.getPreferredWidth();
    int height = bindings.getPreferredHeight();
    bindings.setBounds(area.getX(), area.getY(), width, height);

    area.removeFromLeft(bindings.getWidth() + 50);
    targets.setBounds(area.getX(), area.getY(), 300, 400);

    //form->render();
    form.setTopLeftPosition(area.getX(), targets.getY() + targets.getHeight() + 10);
}    

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

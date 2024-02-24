/**
 * A form panel for configuring MIDI devices.
 */

#include <JuceHeader.h>

#include "../../util/Trace.h"
#include "../../model/MobiusConfig.h"
#include "../../Supervisor.h"

#include "../common/Form.h"
#include "../common/Field.h"
#include "../JuceUtil.h"

#include "ConfigEditor.h"
#include "LogPanel.h"
 
#include "AudioDevicesPanel.h"

AudioDevicesPanel::AudioDevicesPanel(ConfigEditor* argEditor) :
    ConfigPanel{argEditor, "Audio Devices", ConfigPanelButton::Save | ConfigPanelButton::Cancel, true}
{
    setName("AudioDevicesPanel");
    render();
}

AudioDevicesPanel::~AudioDevicesPanel()
{
    // members will delete themselves
    // remove the AudioManager log if we were still showing
    hiding();
}

//////////////////////////////////////////////////////////////////////
//
// ConfigPanel overloads
//
//////////////////////////////////////////////////////////////////////

/**
 * Called by ConfigEditor when we're about to be made visible.
 */
void AudioDevicesPanel::showing()
{
    //AudioManager* mm = Supervisor::Instance->getAudioManager();
    //mm->setLog(&log);
}

/**
 * Called by ConfigEditor when we're about to be made invisible.
 */
void AudioDevicesPanel::hiding()
{
    //AudioManager* mm = Supervisor::Instance->getAudioManager();
    //mm->setLog(nullptr);
}

/**
 * Called by ConfigEditor when asked to edit devices.
 * Unlike most other config panels, we don't have a lot of complex state to manage.
 */
void AudioDevicesPanel::load()
{
    if (!loaded) {

        MobiusConfig* config = editor->getMobiusConfig();
        if (config != nullptr) {
            const char* inputName = config->getAudioInput();
            const char* outputName = config->getAudioOutput();

            inputField->setValue(juce::String(inputName));
            // should have already been set to the current value
            //mm->setInput(juce::String(inputName));
            
            // mm->setOutput(juce::String(outputName));
            // do we really need this?
            //const char* thruName = config->getAudioThrough();
        }
        
        loaded = true;
        // force this true for testing
        changed = true;
    }
}

/**
 * Called by the Save button in the footer.
 * Tell the ConfigEditor we are done.
 */
void AudioDevicesPanel::save()
{
    if (changed) {
        MobiusConfig* config = editor->getMobiusConfig();
        
        config->setAudioInput(inputField->getStringValue().toUTF8());

        editor->saveMobiusConfig();

        loaded = false;
        changed = false;
    }
    else if (loaded) {
        loaded = false;
    }
}

/**
 * Throw away all editing state.
 */
void AudioDevicesPanel::cancel()
{
    loaded = false;
    changed = false;
}

//////////////////////////////////////////////////////////////////////
//
// Internal Methods
// 
//////////////////////////////////////////////////////////////////////

/**
 * AudioDevicesContent is a wrapper around the Form used to select
 * devices and a LogPanel used to display MIDI events.  Necessary
 * because ConfigPanel only allows a single child of it's content
 * component and we want to control layout of the form relative
 * to the log.
 *
 * Interesting component ownership problem...
 * I all I want this to do is handle the layout but I'd like the
 * components owned by the parent, at least the LogPanel.  In resized
 * we either have to have it make assumptions about the children
 * or have the parent give it concrete references to them.   That's
 * like how Form works.  Think about a good pattern for this if
 * it happens more.
 */
void AudioDevicesContent::resized()
{
    // the form will have sized itself to the minimum bounds
    // necessary for the fields
    // leave a little gap then let the log window fill the rest
    // of the available space
    juce::Rectangle<int> area = getLocalBounds();
    
    // kludge, work out parenting awareness
    Form* form = (Form*)getChildComponent(0);
    if (form != nullptr) {
        juce::Rectangle<int> min = form->getMinimumSize();
        form->setBounds(area.removeFromTop(min.getHeight()));
    }
    
    // gap
    area.removeFromTop(20);

    LogPanel* log = (LogPanel*)getChildComponent(1);
    if (log != nullptr)
      log->setBounds(area);
}

//////////////////////////////////////////////////////////////////////
//
// Form Rendering
//
//////////////////////////////////////////////////////////////////////

void AudioDevicesPanel::render()
{
    initForm();
    form.render();

    adcontent.addAndMakeVisible(form);
    adcontent.addAndMakeVisible(log);

    // place it in the ConfigPanel content panel
    content.addAndMakeVisible(adcontent);

    // have been keeping the same size for all ConfigPanels
    // rather than having them shrink to fit, should move this
    // to ConfigPanel or ConfigEditor
    setSize (900, 600);
}

void AudioDevicesPanel::initForm()
{
    inputField = new Field("Input device", Field::Type::String);
    form.add(inputField);
}

void AudioDevicesPanel::fieldChanged(Field* field)
{
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

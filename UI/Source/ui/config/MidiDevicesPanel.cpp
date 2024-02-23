/**
 * A form panel for configuring MIDI devices.
 */

#include <JuceHeader.h>

#include "../../util/Trace.h"
#include "../../model/MobiusConfig.h"

#include "../common/Form.h"
#include "../common/Field.h"
#include "../JuceUtil.h"

#include "ConfigEditor.h"
#include "MidiDevicesPanel.h"

MidiDevicesPanel::MidiDevicesPanel(ConfigEditor* argEditor) :
    ConfigPanel{argEditor, "MIDI Devices", ConfigPanelButton::Save | ConfigPanelButton::Cancel, true}
{
    setName("MidiDevicesPanel");
    render();
}

MidiDevicesPanel::~MidiDevicesPanel()
{
    // members will delete themselves
}

//////////////////////////////////////////////////////////////////////
//
// ConfigPanel overloads
//
//////////////////////////////////////////////////////////////////////

/**
 * Called by ConfigEditor when asked to edit devices.
 * Unlike most other config panels, we don't have a lot of complex state to manage.
 */
void MidiDevicesPanel::load()
{
    if (!loaded) {

        trace("MIDI Input Devices\n");
        juce::Array<juce::MidiDeviceInfo> inputs = juce::MidiInput::getAvailableDevices();
        for (auto input : inputs) {
            trace("  %s %s\n", input.name, input.identifier);
        }

        trace("MIDI Output Devices\n");
        juce::Array<juce::MidiDeviceInfo> outputs = juce::MidiOutput::getAvailableDevices();
        for (auto output : outputs) {
            trace("  %s %s\n", output.name, output.identifier);
        }
        
        MobiusConfig* config = editor->getMobiusConfig();
        if (config != nullptr) {
            const char* inputName = config->getMidiInput();
            const char* outputName = config->getMidiOutput();
            // do we really need this?
            //const char* thruName = config->getMidiThrough();
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
void MidiDevicesPanel::save()
{
    if (changed) {
        MobiusConfig* config = editor->getMobiusConfig();

        // config->setMidiInput(...selected...);
        // config->setMidiOutput(...selected...);
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
void MidiDevicesPanel::cancel()
{
    loaded = false;
    changed = false;
}

//////////////////////////////////////////////////////////////////////
//
// Internal Methods
// 
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
// Form Rendering
//
//////////////////////////////////////////////////////////////////////

void MidiDevicesPanel::render()
{
    initForm();
    form.render();

    // place it in the content panel
    content.addAndMakeVisible(form);

    // at this point the component hierarhcy has been fully constructed
    // but not sized, until we support bottom up sizing start with
    // a fixed size, this will cascade resized() down the child hierarchy

    // until we get auto-sizing worked out, make this plenty wide
    // MainComponent is currently 1000x1000
    setSize (900, 600);
}

void MidiDevicesPanel::initForm()
{
    inputField = new Field("Input device", Field::Type::String);
    form.add(inputField);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

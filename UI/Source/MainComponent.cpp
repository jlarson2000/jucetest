/**
 * Auto generated by Projucer
 * Not sure how much we can do with this without Projucer overwriting
 * it if the configuration changes.
 */

#include <JuceHeader.h>

#include "MainComponent.h"
#include "util/Trace.h"
#include "ui/JuceUtil.h"

#include "mobius/MobiusInterface.h"

MainComponent::MainComponent()
{
    // should this go here or lower?
    MobiusInterface::startup();

    addAndMakeVisible(mainMenu);
    mainMenu.setListener(this);

    addChildComponent(test);
    addChildComponent(table);
    addChildComponent(tabs);

    // Make sure you set the size of the component after
    // you add any child components.  This will cascade resized()
    setSize (1000, 1000);

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (2, 2);
    }
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();

    MobiusInterface::shutdown();
}

//////////////////////////////////////////////////////////////////////
//
// AudioAppComponent
//
//////////////////////////////////////////////////////////////////////

void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
    bufferToFill.clearActiveBufferRegion();
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

//////////////////////////////////////////////////////////////////////
//
// Component
//
//////////////////////////////////////////////////////////////////////

void MainComponent::paint (juce::Graphics& g)
{
    // You can add your drawing code here!
    g.fillAll (juce::Colours::black);
}

/**
 * This is called when the MainContentComponent is resized.
 * If you add any child components, this is where you should
 * update their positions.
 */
void MainComponent::resized()
{
    // !! still don't understand how we do components that
    // want to set their own size, try passing zero for height
    // and let it ignore it
    mainMenu.setBounds(0, 0, getWidth(), mainMenu.getPreferredHeight());

    // ConfigEditor will set its own size
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
void MainComponent::mainMenuSelection(int id)
{
    switch (id)
    {
        case MainMenu::OpenLoop:
        {
        }
        break;
        case MainMenu::OpenProject:
        {
        }
        break;
        case MainMenu::SaveLoop: break;
        case MainMenu::SaveProject: break;
        case MainMenu::QuickSave: break;
        case MainMenu::ReloadScripts: break;
        case MainMenu::ReloadOSC: break;
        case MainMenu::Exit: break;

        case MainMenu::Presets: {
            configEditor.showPresets();
        }
        break;
        case MainMenu::TrackSetups: {
            configEditor.showSetups();
        }
        break;
                
        case MainMenu::GlobalParameters: {
            configEditor.showGlobal();
        }
        break;
                
        case MainMenu::MIDIControl: {
            test.setVisible(true);
            test.center();
            JuceUtil::dumpComponent(&test);
        }
        break;
        
        case MainMenu::KeyboardControl:  {
            // table.show();
            tabs.show();
        }
            break;
            
        case MainMenu::Buttons: {
            configEditor.showButtons();
        }
        break;

        case MainMenu::PluginParamters: break;
        case MainMenu::DisplayComponents: break;
        case MainMenu::Palette: break;
        case MainMenu::Scripts: break;
        case MainMenu::Samples: break;
        case MainMenu::MIDIDevices: break;
        case MainMenu::AudioDevices: break;

        case MainMenu::KeyBindings: break;
        case MainMenu::MIDIBindings: break;
        case MainMenu::RefreshUI: break;
        case MainMenu::About: break;
                
        default: {
            trace("Unknown menu item: %d\n", id);
        }
        break;
    }
}

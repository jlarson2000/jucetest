//
// 

#include <JuceHeader.h>

#include "MainComponent.h"
#include "MainMenu.h"
#include "Trace.h"

MainComponent::MainComponent()
{
    // add subcomponents before setting size
    addAndMakeVisible (titleLabel);
    titleLabel.setFont (juce::Font (16.0f, juce::Font::bold));
    titleLabel.setText ("Click in the white box to enter some text...", juce::dontSendNotification);
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::lightgreen);
    titleLabel.setJustificationType (juce::Justification::centred);

    addAndMakeVisible (anotherLabel);
    anotherLabel.setFont (juce::Font (16.0f, juce::Font::bold));
    anotherLabel.setText ("More...", juce::dontSendNotification);
    anotherLabel.setColour (juce::Label::textColourId, juce::Colours::lightgreen);
    anotherLabel.setJustificationType (juce::Justification::centred);

    addAndMakeVisible(mainMenu);
    // figure out a better way to do this without casting since
    // we're already passing it to the constructor
    mainMenu.setMainComponent(this);
    
    //addAndMakeVisible(popup);
    addChildComponent(presetPopup);
    presetPopup.setListener(this);
    
    // Make sure you set the size of the component after
    // you add any child components.
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

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    titleLabel.setBounds (10,  100, getWidth() - 20,  30);
    anotherLabel.setBounds (10,  130, getWidth() - 20,  30);
    
    // would prefer that we do this when it is opened?
    presetPopup.center();
}

//////////////////////////////////////////////////////////////////////
//
// Menu Callbacks
//
//////////////////////////////////////////////////////////////////////

/**
 * Called by the menu when the popup is to be shown.
 * I guess you can have more than one of these open at a time which
 * isn't terrible, but since you can't move them, it could get confusing.
 * Could simulate modal with a flag.
 
 */
void MainComponent::showPresets()
{
    presetPopup.setVisible(true);
}

void MainComponent::showSetups()
{
    setupPopup.setVisible(true);
}

//////////////////////////////////////////////////////////////////////
//
// Popup Listeners
//
//////////////////////////////////////////////////////////////////////

void MainComponent::configPopupClosed(ConfigPopup* p)
{
    if (p == &presetPopup)
      presetPopup.setVisible(false);
    else if (p == &setupPopup)
      setupPopup.setVisible(false);
}

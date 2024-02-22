/**
 * Auto generated by Projucer
 * Not sure how much we can do with this without Projucer overwriting
 * it if the configuration changes.
 */

#include <JuceHeader.h>

#include "MainComponent.h"

#include "Supervisor.h"
#include "ui/JuceUtil.h"
#include "util/Trace.h"

MainComponent::MainComponent()
{
    // Jeff's component tree debugging hack
    setName("MainComponent");

    addKeyListener(this);
    
    // startup can do a lot of thigns, perhais we should have different
    // phases, first to load any configuration related to the initial window size
    // and device configuration, and then another to start up the engine
    
    supervisor.start();

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

    // Normally you're supposed to set the size of the component after adding
    // children so the resize is triggered and cascades down.  For us,
    // we might want to let the child configuration determine the optimal
    // window size and pass it back up.  Or I guess just let Supervisor do
    // this when ready.

    // start with a size large enough to give us room but still display
    // on most monitors
    
    setSize (1200, 800);
}

MainComponent::~MainComponent()
{
    // I guess do this before audio shutdown to make sure we're
    // not getting any lingering interrupts
    supervisor.shutdown();

    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

/**
 * Return true to indiciate that the key has been consumed.
 */
bool MainComponent::keyPressed(const juce::KeyPress& key, juce::Component* originator)
{
    return keyTracker.keyPressed(key, originator);
}

bool MainComponent::keyStateChanged(bool isKeyDown, juce::Component* originator)
{
    return keyTracker.keyStateChanged(isKeyDown, originator);
}

//////////////////////////////////////////////////////////////////////
//
// AudioAppComponent
//
//////////////////////////////////////////////////////////////////////

// see this thread when you get ready to start handling audio buffers
// https://forum.juce.com/t/using-smart-pointers-with-juce-functions-that-want-raw-pointers/29229/12


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

    // start with basic black, always in style
    // DisplayManager can override this, should we even bother?

    g.fillAll (juce::Colours::black);
}

/**
 * This is called when the MainContentComponent is resized.
 * If you add any child components, this is where you should
 * update their positions.
 */
void MainComponent::resized()
{
    // jsl - This does not cascade through the children automatically
    // unless you call setSize on them, it's unusual here because of the
    // deferred adding of children by DisplayManager, so assume we have
    // something and let it fill us up with hope and wonder
    juce::Component* child = getChildComponent(0);
    if (child != nullptr)
      child->setBounds(getLocalBounds());
}

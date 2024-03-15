#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    addAndMakeVisible(log);
    log.add("Hello world!\n");
    setSize (1024, 768);

    server.start();
}

MainComponent::~MainComponent()
{
    server.stop();
}

//==============================================================================

void MainComponent::paint (juce::Graphics& g)
{
}

void MainComponent::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    log.setBounds(getLocalBounds());
}

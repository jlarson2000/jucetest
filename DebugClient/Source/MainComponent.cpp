
#include <JuceHeader.h>

#include "LogPanel.h"
#include "ConsolePanel.h"
#include "DebugClient.h"

#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    addAndMakeVisible(console);
    console.add("Starting console");

    console.setListener(&client);
    client.connect();
    console.prompt();
    
    setSize (1024, 768);
}

MainComponent::~MainComponent()
{
}

void MainComponent::paint (juce::Graphics& g)
{
}

void MainComponent::resized()
{
    console.setBounds(getLocalBounds());
}

//==============================================================================


#include <JuceHeader.h>

#include "LogPanel.h"
#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    addAndMakeVisible(console);
    console.add("Starting console");
    console.prompt(">");
    setSize (1024, 768);
}

MainComponent::~MainComponent()
{
}

void MainComponent::addLog(juce::String msg)
{
}

void MainComponent::doTest()
{
    juce::StreamingSocket* socket = new juce::StreamingSocket();

    // bind didn't work, try connect
#if 0    
    bool success = socket->bindToPort(9000);
    if (!success) {
        addLog("Unable to bind to port");
    }
#endif
    bool success = socket->connect("localhost", 9000, 1000);
    if (!success) {
        addLog("Unable to connect");
    }
    else {
        // first arg true for readyForReading
        // second arg timeout milliseconds
        int status = socket->waitUntilReady(false, 5000);
        
        if (status < 0) {
            addLog("waitUntilReady error");
        }
        else if (status == 0) {
            addLog("waitUntilReady timeout");
        }
        else {
            addLog("waitUntilReady ready");
        }

        const char* msg = "Client says hello!";
        status = socket->write(msg, strlen(msg));

        if (status < 0) {
            addLog("societ write error");
        }
        else {
            addLog(juce::String("Wrote ") + juce::String(status) +
                   " bytes");
        }
    }

    delete socket;
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
}

void MainComponent::resized()
{
    console.setBounds(getLocalBounds());
}

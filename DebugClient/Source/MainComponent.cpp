
#include <JuceHeader.h>

#include "LogPanel.h"
#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    addAndMakeVisible(log);
    log.add("Client says hello!\n");
    setSize (1024, 768);

    doTest();
}

MainComponent::~MainComponent()
{
}

/**
 * Code from an example said if lockWasGained returns false:
 * 
 * "if something is trying to kill this job the lock will fail
 *  in which case we better return".
 * 
 * In the original Mobius MainThread, this did a return; which
 * would exit the run() loop and presumably end the thread.
 * So this seems to be not a normal situation that we're not
 * expected to recover.  End the thread so the app can shut down.
 */
bool MainComponent::addLog(juce::String msg)
{
    int success = false;
    
    const juce::MessageManagerLock mml (juce::Thread::getCurrentThread());
    success = mml.lockWasGained();
    if (success)
      log.add(msg);

    return success;
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
    log.setBounds(getLocalBounds());
}

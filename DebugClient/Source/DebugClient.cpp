
#include <JuceHeader.h>

#include "ConsolePanel.h"
#include "DebugClient.h"

DebugClient::DebugClient(ConsolePanel* cp)
{
    console = cp;
}

DebugClient::~DebugClient()
{
    if (socket != nullptr) {
        socket->close();
        delete socket;
    }
}

/**
 * Will need a lot more configurability here...
 */
void DebugClient::connect()
{
    if (socket != nullptr && !socket->isConnected()) {
        // server may have absonded this, try again
        console->add("Previous socket disconnected");
        delete socket;
        socket = nullptr;
    }
    
    if (socket == nullptr) {
        juce::StreamingSocket* maybeSocket = new juce::StreamingSocket();

        bool success = maybeSocket->connect("localhost", 9000, 1000);
        if (!success) {
            console->add("Unable to connect to server");
        }
        else {
            // first arg true for readyForReading
            // second arg timeout milliseconds
            int status = maybeSocket->waitUntilReady(false, 5000);
        
            if (status < 0) {
                console->add("waitUntilReady error");
            }
            else if (status == 0) {
                console->add("waitUntilReady timeout");
            }
            else {
                socket = maybeSocket;
            }
        }
        if (socket != maybeSocket)
          delete maybeSocket;
    }
}

void DebugClient::lineReceived(juce::String line)
{
    connect();
    if (socket != nullptr) {
        int expected = line.length();
        int status = socket->write(line.toUTF8(), expected);
        if (status == -1) {
            console->add("Error writing to socket");
            delete socket;
            socket = nullptr;
        }
        else if (status != expected) {
            console->add("Socket write anomoly");
            console->add(juce::String("Expected ") +
                         juce::String(expected) +
                         ", sent " + juce::String(status));
        }
    }
    else {
        console->add("Socket not open");
    }
}

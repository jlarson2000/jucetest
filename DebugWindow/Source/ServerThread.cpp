
#include <JuceHeader.h>

#include "LogPanel.h"
#include "ClientThread.h"
#include "ServerThread.h"

ServerThread::ServerThread(LogPanel* argLog) :
    Thread(juce::String("Debug Log Server"))  // second arg is threadStackSize
{
    log = argLog;
}

ServerThread::~ServerThread()
{
}

void ServerThread::start()
{
    juce::Thread::RealtimeOptions options;

    // this is what Mobius does, we probably don't need
    // 1ms period
    options.withPriority(10);
    options.withPeriodMs(1);

    if (!startRealtimeThread(options)) {
        // don't have trace over here, what does DBG do?
        //trace("Unable to start thread\n");
        printf("Unable to start thread\n");
    }
}

void ServerThread::stop()
{
    // supposed to cause waitForNextConnection to stop blocking
    // unclear who should be calling stop()
    // added a call in stopThreads, but so does ~MainComponent
    // which gets an error if you try to close it again
    if (socket != nullptr) {
        socket->close();
        delete socket;
        socket = nullptr;
    }
    
    for (int i = 0 ; i < clients.size() ; i++) {
        ClientThread* client = clients[i];
        client->stop();
    }
    clients.clear();
    
    // example says: allow 2 seconds to stop cleanly - should be plenty of time
    if (!stopThread(2000)) {
        //trace("Unable to stop thread\n");
        printf("Unable to stop thread\n");
    }
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
bool ServerThread::addLog(juce::String msg)
{
    int success = false;
    
    const juce::MessageManagerLock mml (juce::Thread::getCurrentThread());
    success = mml.lockWasGained();
    if (success)
      log->add(msg);

    return success;
}

/**
 * Wait for a connection request, then spawn a ClientThread to
 * receive messages on it.
 */
void ServerThread::run()
{
    addLog("Server thread starting!");
    
    socket = new juce::StreamingSocket();

    // unclear whether we're supposed to bind or go directly to createListener
    // bind gets an error so listener it is
    //bool success = socket->bindToPort(9000);

    bool success = socket->createListener(9000);
    if (!success) {
        addLog("Server: Unable to create listener");
    }
    else {
        bool error = false;

        while (!error && !threadShouldExit()) {
        
            // this can hang indefnitely and there is no timeout
            // not sure if that's a Juce limitation or the underlying API
            // forum chatter says to break out of this you need to close the
            // socket we're listening on
            addLog("Server: Waiting for connection");
            juce::StreamingSocket* connection = socket->waitForNextConnection();

            if (connection == nullptr) {
                // what does this mean?  did it time out and we're
                // supposed to wait again?
                addLog("Server: waitForConnection returned null");
                error = true;
            }
            else {
                addLog("Server: Connection received");
                int cnum = clients.size() + 1;
                ClientThread* ct = new ClientThread(cnum, connection, log);
                clients.add(ct);
                ct->run();
            }
        }
    }

    addLog("Server: Deleting socket");
    //delete socket;
}


#include <JuceHeader.h>

#include "LogPanel.h"
#include "ServerThread.h"

ServerThread::ServerThread(LogPanel* argLog) :
    Thread(juce::String("Mobius Debug Log"))  // second arg is threadStackSize
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
 * I think in normal multi-client server situations, the server waits
 * for a connection then spawns a new thread to handle that connection
 * until it is broken.
 *
 * Here we'll start expecting just one at a time.
 */
void ServerThread::run()
{
    addLog("Hello from the Server thread!");
    
    juce::StreamingSocket* socket = new juce::StreamingSocket();
    //bool success = socket->bindToPort(9000);
    bool success = true;
    
    if (!success) {
        addLog("Unable to bind to port");
    }
    else {
        // unclear whether we're supposed to do both bind
        // and createListener, I think just one
        success = socket->createListener(9000);
        if (!success) {
            addLog("Unable to create listener");
        }
        else {
            // ordinarlly I think we would be in a loop waiting
            // for connections, spawning threads to handle them,
            // and when the connection is broken, waiting for another one
            addLog("Waiting for connection");
            juce::StreamingSocket* connection = socket->waitForNextConnection();

            if (connection == nullptr) {
                // what does this mean?  did it time out and we're
                // supposed to wait again?
                addLog("waitForConnection returned null");
            }
            else {
                // waitUntilReady "waits until the socket is ready for
                // reading or writing"
                // unclear if this is necessary
                addLog("Connection received");
                // first arg true means "ready for reading"
                // second arg is timeout in milliseconds
                int status = connection->waitUntilReady(true, 5000);
                if (status < 0) {
                    addLog("waitUntilReady error");
                }
                else if (status == 0) {
                    addLog("waitUntilReady timeout");
                }
                else {
                    addLog("waitUntilReady ready");

                    // unclear whether the loop needs to surround
                    // waitUntilReady or if that's a one time thing

                    // paranioa, make it a little larger than what we
                    // ask for
                    char inbuf[1016];

                    // third arg is blockUntilSpecifiedAmountHasArrived
                    // the data being sent may be broken up into multiple
                    // packets so we have to reassemble that
                    int bytes = connection->read(&inbuf, 1000, false);
                    if (bytes < 0) {
                        addLog("Socket read error");
                    }
                    else {
                        addLog(juce::String("Socket read bytes ") +
                               juce::String(bytes));
                        
                        // assume it's text
                        inbuf[bytes] = 0;
                        addLog(inbuf);
                    }
                }

                addLog("Deleting connection");
                delete connection;
            }
        }
    }
    addLog("Deleting socket");
    delete socket;
}

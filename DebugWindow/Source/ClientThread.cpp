
#include <JuceHeader.h>

#include "LogPanel.h"
#include "ServerThread.h"
#include "ClientThread.h"

ClientThread::ClientThread(int num, juce::StreamingSocket* argSocket,
                           LogPanel* argLog) :
    // second arg is threadStackSize
    Thread(juce::String("Client " + juce::String(num)))
{
    clientNumber = num;
    socket = argSocket;
    log = argLog;
}

ClientThread::~ClientThread()
{
}

void ClientThread::start()
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

void ClientThread::stop()
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
bool ClientThread::addLog(juce::String msg)
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
void ClientThread::run()
{
    addLog(juce::String("Starting client thread " +
                        juce::String(clientNumber)));

    // unclear if we need to call waitUntilReady once before the loop
    // or once at the start of each loop, or even if we need it at all
    
    // first arg true means "ready for reading"
    // second arg is timeout in milliseconds
    // getting timeout on this, skip it?
    bool ready = true;

    if (!ready) {
        int status = socket->waitUntilReady(true, 5000);
        if (status < 0) {
            addLog("waitUntilReady error");
        }
        else if (status == 0) {
            addLog("waitUntilReady timeout");
        }
        else {
            ready = true;
        }
    }

    if (ready) {
        bool error = false;
        
        while (!error && !threadShouldExit()) {

            // paranioa, make it a little larger than what we ask for
            char inbuf[1016];

            // third arg is blockUntilSpecifiedAmountHasArrived
            // the data being sent may be broken up into multiple
            // packets so we have to reassemble that
            int bytes = socket->read(&inbuf, 1000, false);
            if (bytes < 0) {
                addLog("Socket read error");
                // tolerate this or shut down now?
                error = true;
            }
            else if (bytes == 0) {
                // seem to get a lot of these
            }
            else {
                //addLog(juce::String("Socket read bytes ") +
                //juce::String(bytes));
                        
                // assume it's text
                inbuf[bytes] = 0;
                addLog(inbuf);
            }
        }
    }

    // suppress this if we're destructing?
    addLog(juce::String("Client ") + juce::String(clientNumber) +
           juce::String(" shutting down"));

    delete socket;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


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

void ServerThread::run()
{
    // threadShouldExit returns true when the stopThread method is called
    while (!threadShouldExit()) {

        // refresh the UI every 10ms
        // Juce example has this, where does it come from?
        wait(100);

        // from the Juce example
        // because this is a background thread, we mustn't do any UI work without
        // first grabbing a MessageManagerLock..
        const juce::MessageManagerLock mml (Thread::getCurrentThread());
        if (!mml.lockWasGained()) {
            // if something is trying to kill this job the lock will fail
            // in which case we better return
            return;
        }

        // thread is locked, we can mess with components

        log->add("Hello from the Server thread!\n");
    }
}

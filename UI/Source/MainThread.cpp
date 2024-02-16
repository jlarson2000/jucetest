/*
 * Initial attempt at using the Juce thread library to manage the
 * single Mobius master thread.
 *
 * Chatter says that c++11 std:;thread is nicer and Jules said he's like
 * to use it, that was from 2013 so maybe it does by now.  Start here and
 * look at std:: later, my needs are few beyond millisecond accuracy.
 *
 * Focusing on UI refresh needs until we need more resolution for
 * MIDI output clocks.
 */

#include <JuceHeader.h>

#include "util/Trace.h"
#include "MainThread.h"

MainThread::MainThread() :
    Thread(juce::String("Mobius"))  // second arg is threadStackSize
{
}

MainThread::~MainThread()
{
}

void MainThread::start()
{
    juce::Thread::RealtimeOptions options;

    // not sure how much all of this is necessary
    // going to have to be careful about this in the context
    // of a plugin host
    // most comments indicate that this only works for Mac or Posix
    // so start with just priority
    options.withPriority(10);
    options.withPeriodMs(1);

    if (!startRealtimeThread(options)) {
        trace("Unable to start thread\n");
    }
}

void MainThread::stop()
{
    // example says: allow 2 seconds to stop cleanly - should be plenty of time
    if (!stopThread(2000)) {
        trace("Unable to start thread\n");
    }
}

/**
 * Looks like this is similar to Java threads, in that run() is
 * called once and if you have any timing to do you have to handle
 * that yourself.   
 */
void MainThread::run()
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
        processEvents();
    }
}

void MainThread::processEvents()
{
    counter++;

    if (counter > 10) {
        trace("Beep\n");
        counter = 0;
    }
}












    

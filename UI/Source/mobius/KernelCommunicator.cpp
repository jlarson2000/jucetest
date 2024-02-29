
#include <JuceHeader.h>

#include "../util/Trace.h"
#include "../model/MobiusConfig.h"

#include "KernelCommunicator.h"

//////////////////////////////////////////////////////////////////////
//
// KernelCommunicator
//
//////////////////////////////////////////////////////////////////////

KernelCommunicator::KernelCommunicator()
{
}

KernelCommunicator::~KernelCommunicator()
{
    // should be gone by now but just in case
    delete sentConfiguration;
    delete returnedConfiguration;
}

/**
 * Store a pending configuration in the communicator.
 * If we already have one it means the audio stream is stuck.
 * Returns true if there was an anomoly.
 */
bool KernelCommunicator::sendConfiguration(MobiusConfig* config)
{
    bool warn = true;
    juce::ScopedLock lock (criticalSection);

    if (sentConfiguration != nullptr) {
        // need to be saving things like this somewhere and presenting
        // them with more vigor
        trace("KernelCommunicator: configuration not yet consumed!\n");
        delete sentConfiguration;
        warn = true;
    }
    sentConfiguration = config;
    return warn;
}

/**
 * Kernel receives a pending configuration
 */
MobiusConfig* KernelCommunicator::receiveConfiguration()
{
    juce::ScopedLock lock (criticalSection);
    MobiusConfig* neu = sentConfiguration;
    sentConfiguration = nullptr;
    return neu;
}

/**
 * Return a stale configuration.
 * Unlike send, the kernal can't delete this object if the shell
 * isn't responding, so it leaks
 *
 * Hmm, since the shell's timer thread won't be as responsive
 * as the audio interrupt, would we want a notification mechanism?
 */
void KernelCommunicator::returnConfiguration(MobiusConfig* config)
{
    juce::ScopedLock lock (criticalSection);
    if (returnedConfiguration != nullptr) {
        trace("KernelCommunicator: returned configuration not consumed!\n");
        // it leaks
    }
    returnedConfiguration = config;
}

/**
 * Accept a stale configuration
 * Since all the shell will do is delete it, could just delete it here
 * and return true/false.  What's the most common pattern?
 */
MobiusConfig* KernelCommunicator::acceptConfiguration()
{
    juce::ScopedLock lock (criticalSection);
    MobiusConfig* stale = returnedConfiguration;
    returnedConfiguration = nullptr;
    return stale;
}

//////////////////////////////////////////////////////////////////////
//
// Message Lists
//
//////////////////////////////////////////////////////////////////////




/**
 * Fluff the message pool if it is getting low.
 * This must onlh be called from the shell.
 */
void KernelCommunicator::checkCapacity()
{
    
 
 




/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

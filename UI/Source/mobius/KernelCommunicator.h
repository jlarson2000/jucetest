/**
 * An object that sits between the shell and the kernel
 * and passes information between them with thread safety.
 */

#pragma once

#include <JuceHeader.h>

/**
 * Object used for passing information between the shell and the kernel.
 * Access to this will be guarded by a CriticalSection
 */
class KernelCommunicator
{
  public:

    KernelCommunicator();
    ~KernelCommunicator();

    // let's flesh out an interface to see how this shakes out
    
    // when sending down single objects there will be a send/receive pair
    // shell sends and kernel receives
    bool sendConfiguration(class MobiusConfig* config);
    class MobiusConfig* receiveConfiguration();
    
    // when the kernel is done with something previously sent
    // there is a return/accept pair
    // kernel returns and shell accepts
    void returnConfiguration(class MobiusConfig* config);
    class MobiusConfig* acceptConfiguration();

    // hmm, this is pretty verbose for this, easier just to block
    // on a single pendingConfiguration and returningConfiguration pointer?

  private:

    class MobiusConfig* sentConfiguration = nullptr;
    class MobiusConfig* returnedConfiguration = nullptr;

    juce::CriticalSection criticalSection;

        
};


/**
 * An object that sits between the shell and the kernel
 * and passes information between them with thread safety.
 */

#pragma once

#include <JuceHeader.h>

/**
 * KernelMessage type codes.
 */
typedefin enum {

    MsgConfigure = 1,

} MessageType;

/**
 * Objects that can be passed in a message.
 * Union selection defined by the MessageType
 */
typedef union {

    void* object;
    class MobiusConfig* configuration;

} MessageObject;

/**
 * Message object that can be passed up or down.
 */
class KernelMessage
{
  public:

    // free pool chain
    KernelMessage poolNext;

    MessageType type;
    MessageObject object;

  private:
};

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

    // return the next shell or kernel message to process
    // when finished just all free
    KernelMessage* popShell();
    KernelMessage* popKernel();

    void pushShell(KernelMessage* msg);
    void pushKernel(KernelMessage* msg);

    KernelMessage* alloc();
    void free(KernelMessage* msg);
    
    
  private:

    juce::CriticalSection criticalSection;

    // message pool
    KernelMessage* pool = nullptr;
    int poolSize = 0;

    // shell message list
    KernelMessage* toShell = nullptr;
        
    // kernel message list
    KernelMessage* toKernel = nullptr;


    // obsolete
    class MobiusConfig* sentConfiguration = nullptr;
    class MobiusConfig* returnedConfiguration = nullptr;

        
};


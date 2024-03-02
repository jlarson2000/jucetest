/**
 * An object that sits between the shell and the kernel
 * and passes information between them with thread safety.
 *
 * KernelMessage deifnes a common generic model for all types
 * of messages that pass between the shell and the kernel.
 * It requires a "type" code to specify what kind of message it is,
 * may have a few arguments whose meaning is dependent on the type,
 * and may have a void* to a complex object relevant for that type.
 * Would be nice to work out some more elegant polymorphism here.
 *
 * KernelCommunicator is a singleton object shared by the shell and kernel
 * and contains several lists of KernelMessages.  These are pooled for
 * reuse and prevent memory management within the kernel.  Everying
 * is guarded with CriticalSections.
 *
 */

#pragma once

#include <JuceHeader.h>

//////////////////////////////////////////////////////////////////////
//
// KernelMessage
//
//////////////////////////////////////////////////////////////////////

/**
 * The types of messages
 */
typedef enum {

    /**
     * Passes a MobiusConfiguration object.
     * Ownership of the object passes to the receiver.
     */
    MsgNone = 0,
    MsgConfigure,
    MsgAction,
    MsgSampleTrack

} MessageType;

/**
 * A union of the various objects that can be passed in a message.
 * Don't really need this but it's sligly more visually appealing
 * than a blind cast.
 */
typedef union {

    void* pointer;
    class MobiusConfig* configuration;
    class SampleTrack* sampleTrack;
    class UIAction* action;
    
} MessageObject;

/**
 * A message object that can be passed up or down.
 * Messages may be maintined on one of several linked lists.
 * avoiding vectors right now to reduce memory allocation headaches
 * and a good old fashioned list works well enough and is encapsulated.
 */
class KernelMessage
{
  public:

    // message list chain, nullptr if not on a list
    KernelMessage* next = nullptr;

    // what it is
    MessageType type = MsgNone;

    // what it has
    MessageObject object;

    // todo: a few fixed arguments so we don't have to pass objects

    void init();
};

/**
 * The singleton object used for communciation between the shell and the kernel.
 * Maintains the following message lists.
 *
 *    pool          a pool of free messages
 *    toKernel      a list of message sent from the shell to the kernel
 *    toShell       list of message sent from the kernel to the shell
 *
 * The kernel consumes it's event list at the start of every audio interrupt.
 * The shell consumes it's event list during performMaintenance which is normally
 * called by a timer thread with 1/10 a second interval.
 *
 * During consumption, the receiver will call either popShell() or popKernel()
 * to obtain the next message in the queue.  After processing it should return
 * this to the pool with free().
 *
 * During interval processing a message to be sent is allocated with alloc(),
 * filled out with content, then added to one of the lists with either pushShell()
 * or pushKernel()
 *
 * Only the shell is allowed to periocially call checkCapacity() which will
 * make sure that the internal message pool is large enough to handle future
 * message allocations.
 *
 * If alloc() is called and the pool is empty, it will return nullptr.
 * In normal use this is almost always an indication of a memory leak.
 * In theory, a period of extremely intense activity could need more messages
 * than we have available but that really shouldn't happen in practice.
 * Rogue scripts would be the only possible example.
 *
 * Statistics are maintained and may be traced for leak diagnostics.
 */
class KernelCommunicator
{
  public:

    KernelCommunicator();
    ~KernelCommunicator();

    KernelMessage* alloc();
    void free(KernelMessage* msg);
    
    void pushShell(KernelMessage* msg);
    KernelMessage* popShell();
    
    void pushKernel(KernelMessage* msg);
    KernelMessage* popKernel();

    void checkCapacity();
    void traceStatistics();
    
  private:

    juce::CriticalSection criticalSection;

    // the total number of message allocations created with new
    // normally also maxPool
    int totalCreated;

    KernelMessage* pool = nullptr;
    int poolSize = 0;

    KernelMessage* toShell = nullptr;
    int shellSize = 0;
    
    KernelMessage* toKernel = nullptr;
    int kernelSize = 0;

    int minPool = 0;
    int maxShell = 0;
    int maxKernel = 0;
    int poolExtensions = 0;
    
    void deleteList(KernelMessage* list);
        
};

//
// Tuning constants for pool capacity
//

/**
 * The inital size of the pool.
 * This should ideally be set high enough to avoid additional
 * allocations during normal use.
 */
const int KernelPoolInitialSize = 20;

/**
 * The threshold for new allocations.
 * If the free pool dips below this size, another block
 * is allocated.
 */
const int KernelPoolSizeConcern = 5;

/**
 * The number of messages to allocate when the
 * SizeConern threshold is reached.
 */
const int KernelPoolReliefSize = 10;

/**
 * The number of messages on the shell or kernel queue above which
 * we start to question our life choices.
 */
const int KernelPoolUseConcern = 3;

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

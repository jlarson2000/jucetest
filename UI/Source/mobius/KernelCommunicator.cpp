/**
 * An object that sits between the shell and the kernel
 * and passes information between them with thread safety.
 *
 * The implementations of each MessageType is defined in either
 * MobiusShell or MobiusKernel.
 *
 * As this evolves I think there are some generic types we
 * might be able handle here.
 *
 */

#include <JuceHeader.h>

#include "../util/Trace.h"
#include "../model/MobiusConfig.h"

#include "KernelCommunicator.h"

//////////////////////////////////////////////////////////////////////
//
// KernelCommunicator
//
//////////////////////////////////////////////////////////////////////

/**
 * There will only ever be one of these and we need it right away
 * so go ahead and build it out now.  We are in the shell context
 * usualy during static construction of MobiusShell.
 */
KernelCommunicator::KernelCommunicator()
{
    // initialize default pool
    checkCapacity();
}

KernelCommunicator::~KernelCommunicator()
{
    // full stats when debugging, could simplify to
    // just tracing anomolies when things stabalize
    traceStatistics();
    
    deleteList(pool);
    deleteList(toShell);
    deleteList(toKernel);
}

/**
 * Reclaim memory for a message list to avoid exit warnings.
 */
void KernelCommunicator::deleteList(KernelMessage* list)
{
	KernelMessage* next = nullptr;
    while (list != nullptr) {
        next = list->next;
        list->next = nullptr;
        delete list;
        list = next;
    }
}

/**
 * Ensure that the pool has a confortable number of messages
 * available for use.
 *
 * Emit trace messages if we have to grow it.
 *
 * This could also be a place to detect if one of the sides is stuck
 * though there might be better places to do that, and there are bigger
 * problems if it happens.
 *
 * Access to the counters could be unstable in two cases:
 *   - shell is receiving maintenance timer thread events at the samme
 *     time as it is receiving UI thread requests
 *   - kernel is pushing messages at the same time as we're checking
 *
 * Could wrap a csect around all this but those cases are rare
 * and don't result in any danger except some momentary traced size
 * anomolies.  I suppose we might trigger growth a little too soon but
 * if we're that close we're probably going to grow anyway.
 */
void KernelCommunicator::checkCapacity()
{
    if (totalCreated == 0) {
        // we're initialzing
        for (int i = 0 ; i < KernelPoolInitialSize ; i++) {
            KernelMessage* msg = new KernelMessage();
            free(msg);
        }
        minPool = KernelPoolInitialSize;
        totalCreated = KernelPoolInitialSize;
    }
    else if (poolSize < KernelPoolSizeConcern) {
        Trace(1, "KernelCommunicator: pool extension by %d\n", KernelPoolReliefSize);
        Trace(1, "  poolSize %d toKernel %d toShell %d\n",
              poolSize, shellSize, kernelSize);

        // technically should have a csect around this
        int available = poolSize + shellSize + shellUsing + kernelSize + kernelUsing;
        if (available != totalCreated) {
            Trace(1, "KernelCommunicator: leak!  %d Created with %d available\n",
                  totalCreated, available);
        }

        for (int i = 0 ; i < KernelPoolReliefSize ; i++) {
            KernelMessage* msg = new KernelMessage();
            free(msg);
        }
        poolExtensions++;
        totalCreated += KernelPoolReliefSize;
    }
}

/**
 * Trace interesting statistics about the pool
 * Not bothering with a csect on this one but it's easy enough.
 * Depending on the trace interval it's going to hard to catch this
 * in action, but the maximums are interesting.
 */
void KernelCommunicator::traceStatistics()
{
    Trace(1, "KernelCommunicator: statistics\n");
    Trace(1, "  Created %d available %d\n", totalCreated);

    int available = poolSize + shellSize + kernelSize;
    if (totalCreated > available)
      Trace(1, "  Leaked %d\n", totalCreated - available);

    Trace(1, "  min pool %d\n", minPool);
    Trace(1, "  max shell %d\n", maxShell);
    Trace(1, "  max kernel %d\n", maxKernel);

    if (shellSize > 0) {
        Trace(1, "  shell in use %d\n", shellSize);
    }

    if (kernelSize > 0) {
        Trace(1, "  kernel in use %d\n", kernelSize);
    }
}

/**
 * Message initialization
 * This isn't required, but as we start adding things
 * it is nice for debugging to clear out any lingering
 * state from the last message, and in future cases may
 * actually confuse the handler if we don't.
 * Doesn't have to be called within a csect.
 */
void KernelMessage::init()
{
    next = nullptr;
    type = MsgNone;
    object.pointer = nullptr;
}

/**
 * Return a message from the pool.
 * Returns nullptr if the pool is exhausted.
 * This is private, shell/kernel must call one of their usage tracking xxxAlloc methods
 * 
 */
KernelMessage* KernelCommunicator::alloc()
{
    juce::ScopedLock lock (criticalSection);
    KernelMessage* msg = nullptr;

    if (pool == nullptr) {
        Trace(1, "KernelCommunicator: pool has no fucks left to give\n");
    }
    else {
        msg = pool;
        pool = msg->next;
        msg->next = nullptr;
        poolSize--;
        if (poolSize < minPool)
          minPool = poolSize;
    }
    return msg;
}

/**
 * Return a message to the pool.
 * This is private, shell/kernel must call one of their usage tracking
 * xxxAbandon or xxxSend methods
 */
void KernelCommunicator::free(KernelMessage* msg)
{
    if (msg->next != nullptr) {
        Trace(1, "KernelCommunicator: attempt to free message that thinks it is on a list!\n");
    }

    // keep pooled message clean for the next use, could also
    // do this in alloc but it prevents debugger confusion if it
    // is clean while in the pool
    msg->init();

    juce::ScopedLock lock (criticalSection);
    {
        msg->next = pool;
        pool = msg;
        poolSize++;
    }
}

//////////////////////////////////////////////////////////////////////
// Shell Message Processing
//////////////////////////////////////////////////////////////////////

/**
 * Allocate a message for the shell.
 */
KernelMessage* KernelCommunicator::shellAlloc()
{
    KernelMessage* msg = alloc();
    // probably should csect on this if we can have multiple shell-level threads
    shellUsing++;
    return msg;
}

/**
 * Return a message from the shell's list
 * Note that the way this is implemented it's a LIFO rather than FIFO
 * which really shouldn't matter.  If we find a case that does,
 * handle that during insertion.
 */
KernelMessage* KernelCommunicator::shellReceive()
{
    juce::ScopedLock lock (criticalSection);

    KernelMessage* msg = toShell;
    if (msg != nullptr) {
        toShell = msg->next;
        msg->next = nullptr;
        shellSize--;
        shellUsing++;
    }
    return msg;
}

/**
 * Shell decided not to use this, after all the work we did for it.
 */
void KernelCommunicator::shellAbandon(KernelMessage* msg)
{
    free(msg);
    shellUsing--;
}

/**
 * Add a message to the kernel's list
 */
void KernelCommunicator::shellSend(KernelMessage* msg)
{
    juce::ScopedLock lock (criticalSection);

    // since we must be in the shell, check capacity every time
    // to extend the pool if necessary, seeing exhaustion when
    // twisting control knobs rapidly and a lot of UIAction events come
    // in during the 1/10 second maintenance interval
    // could be smarter about merging unprocessed actions, but the Kernel keeps
    // up pretty well as it is on audio interrupttiming, the shell waits longer
    checkCapacity();
    
    if (msg->next != nullptr) {
        Trace(1, "KernelCommunicator: attempt to push message that thinks it is on a list!\n");
    }

    msg->next = toKernel;
    toKernel = msg;
    kernelSize++;
    if (kernelSize > maxKernel)
      maxKernel = kernelSize;
    shellUsing--;
}

//////////////////////////////////////////////////////////////////////
// Kernel Message Processing
//////////////////////////////////////////////////////////////////////

/**
 * Allocate a message for the kernel.
 */
KernelMessage* KernelCommunicator::kernelAlloc()
{
    KernelMessage* msg = alloc();
    // probably should csect on this if we can have multiple shell-level threads
    kernelUsing++;
    return msg;
}

/**
 * Return a message from the kernel's list
 */
KernelMessage* KernelCommunicator::kernelReceive()
{
    juce::ScopedLock lock (criticalSection);

    KernelMessage* msg = toKernel;
    if (msg != nullptr) {
        toKernel = msg->next;
        msg->next = nullptr;
        kernelSize--;
        kernelUsing++;
    }
    return msg;
}

/**
 * Shell decided not to use this, after all the work we did for it.
 */
void KernelCommunicator::kernelAbandon(KernelMessage* msg)
{
    free(msg);
    kernelUsing--;
}

/**
 * Add a message to the shell's list
 * See comments in popShell for potential issues with ordering.
 */
void KernelCommunicator::kernelSend(KernelMessage* msg)
{
    juce::ScopedLock lock (criticalSection);

    if (msg->next != nullptr) {
        Trace(1, "KernelCommunicator: attempt to push message that thinks it is on a list!\n");
    }

    msg->next = toShell;
    toShell = msg;
    shellSize++;
    if (shellSize > maxShell)
      maxShell = shellSize;
    kernelUsing--;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

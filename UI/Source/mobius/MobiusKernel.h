/**
 * An evolving object that wraps "kernel" state and functions.
 *
 * This is the stuff that runs in the audio thread and communictaion
 * must be strictly controlled between this and the "shell".
 *
 * In old code a single Mobius class contained both shell and kernel
 * code and data.  Due to the extensive refactoring necessary for the
 * Juce migration it didn't really make sense to retain any of that so
 * we'll start evolving things to the new model as they are ported over.
 *
 * Extreme caution should be made calling functions on this object.
 * That should only be done by kernel code.  Shell code normally uses
 * the a KernelCommunication object to pass information back and forth
 * between the threads.
 *
 * Ideally we should make all the kernal-only functions protected but
 * that would require great gobs of friend classes.  Probably better to have
 * the shell use an interface instead.
 *
 */

#pragma once

#include "MobiusContainer.h"
#include "KernelEvent.h"

class MobiusKernel : public MobiusContainer::AudioListener
{
  public:

    MobiusKernel(class MobiusShell* shell, class KernelCommunicator* comm);
    ~MobiusKernel();

    /**
     * Initialize the kernel prior to it being active.
     * The difference between this and what we pass in the constructor
     * is kind of arbitrary, consider doing it one way or the other.
     * Or just pulling it from the MobiusShell
     */
    void initialize(class MobiusContainer* cont, class MobiusConfig* config);

    /**
     * Respond to a shell request for a core parameter.
     * Temporary interface, to be replaced by UIQuery at some point.
     */
    int getParameter(class UIParameter* p, int trackNumber);

    // normally this should be private, but leave it open for the shell for testing
    void consumeCommunications();

    // AudioListener
    void containerAudioAvailable(MobiusContainer*cont);

    // stuff being added for Mobius core
    class AudioPool* getAudioPool() {
        return audioPool;
    }

    class MobiusConfig* getMobiusConfig() {
        return configuration;
    }

    class MobiusContainer* getContainer() {
        return container;
    }

    class MobiusState* getState();

    // event management
    class KernelEvent* newEvent() {
        return eventPool.getEvent();
    }

    // does this need to be public?
    // what would call newEvent but not sendEvent?
#if 0    
    void returnEvent(KernelEvent* e) {
        eventPool.returnEvent(e);
    }
#endif
    
    void sendEvent(KernelEvent* e);
    
  private:

    // stuff we are either passed or pull from the shell
    class MobiusShell* shell = nullptr;
    class KernelCommunicator* communicator = nullptr;
    class MobiusContainer* container = nullptr;
    class MobiusConfig* configuration = nullptr;
    class AudioPool* audioPool = nullptr;

    // this we own
    KernelEventPool eventPool;
    
    // internal state
    //
    // slowly start dragging stuff over
    // use static members eventually
    
    class SampleManager* samples = nullptr;

    // the big guy
    // make this a stack object at some point
    class Mobius* mCore = nullptr;
    class UIAction* coreActions = nullptr;

    // KernelMessage handling
    void reconfigure(class KernelMessage*);
    void installSamples(class KernelMessage* msg);
    void doAction(KernelMessage* msg);
    void doEvent(KernelMessage* msg);
    
};


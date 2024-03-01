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

    // normally this should be private, but leave it open for the shell for testing
    void consumeCommunications();

    // AudioListener
    void containerAudioAvailable(MobiusContainer*cont);

  private:

    // stuff we are either passed or pull from the shell
    class MobiusShell* shell = nullptr;
    class KernelCommunicator* communicator = nullptr;
    class MobiusContainer* container = nullptr;
    class MobiusConfig* configuration = nullptr;
    class AudioPool* audioPool = nullptr;
    
    // internal state
    //
    // slowly start dragging stuff over
    // use static members eventually
    
    class Recorder* mRecorder = nullptr;
    //class SampleTrack* mSampleTrack = nullptr;

    // AudioListener activities
    void interruptStart();
    void interruptEnd();

    // KernelMessage handling
    void reconfigure();
    void initRecorder();
    //void installSamples(class SamplePack* pack);

};


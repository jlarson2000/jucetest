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
 * While this is currently part of MobiusSimulator, it is really the slow
 * evolution of the real kernel.
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

// for CriticalSection
#include <JuceHeader.h>

class MobiusKernel
{
  public:

    MobiusKernel(class MobiusSimulator* shell, class KernelCommunicator* comm);
    ~MobiusKernel();

    /**
     * Give the kernel an object representing the audio interface.
     */
     void setAudioInterface(class AudioInterface* ai);
    
    /**
     * Allow the kernel to configure itself after construction.
     * This copy of MobiusConfig will be owned by the Kernel and
     * is different than the copy owned by the shell.
     */
    void setInitialConfiguration(class MobiusConfig* config);

    // normally this should be private, but leave it open for the shell for testing
    void consumeCommunications();

  private:

    class MobiusSimulator* shell = nullptr;
    class KernelCommunicator* communicator = nullptr;
    class AudioInterface* audioInterface = nullptr;
    class MobiusConfig* configuration = nullptr;

    // slowly start dragging stuff over
    // use static members eventually
    class Recorder* mRecorder = nullptr;
    class AudioPool* mAudioPool = nullptr;
    //class SampleTrack* mSampleTrack = nullptr;

    void reconfigure();
    void initObjectPools();
    void initRecorder();
    void installSamples();

};


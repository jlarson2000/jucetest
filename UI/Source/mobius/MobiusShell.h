/*
 * Inner implementation of MobiusInterface that wraps all
 * other Mobius engine logic.
 *
 * This is gradually being fleshed out with some things still
 * provided by a simulation.
 */

#pragma once

#include "../model/MobiusState.h"

#include "MobiusContainer.h"
#include "KernelCommunicator.h"
#include "MobiusKernel.h"
#include "MobiusInterface.h"

class MobiusShell : public MobiusInterface
{
  public:

    MobiusShell(class MobiusContainer* container);
    ~MobiusShell();

    void setListener(class MobiusListener* l);
    void configure(class MobiusConfig* config);
    
    MobiusState* getState();
    void doAction(class UIAction* action);
    int getParameter(Parameter* p, int tracknum = 0);

    void installSamples(SampleConfig* samples);
    void performMaintenance();
    
    // temporary test interfaces
    void simulateInterrupt(float* input, float* output, int frames);
    void test();
    
    // things the Kernel needs
    class AudioPool* getAudioPool();
    class SamplePack* consumePendingSamplePack();

  private:

    class MobiusContainer* container = nullptr;
    MobiusListener* listener = nullptr;
    class MobiusConfig* configuration = nullptr;
    class AudioPool* audioPool = nullptr;
    
    MobiusState state;
    KernelCommunicator communicator;
    MobiusKernel kernel {this, &communicator};
    
    // kernel communication
    void consumeCommunications();
    void kernelConfigure(class MobiusConfig* config);
    // another style of communication, think more about this
    class SamplePack* mPendingSamplePack;
    
    // from here on down is total simulation
    // above is evolving toward being real
    
    void initState();
    void globalReset();
    void doReset(class UIAction* action);
    void reset(class MobiusLoopState* loop);

    void doRecord(class UIAction* action);

    void play(class MobiusLoopState* loop, int bufferFrames);
    void notifyBeatListeners(class MobiusLoopState* loop, long bufferFrames);
    
    void simulateEvents();
    class MobiusEventState* simulateEvent(class MobiusLoopState* loop, class UIEventType* type, int q);

    class MobiusTrackState* getTargetTrack(class UIAction* action);
    void doSwitch(UIAction* action, int next);

    // Parameters
    class Setup* getActiveSetup();
    class SetupTrack* getSetupTrack(int tracknum);
    class Preset* getTrackPreset(class SetupTrack* track);
    class Preset* getTrackPreset(int tracknum);

};


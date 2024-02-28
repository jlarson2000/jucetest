/*
 * Inner implementation of MobiusInterface that wraps all
 * other Mobius engine logic.
 *
 * This provides a simulator for the real engine for UI testing
 */

#pragma once

#include "../model/MobiusState.h"

#include "MobiusContainer.h"
#include "KernelCommunicator.h"
#include "MobiusKernel.h"
#include "MobiusInterface.h"

class MobiusSimulator : public MobiusInterface
{
  public:

    MobiusSimulator(class MobiusContainer* container);
    ~MobiusSimulator();

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
    
  private:

    class MobiusContainer* container = nullptr;
    MobiusListener* listener = nullptr;
    class MobiusConfig* configuration = nullptr;

    MobiusState state;
    KernelCommunicator communicator;
    MobiusKernel kernel {this, &communicator};
    
    // kernel communication
    void consumeCommunications();
    void kernelConfigure(class MobiusConfig* config);
    
    // from here on down is total simulation
    // above is evolving toward being real

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


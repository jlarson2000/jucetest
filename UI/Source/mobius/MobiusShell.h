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
#include "AudioPool.h"
#include "MobiusKernel.h"
#include "MobiusInterface.h"
#include "Simulator.h"

class MobiusShell : public MobiusInterface
{
    friend class MobiusKernel;
    
  public:

    MobiusShell(class MobiusContainer* container);
    ~MobiusShell();

    void setListener(class MobiusListener* l);
    void configure(class MobiusConfig* config);
    
    // UI Control
    
    MobiusState* getState();    // also shared by the kernel
    void doAction(class UIAction* action);
    int getParameter(Parameter* p, int tracknum = 0);
    void installSamples(SampleConfig* samples);

    // management thread 
    void performMaintenance();

    // temporary test interfaces
    void simulateInterrupt(float* input, float* output, int frames);
    void test();
    
  protected:
    
    // accessors for the Kernel only
    class AudioPool* getAudioPool();

  private:

    class MobiusContainer* container = nullptr;
    MobiusListener* listener = nullptr;
    class MobiusConfig* configuration = nullptr;
    
    // kernel communication and shared state
    KernelCommunicator communicator;
    MobiusState state;

    // note that AudioPool must be declared before
    // Kernel so that they are destructed in reverse
    // order and Kernel can return things to the pool
    // before it is destructed
    class AudioPool audioPool;
    
    // the kernel itself
    // todo: try to avoid passing this down, can we do
    // everything with messages?
    MobiusKernel kernel {this, &communicator};
    
    // temporary simulator
    Simulator simulator {this};

    //
    // internal functions
    //
    
    void consumeCommunications();
    void sendKernelConfigure(class MobiusConfig* config);
    void doKernelAction(UIAction* action);
    
};


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
#include "KernelEventHandler.h"
#include "ScriptAnalyzer.h"
#include "UnitTests.h"

class MobiusShell : public MobiusInterface
{
    friend class MobiusKernel;
    friend class KernelEventHandler;
    friend class UnitTests;
    
  public:

    MobiusShell(class MobiusContainer* container);
    ~MobiusShell();

    void setListener(class MobiusListener* l);
    void configure(class MobiusConfig* config);
    
    // UI Control
    
    MobiusState* getState();    // also shared by the kernel
    void doAction(class UIAction* action);
    int getParameter(UIParameter* p, int tracknum = 0);
    
    void installSamples(SampleConfig* samples);
    
    // management thread 
    void performMaintenance();

    // temporary test interfaces
    void simulateInterrupt(float* input, float* output, int frames);
    void test();

    // temporary for ScriptAnalyzer
    class MobiusKernel* getKernel() {
        return &kernel;
    }
    
    class DynamicConfig* getDynamicConfig();
    
    // for internal components like KernelEventHandler

    class MobiusContainer* getContainer() {
        return container;
    }
    
    class MobiusConfig* getConfiguration() {
        return configuration;
    }

  protected:
    
    // accessors for the Kernel only
    class AudioPool* getAudioPool();

  private:

    class MobiusContainer* container = nullptr;
    MobiusListener* listener = nullptr;
    class MobiusConfig* configuration = nullptr;
    
    // kernel communication and shared state
    KernelCommunicator communicator;
    KernelEventHandler kernelEventHandler {this};
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
    bool doSimulation = false;
    Simulator simulator {this};

    ScriptAnalyzer scriptAnalyzer {this};

    // Unit Test Support
    UnitTests unitTests {this};
    
    //
    // internal functions
    //
    
    void consumeCommunications();
    void sendKernelConfigure(class MobiusConfig* config);
    void doKernelAction(UIAction* action);

    //
    // Intrinsic actions
    //

    void doIntrinsic(UIAction* action);
    void loadScripts(UIAction* action);
    void loadSamples(UIAction* action);

};


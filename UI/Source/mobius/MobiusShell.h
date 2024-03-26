/*
 * Inner implementation of MobiusInterface that wraps all
 * other Mobius engine logic.
 *
 * This is gradually being fleshed out with some things still
 * provided by a simulation.
 */

#pragma once

#include "../model/MobiusState.h"
#include "../model/DynamicConfig.h"

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
    friend class AudioDifferencer;
    
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

    // for a small number of things like script analysis that
    // need to live dangerously
    // need to start protecting these
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

    // for internal components like KernelEventHandler
    // that want to notify the listener
    MobiusListener* getListener() {
        return listener;
    }

  protected:
    
    DynamicConfig* getInternalDynamicConfig();

    // accessors for the Kernel only
    class AudioPool* getAudioPool();

    // UnitTests
    void loadSamples(SampleConfig* src, bool unitTestSetup);
    
  private:

    class MobiusContainer* container = nullptr;
    MobiusListener* listener = nullptr;
    class MobiusConfig* configuration = nullptr;
    
    MobiusState simulatorState;
    DynamicConfig dynamicConfig;

    // kernel communication and shared state
    KernelCommunicator communicator;
    KernelEventHandler kernelEventHandler {this};
    
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

    void initDynamicConfig();
    void installDynamicConfig(class SampleManager* samples);
    void installDynamicConfig(class Scriptarian* scripts);
    void removeDynamicActions(class ActionType* type);

    class SampleManager* loadSamples(class SampleConfig* src);
    void sendSamples(class SampleManager* manager, bool safeMode);
    
    class Scriptarian* loadScripts(class ScriptConfig* src);
    void sendScripts(class Scriptarian* manager, bool safeMode);
    
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


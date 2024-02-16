/*
 * Inner implementation of MobiusInterface that wraps all
 * other Mobius engine logic.
 *
 * This provides a simulator for the real engine for UI testing
 */

#pragma once

#include "../model/MobiusState.h"

#include "MobiusInterface.h"

class MobiusSimulator : public MobiusInterface
{
  public:

    MobiusSimulator();
    ~MobiusSimulator();

    void configure(class MobiusConfig* config);
    MobiusState* getState();
    void doAction(class UIAction* action);

    void simulateInterrupt(float* input, float* output, int frames);

    void test();
    
  private:

    class MobiusConfig* configuration;
    MobiusState state;

    void globalReset();
    void doReset(class UIAction* action);
    void reset(class MobiusLoopState* loop);

    void doRecord(class UIAction* action);

    
};


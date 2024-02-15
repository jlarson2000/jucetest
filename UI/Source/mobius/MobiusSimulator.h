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

    void configure(MobiusConfig* config);
    class MobiusState* getState();
    void doAction(UIAction* action);


    void test();
    
  private:

    class MobiusConfig* configuration;
    MobiusState state;
    
};


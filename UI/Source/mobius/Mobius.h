/*
 * Inner implementation of MobiusInterface that wraps all
 * other Mobius engine logic.
 */

#pragma once

#include "MobiusInterface.h"

class Mobius : public MobiusInterface
{
  public:

    Mobius();
    ~Mobius();
    
    MobiusConfig* editConfiguration();
    void saveConfiguration(MobiusConfig* config);

    // run a test
    void test();
    
  private:

    MobiusConfig* readConfiguration();
};


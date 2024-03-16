/**
 * Helper class for MobiusShell that encapsulates code
 * related to prcessing KernelEvents
 */

#pragma once

class KernelEventHandler
{
  public:

    KernelEventHandler(class MobiusShell* argShell) {
        shell = argShell;
    }

    ~KernelEventHandler() {}

    void doEvent(class KernelEvent* e);

  private:

    class MobiusShell* shell;
    
};


 

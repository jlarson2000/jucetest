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
    
    void doSaveLoop(class KernelEvent* e);
    void doSaveAudio(class KernelEvent* e);
    void doSaveProject(class KernelEvent* e);
    void doSaveConfig(class KernelEvent* e);
    void doLoad(class KernelEvent* e);
    void doDiff(class KernelEvent* e);
    void doDiffAudio(class KernelEvent* e);
    void doPrompt(class KernelEvent* e);
    void doEcho(class KernelEvent* e);
    void doTimeBoundary(class KernelEvent* e);

};

 

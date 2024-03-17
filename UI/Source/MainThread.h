
#pragma once

#include <JuceHeader.h>
#include "util/Trace.h"

class MainThread : public juce::Thread, public TraceListener
{
  public:
    
    MainThread(class Supervisor* super);
    ~MainThread();

    void start();
    void stop();

    void run() override;

    // TraceListener
    void traceEvent();

  private:

    class Supervisor* supervisor;

    int counter = 0;
    
};


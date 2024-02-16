
#pragma once

#include <JuceHeader.h>

class MainThread : public juce::Thread
{
  public:
    
    MainThread(class Supervisor* super);
    ~MainThread();

    void start();
    void stop();

    void run() override;

  private:

    class Supervisor* supervisor;

    void processEvents();
    int counter = 0;
    
};


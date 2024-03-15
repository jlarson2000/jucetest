
#pragma once

#include <JuceHeader.h>

class ServerThread : public juce::Thread
{
  public:
    
    ServerThread(class LogPanel* log);
    ~ServerThread();

    void start();
    void stop();

    void run() override;

  private:

    class LogPanel* log;

    int counter = 0;
    
};


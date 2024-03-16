
#pragma once

#include <JuceHeader.h>

class ClientThread : public juce::Thread
{
  public:
    
    ClientThread(int number, class juce::StreamingSocket* socket,
                 class LogPanel* log);
    ~ClientThread();

    void start();
    void stop();

    void run() override;


  private:

    bool addLog(juce::String msg);

    int clientNumber = 0;
    juce::StreamingSocket* socket = nullptr;
    class LogPanel* log = nullptr;

    int counter = 0;
    
};


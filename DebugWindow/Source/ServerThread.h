
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

    bool addLog(juce::String msg);

    class LogPanel* log = nullptr;
    juce::StreamingSocket* socket = nullptr;
    juce::Array<class ClientThread*> clients;
    
};


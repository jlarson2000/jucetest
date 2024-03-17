
#pragma once

#include <JuceHeader.h>
#include "ConsolePanel.h"

class DebugClient : public ConsolePanel::Listener
{
  public:

    DebugClient(class ConsolePanel* console);
    ~DebugClient();

    void connect();
    void lineReceived(juce::String line);

  private:

    ConsolePanel* console;
    juce::StreamingSocket *socket = nullptr;
};

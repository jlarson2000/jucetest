
#include <JuceHeader.h>

class MainThread : public juce::Thread
{
  public:
    
    MainThread();
    ~MainThread();

    void start();
    void stop();

    void run() override;

  private:

    void processEvents();
    int counter = 0;
    
};


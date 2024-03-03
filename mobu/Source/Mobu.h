/**
 * Top level class implementing the Mobu utilities.
 * Instantiated and deleted by Main.cpp
 */

#include <JuceHeader.h>

class Mobu
{
  public:
    
    Mobu(int argc, char* argv[]);
    ~Mobu();

    void run();

  private:

    juce::String fileName;

};

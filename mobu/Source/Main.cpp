/*
  ==============================================================================

    This file contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include <JuceHeader.h>

#include "Mobu.h"

//==============================================================================
int main (int argc, char* argv[])
{
    // jsl - expecting more here, I guess because we didn't
    // ask for any modules that required more of an environment?

    Mobu* mobu = new Mobu(argc, argv);
    mobu->run();
    delete mobu;

    return 0;
}

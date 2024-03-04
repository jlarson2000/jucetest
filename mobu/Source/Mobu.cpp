/**
 * Top level class implementing the Mobu utilities.
 * Instantiated and deleted by Main.cpp
 */

#include <JuceHeader.h>

#include <iostream>

#include "ParameterGenerator.h"
#include "Mobu.h"

Mobu::Mobu(int argc, char* argv[])
{
    if (argc < 2) {
        printf("mobu: <filename>\n");
        // debugging hack till I figure out how to set command line args in VStudio
        fileName = "test.xml";
    }
    else {
        fileName = argv[1];
    }
}

Mobu::~Mobu()
{
}

void Mobu::run()
{
    ParameterGenerator pgen;

    pgen.generate(fileName, false);
}


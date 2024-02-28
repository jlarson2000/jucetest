
#include "MobiusInterface.h"
#include "MobiusSimulator.h"

MobiusInterface* MobiusInterface::Singleton = nullptr;

MobiusInterface::~MobiusInterface()
{
}

MobiusInterface* MobiusInterface::getMobius(MobiusContainer* container)
{
    if (Singleton == nullptr) {
        Singleton = new MobiusSimulator(container);
    }
    return Singleton;
}

void MobiusInterface::startup()
{
    // formerly created the Singleton here but moved it to getMobius
    // what more do we need here, tell Singleton to do something?
}

void MobiusInterface::shutdown()
{
    delete Singleton;
    Singleton = nullptr;
}

/*
MobiusInterface::MobiusInterface()
{
}

MobiusInterface::~MobiusInterface()
{
}
*/


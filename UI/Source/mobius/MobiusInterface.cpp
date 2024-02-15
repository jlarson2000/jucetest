
#include "MobiusInterface.h"
#include "MobiusSimulator.h"

MobiusInterface* MobiusInterface::Singleton = nullptr;

void MobiusInterface::startup()
{
    if (Singleton == nullptr) {
        Singleton = new MobiusSimulator();
    }
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

MobiusInterface* MobiusInterface::getMobius()
{
    return Singleton;
}

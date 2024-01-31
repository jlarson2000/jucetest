
#include "MobiusInterface.h"
#include "Mobius.h"

MobiusInterface* MobiusInterface::Singleton = nullptr;

void MobiusInterface::startup()
{
    if (Singleton == nullptr) {
        Singleton = new Mobius();
    }
}

void MobiusInterface::shutdown()
{
    delete Singleton;
    Singleton = nullptr;
}

MobiusInterface::MobiusInterface()
{
}

MobiusInterface::~MobiusInterface()
{
}

MobiusInterface* MobiusInterface::getMobius()
{
    return Singleton;
}


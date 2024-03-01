/**
 * The Mobius engine shell which interacts with the MobiusContainer
 * and manages the MobiusKernel.
 *
 * This is gradually being fleshed out, parts are still simulated.
 */

#include "../util/Trace.h"
#include "../util/Util.h"

#include "../model/MobiusConfig.h"
#include "../model/MobiusState.h"
#include "../model/ModeDefinition.h"
#include "../model/FunctionDefinition.h"
#include "../model/Parameter.h"
#include "../model/Setup.h"
#include "../model/Preset.h"
#include "../model/UIAction.h"
#include "../model/XmlRenderer.h"
#include "../model/UIEventType.h"

#include "MobiusContainer.h"
#include "MobiusKernel.h"
#include "SampleTrack.h"
#include "Simulator.h"

#include "MobiusShell.h"

//////////////////////////////////////////////////////////////////////
//
// Initialization
//
//////////////////////////////////////////////////////////////////////

MobiusShell::MobiusShell(MobiusContainer* cont)
{
    container = cont;
    // this is given to us later
    configuration = nullptr;

    audioPool = new AudioPool();
}

MobiusShell::~MobiusShell()
{
    delete configuration;

    audioPool->dump();
    delete audioPool;
}

void MobiusShell::setListener(MobiusListener* l)
{
    listener = l;
}

//////////////////////////////////////////////////////////////////////
//
// Configuration
//
//////////////////////////////////////////////////////////////////////

/**
 * This always makes an internal copy of the passed object.
 * It is the responsibility of the caller to either free it
 * or keep using it.
 *
 * Two copies are made, one for the shell and one for the kernel.
 * The kernel copy must be phased in through the communicator.
 * Unless this is the first call during startup.
 *
 * !! not liking this guessing about whether this is the first
 * call or not.  startup processing needs to be defined better.
 * Kernel instantiation vs. initialization is messy, work on this
 */
void MobiusShell::configure(MobiusConfig* config)
{
    bool firstTime =  (configuration == nullptr);
    
    // clone it so we can make internal modifications
    // since we can be called after config editing delete the existing one
    // give this class a proper clone() method so we don't have to use XML
    delete configuration;
    XmlRenderer xr;
    configuration = xr.clone(config);

    // clone it again and give it to the kernel
    MobiusConfig* kernelCopy = xr.clone(config);
    if (firstTime) {
        // initialization mess
        // to do what it does, the Kernel needs to start with
        //   shell - given at construction
        //   communicator - given at construction
        //   container - given here
        //   config - given here the first time, then passed with message
        //   audioPool - immediately calls back to getAudioPool
        //
        // most if not all of this could done the same way, either
        // push it all down once in initialize() or have it pull
        // it one at a time inside initialize, can do some things
        // in the constructor, but not all like audioPool and config
        // 
        // 
        kernel.initialize(container, kernelCopy);
    }
    else {
        sendKernelConfigure(kernelCopy);
    }

    // temporarily simulation to track configuration changes
    simulator.initialize(configuration);
}

/**
 * Install loaded samples.
 * This builds a new SampleTrack and sends it to the kernel.
 * We could do difference detection here, but that should have been
 * doen earlier before we bothered loading it.
 *
 * We take ownership of the object.
 */
void MobiusShell::installSamples(SampleConfig* samples)
{
    SampleTrack* track = new SampleTrack(audioPool, samples);

    // SampleTrack took what it needed and left this behind
    // it didn't actually steal the float buffers
    delete samples;

    KernelMessage* msg = communicator.alloc();
    msg->type = MsgSampleTrack;
    msg->object.sampleTrack = track;
    communicator.pushKernel(msg);
}

//////////////////////////////////////////////////////////////////////
//
// Maintenance
//
//////////////////////////////////////////////////////////////////////

/**
 * Expected to be called at regular small intervals by a thread
 * managed in the UI, usually 1/10 second.
 * 
 * Since Juce is already leaking down here could consider using the
 * Timer directly and registring ourselves rather than having
 * MainThread do it.
 */
void MobiusShell::performMaintenance()
{
    consumeCommunications();
}

//////////////////////////////////////////////////////////////////////
//
// State
//
//////////////////////////////////////////////////////////////////////

MobiusState* MobiusShell::getState()
{
    return &state;
}

//////////////////////////////////////////////////////////////////////
//
// Kernel communication
//
//////////////////////////////////////////////////////////////////////

/**
 * We share an AudioPool with the kernel, once this is called
 * the pool can not be deleted.  Kernel calls back to this,
 * would be cleaner if we just passed that to kernel.initialize()
 *
 */
AudioPool* MobiusShell::getAudioPool()
{
    return audioPool;
}

/**
 * Send the kernel its copy of the MobiusConfig
 * The object is already a copy
 */
void MobiusShell::sendKernelConfgure(MobiusConfig* config)
{
    KernelMessage* msg = communicator.alloc();
    if (msg != nullptr) {
        msg.type = MsgConfigure;
        msg.object.configuration = config;
        communicator.pushKernel(msg);
    }
    // else, pool exhaustion, already traced
}

/**
 * Consume any messages sent back from the kernel.
 */
void MobiusShell::consumeCommunications()
{
    KernelMessage* msg = communicator.popShell();
    while (msg != nullptr) {
        switch (msg->type) {

            case MsgConfigure: {
                // kernel is done with the previous configuration
                delete msg->object.configuration;
            }
                break;
            case MsgSampleTrack: {
                // kernel is giving us back the old SampleTrack
                delete msg->object.sampleTrack;
            }
                break;
        }
        
        communicator.free(msg);
        msg = communicator.popShell();
    }
}



/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


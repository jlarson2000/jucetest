/**
 * An implementation of MobiusContainer that bridge Juce to the old world.
 *
 * As interrupts come in, we need to convert the Juce buffer structure
 * with separate channel arrays into an interleavad array and save them.
 * Then call the listener, which will immediately calls us back and ask for those
 * buffers.  The "ports" concept was to support more than 2 channels.  Say the hardware
 * had 8 channels.  These would be presented as 4 ports with 2 channels each, left and right.
 * In theory you could have more than 2 channels per port for surround but that was
 * never implemented.
 *
 * For the initial integration, we'll assume 2 stereo channels per port and only one port.
 * This is all the RME seems to allow anyway.  Leave multiple ports for another day.
 *
 * Besides buffers, the stream is expected to provide the the sample rate for synchronization.
 * We save that at startup in the Juce prepareToPay call, it can presumably change
 * if you reconfigure the hardware, I believe Juce is supposed to call releasesResources
 * and prepareToPlay with the new rate when that happens, but there is some ambiguity.
 *
 */

#include <JuceHeader.h>

#include "util/Trace.h"
#include "mobius/MobiusContainer.h"

#include "Supervisor.h"
#include "JuceAudioInterface.h"

JuceMobiusContainer::JuceMobiusContainer(Supervisor* s)
{
    supervisor = s;
}

JuceMobiusContainer::~JuceMobiusContainer()
{
}

/**
 * Register the listener to receive notifications as
 * audio buffers come in.
 */
void JuceMobiusContainer::setlistener(MobiusContainer::Listener* l)
{
    listener = l;
}

//////////////////////////////////////////////////////////////////////
//
// MobiusContainer
//
//////////////////////////////////////////////////////////////////////

int JuceMobiusContainer::getInputPorts()
{
    return 1;
}

int JuceMobiusContainer::getOutputPorts()
{
    return 1;
}

int JuceMobiusContainer::getSampleRate()
{
    return sampleRate;
}

int JuceMobiusContainer::getInputLatency()
{
    trace("JuceMobiusContainer::getInputLatencyFrames doesn't want you to know!\n");
    return 0;
}

int JuceMobiusContainer::getOutputLatency()
{
    trace("JuceMobiusContainer::getOutputLatencyFrames doesn't want you to know!\n");
    return 0;
}

/**
 * I don't remember exactly what this was, but it has to do
 * with synchronization so it will be important once we do sync.
 */
double JuceMobiusContainer::getStreamTime()
{
    return 0.0;
}

double JuceMobiusContainer::getLastInterruptStreamTime()
{
    return 0.0;
}

AudioTime* JuceMobiusContainer::getAudioTime()
{
    return nullptr;
}

/**
 * This is called by the AudioHandler during an interrupt to
 * get the buffers to process.  Return the number of samples
 * we saw in the last call to getNextAudioBlock
 */
long JuceMobiusContainer::getInterruptFrames()
{
    return nextBlockSamples;
}

/**
 * Return the inputBuffer containing interleaved samples from
 * getNextAudioBlock and let the handler fill the interleaved
 * output buffer.  The output buffer will have been cleared
 * so it doesn't have to do anything if it doesn't want to.
 */
void JuceMobiusContainer::getInterruptBuffers(int inport, float** input, 
                                             int outport, float** output)
{
    *input = inputBuffer;
    *output = outputBuffer;
}
	
//////////////////////////////////////////////////////////////////////
//
// Juce
//
//////////////////////////////////////////////////////////////////////

/**
 * Save the sampe rate so it can be inspected later for synchronization.
 * samplesPerBlockExpected I guess is to tell you to prepare for that many
 * if you need to organize internal buffers, though we're in the interrupt
 * so it's a little late now to be allocating memory.
 */
void JuceMobiusContainer::prepareToPlay (int samplesPerBlockExpected, double floatSampleRate)
{
    // AudioStream wants this as an int
    // in testing this comes in as 44100.0000
    // not sure under what conditions this would actually be a fractional value
    sampleRate = (int)floatSampleRate;
    expectedSamplesPerBlock = samplesPerBlockExpected;
}

/**
 * I think this should only be called during shutdown or after device configuration
 * changes.  For Mobius, it probably needs to perform a GlobalReset because
 * any synchronization calculations it was making may be wrong if the sample rate
 * changes on the next prepareToPlay call.
 */
void JuceMobiusContainer::releaseResources()
{
}

/**
 * Convert the Juce buffer storage convention in to the Mobius/PortAudio convention.
 * Unfortunately we don't call the stream handler directly, we just tell it something
 * arrived and it has to call back to getInterruptBuffers above so we can't just pass
 * these along, but it doesn't really matter since the format conversion requires
 * an intermediate model anyway.
 */
void JuceMobiusContainer::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // do NOT call clearActiveBufferRegion like the boilerplate code
    // from Projucer, it is unclear whether this just does output buffers
    // or if it also wipes the input buffers too
    // the output buffers will get propery initialized during deinterleaving

    // number of samples we're expected to consume and fill
    // save this for the handler callback
    nextBlockSamples = bufferToFill.numSamples;
    
    // the starting location within the Juce buffer to use
    // it seems like the buffer may be larger than what we're expected to
    // to process, this might be how they handle plugins that don't like variable block
    // sizes from the host, receive a large buffer from the host, then send it down
    // in smaller fixed size chunks, I've only seen startSample be zero, but support it
    int startSample = bufferToFill.startSample;

    // leave the result in our inputBuffer array
    interleaveInputBuffers(bufferToFill, inputBuffer);
    
    // clear the interleaved output buffer just in case the handler is bad
    clearInterleavedBuffer(nextBlockSamples, outputBuffer);

    // call the handler which will immediately call back to 
    // getInterruptFrames and getInterruptBuffers
    if (handler != nullptr) {
        handler->processAudioBuffers(this);
    }
    else {
        // inject a temporary test
        test(bufferToFill.numSamples);
    }
    
    // copy what was left in the in the interleaved output buffer back to
    // the Juce buffer, if we don't have a handler we'll copy the
    // empty output buffer left by clearInterleavedBuffer and clear
    // the channel buffers we didn't use
    deinterleaveOutputBuffers(bufferToFill, outputBuffer);

    // pray
}

/**
 * Wipe one of our interleaved buffers of content.
 * Only for the output buffer
 * Note that numSamples here is number of samples in one non-interleaved buffer
 * passed to getNextAudioBlock.  We need to multiply that by the number of channels
 * for the interleaved buffer.
 *
 * Our internal buffer will actually be larger than this (up to 4096 frames) but
 * just do what we need.
 */
void JuceMobiusContainer::clearInterleavedBuffer(int numSamples, float* buffer)
{
    // is it still fashionable to use memset?
    int totalSamples = numSamples * 2;
    memset(buffer, 0, (sizeof(float) * totalSamples));
}

/**
 * Convert a set of non-interleaved input buffers into an interleaved buffer.
 * Here we expect two channel buffers.  In theory there could be more but we'll ignore
 * them.  If there is only one, clear those samples so the engine doesn't
 * have to deal with it.
 *
 * The tutorial is kind of insane about how buffers are organized, so I'm capturing some
 * of it here:
 * 
 * It is important to know that the input and output buffers are not completely separate.
 * The same buffer is used for the input and output. You can test this by temporarily
 * commenting out all of the code in the getNextAudioBlock() function. If you then run
 * the application, the audio input will be passed directly to the output.
 * In the getNextAudioBlock() function, the number of channels in the AudioSampleBuffer
 * object within the bufferToFill struct may be larger than the number input channels,
 * the number of output channels, or both. It is important to access only the data that
 * refers to the number of input and output channels that you have requested, and that
 * are available. In particular, if you have more input channels than output channels you
 * must not modify the channels that should contain read-only data.
 *
 * Yeah, that sounds scary.
 *
 * In the getNextAudioBlock() function we obtain BigInteger objects that represent the list
 * of active input and output channels as a bitmask (this is similar to the std::bitset
 * class or using a std::vector<bool> object). In these BigInteger objects the channels
 * are represented by a 0 (inactive) or 1 (active) in the bits comprising the BigInteger value.
 *
 * Oh, do go on...
 *
 * To work out the maximum number of channels over which we need to iterate, we can inspect
 * the bits in the BigInteger objects to find the highest numbered bit. The maximum number
 * of channels will be one more than this.
 *
 * The code should be reasonably self-explanatory but here are a few highlights:
 *
 * [narrator: no, no it is not]
 *
 * [1]: We may have requested more output channels than input channels, so our app needs to
 * make a decision about what to do about these extra outputs. In this example we simply
 * repeat the input channels if we have more outputs than inputs. (To do this we use the
 * modulo operator to "wrap" our access to the input channels based on the number of input
 * channels available.) In other applications it may be more appropriate to output silence
 * for higher numbered channels where there are more output channels than input channels.
 * [2]: Individual input channels may be inactive so we output silence in this case too.
 *
 * Okay, let's see if I undersand this
 *
 * The getActiveFooChannels returns a big int like this
 *    0000101001
 * where each bit represents a channel and it will be set if it is active.  You
 * should only read from or write to channel buffers that are marked active.  In order
 * to properly silence output buffers you should iterate over all of them.  For input buffers
 * I guess you only need to consume what you need.  There can be a different number
 * of input and output channels.  To know how many you're allowed to iterate over
 * find the highest bit set.  Channels and bit numbers are zero based.
 * There is no guarantee that a pair of channels that are considered "stereo" will
 * be next to each other that is simply a convention followed by the person plugging
 * things in to the hardware and enabling channels in the device manager.
 *
 * In most "normal" usage you plug each stereo channel into adjacent hardware jacks so
 * you should usually see this:
 *      000000011
 * If they have chosen not to do that, we have to decide:
 *   1) use adjacent channels anyway and just consume/send silence if it is inactive
 *   2) arbitrarily find two active channels and merge them
 *
 * When you look at the AudioDeviceSelector component, you can pick different input
 * output devices.  There can be only one of each, unclear whether this is a Juce enforcement
 * or an OS enforcement.  You can also only use one "device type" at a time which appear
 * to be the device driver types: Windows Audio, ASIO, etc.
 *
 * In my case I'm using the Fireface.  For output selection the RME driver lists
 * a number of "devices" that represent physical ports arranged in stereo pairs.
 * "Speakers (RME Fireface UC)", "Analog 3+4", etc.  When you select the RME for either
 * input or output in the Active channels sections there is only one item "Output channel 1+2
 * and "Input channel 1+2".  I think because I asked the UI to group them, if left mono
 * there would be two items you could enable individually.  In theory other drivers might
 * provide larger clusters than two, pretty sure that's how it used to work, you just
 * selected "RME" and you got 8 or however many physical channels there were.  Maybe that's
 * a function of ASIO4All or maybe there is an RME driver setting to tweak that.
 *
 * In any case the driver gives you a list of devices to pick, and each device can have a
 * number of channels, and each channel can be selectively active or inactive.
 *
 * For the purposes of standalone Mobius, since we are in control over the device
 * configuration we can say you must activate a pair of channels for stereo and it
 * is recommended that they be adjacent.  But we don't have to depend on that, if you
 * wanted L going into jack 1 and R going into jack 5 who are we to judge.  But we're not
 * going to provide a UI that lets to pick which channels should be considered a stereo
 * pair.  This is enforced to a degree by making the selector UI group them in pairs, but
 * in cases where there is an odd number you still might end up with a "pair" that has only one
 * channel, I think.  Maybe it just doesn't show those.
 *
 * To allow for configuration flexibility, we'll take option 2 above.  Find the lowest numbered
 * pair of active channels and combine them into our stereo buffer.  If someone wanted
 * to say put all the Left channels on jacks 1-4 and all the Right channels on jacks
 * 5-8 they couldn't do that, but then they have larger problems in life.
 *
 * Anyway, this is where we could implement the old concept of "ports".  Find as many
 * pairs of active channels as you can and treat those as ports.  However, the RME driver
 * only shows 2 channels per "device" so we can't do that without changing something
 * in the driver.
 *
 * I picked the wrong week to give up drinking...
 *
 * Sweet jesus, bufferToFill.buffer->getReadPointer does not return a float*
 * it is templated and returns "something" you apparently aren't suposed to know
 * and the examples hide this by using "auto" variables to hide the type.  It may be float*
 * or I guess it may be double*.  The compiler complains if you try to assign it to a float*
 * with "C2440	'=': cannot convert from 'const Type *' to 'float *"
 * This means you can't easily iterate over them and save the pointers for later.
 * Instead we'll iterate and remember just the channel numbers, then call getReadPointer
 * in the next loop where we need to access the samples
 */
void JuceMobiusContainer::interleaveInputBuffers(const juce::AudioSourceChannelInfo& bufferToFill,
                                                float* resultBuffer)
{
    // to do the active channel folderol, we need to get to the AudioDeviceManager
    juce::AudioDeviceManager& deviceManager = Supervisor::Instance->getAudioDeviceManager();
    auto* device = deviceManager.getCurrentAudioDevice();
    auto activeInputChannels = device->getActiveInputChannels();
    auto activeOutputChannels = device->getActiveOutputChannels();
    auto maxInputChannels = activeInputChannels.getHighestBit() + 1;
    auto maxOutputChannels = activeOutputChannels.getHighestBit() + 1;

    // because we can't be sure we'll find 2 or any active channels, zero
    // out the result buffer so we don't have to clean up the mess after
    // the next loop
    clearInterleavedBuffer(bufferToFill.numSamples, resultBuffer);
    
    // look for the two lowest numbered active channels to serve as the source
    // for our stereo frames, when we find one, copy them as soon as we find them
    // so we can use the fucking auto
    int interleavedChannel = 0;
    for (auto channel = 0 ; channel < maxInputChannels ; channel++) {
            // this is a misnomer, it isn't the channels that are active
        // it's a bit vector of channels that could be active
        if (activeInputChannels[channel]) {
            // okay, here's one
            auto* buffer = bufferToFill.buffer->getReadPointer(channel, bufferToFill.startSample);
            for (int i = 0 ; i < bufferToFill.numSamples ; i++) {
                int frameOffset = i * 2;
                resultBuffer[frameOffset + interleavedChannel] = buffer[i];
            }
            interleavedChannel++;
            if (interleavedChannel >= 2)
              break;
        }
    }

    // our job was just to get the input samples, output comes later
    // after our handler is called
}

/**
 * Take the interleaved Mobius output buffer and spray that into active
 * Juce output channel buffers.
 *
 * Similar issues with output channels that represent stereo pairs.
 * In practice these will almost always be the first two but if not pick
 * the lowest two we can find.
 *
 * For any active output channels that we decide not to use, fill them
 * with zeros.  I think that is required because they're not cleared
 * by default.  Or we could put a jaunty tune in there.
 */
void JuceMobiusContainer::deinterleaveOutputBuffers(const juce::AudioSourceChannelInfo& bufferToFill,
                                                   float* sourceBuffer)
{
    // to do the active channel folderol, we need to get to the AudioDeviceManager
    // once this is working, can save some of this so we don't have to do it all again
    // but it should be fast enough
    juce::AudioDeviceManager& deviceManager = Supervisor::Instance->getAudioDeviceManager();
    auto* device = deviceManager.getCurrentAudioDevice();
    auto activeInputChannels = device->getActiveInputChannels();
    auto activeOutputChannels = device->getActiveOutputChannels();
    auto maxInputChannels = activeInputChannels.getHighestBit() + 1;
    auto maxOutputChannels = activeOutputChannels.getHighestBit() + 1;
    
    // first locate the two lowest numbered active channels to serve as the
    // destination for our stereo frames
    int interleavedChannel = 0;
    for (int channel = 0 ; channel < maxOutputChannels ; channel++) {
        if (activeOutputChannels[channel]) {
            auto* buffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);
            if (interleavedChannel < 2) {
                for (int i = 0 ; i < bufferToFill.numSamples ; i++) {
                    int frameOffset = i * 2;
                    buffer[i] = sourceBuffer[frameOffset + interleavedChannel];
                }
            }
            else {
                // an extra one, clear it
                for (int i = 0 ; i < bufferToFill.numSamples ; i++) {
                    buffer[i] = 0.0f;
                }
            }
            interleavedChannel++;
        }
    }
    
    // if you're like me you're tired, but it's a good kind of tired
}

//////////////////////////////////////////////////////////////////////
//
// Tests
//
//////////////////////////////////////////////////////////////////////

/**
 * If we don't have a handler we'll call down here and inject
 * something interesting for testing.
 *
 * Start by just copying the input to the output to verify
 * that interleaving works.
 */
void JuceMobiusContainer::test(int numSamples)
{
    // input buffer has been filled, copy them to the output buffer buffer

    int frameSamples = numSamples * 2;
    for (int i = 0 ; i < frameSamples ; i++) {
        outputBuffer[i] = inputBuffer[i];
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

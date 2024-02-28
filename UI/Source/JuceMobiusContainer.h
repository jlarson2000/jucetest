/**
 * Implementation of MobiusContainer that bridges the Juce
 * application environment.
 */

#pragma once

#include <JuceHeader.h>

#include "mobius/MobiusContainer.h"

/**
 * The maximum number of frames we'll allow in our intermediate interleaved
 * buffers.  Fixed so we can allocate them once and/or have them on the stack.
 *
 * Old comments:
 * This should as large as the higest expected ASIO buffer size.
 * 1024 is probably enough, but be safe.  
 * UPDATE: auval uses up to 4096, be consistent with
 *
 * This really should be dynamically allocated above the audio thread but
 * I don't know how we would know the expected size.  prepareToPlay passes
 * the size to expect but I don't know what thread that is in.  If a buffer
 * comes in bigger than this just bail.
 */
const int JuceAudioMaxFramesPerBuffer = 4096;

/**
 * Number of samples per frame.
 * We have always just supported 2.
 */
const int JuceAudioMaxChannels = 2;

/**
 * This then is the size of one interleaved input or output buffer we
 * need to allocate.  Under all circumstances the Juce buffer processing
 * will use the same size for both the input and output buffers.
 *
 * This will result in two buffers of 8k being allocated on the stack
 * if you do stack allocation.  Shouldn't be a problem these days but might
 * want to move these to the heap.
 *
 * Currently Supervisor has a JuceAudioInterface on the stack.
 */
const int JuceAudioMaxSamplesPerBuffer = JuceAudioMaxFramesPerBuffer * JuceAudioMaxChannels;


class JuceMobiusContainer : public MobiusContainer
{
  public:

    JuceMobiusContainer(class Supervisor* s);
    ~JuceMobiusContainer();

    // MainComponent/Supervisor will pass along the Juce notifications
        
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill);
    void releaseResources();

    // MobiusContainer

    void setAudioListener(class MobiusContainer::AudioListener* l);
    int getInputPorts();
    int getOutputPorts();
    int getSampleRate();
    int getInputLatency();
    int getOutputLatency();

    double getStreamTime();
    double getLastInterruptStreamTime();
    class AudioTime* getTime();
	long getInterruptFrames();
	void getInterruptBuffers(int inport, float** input, 
                                     int outport, float** output);

  private:

    class Supervisor* supervisor = nullptr;
    class MobiusContainer::AudioListener* listener = nullptr;

    // these are captured in prepareToPlay
    int sampleRate = 0;
    int expectedSamplesPerBlock = 0;

    // the number of samples we actually received
    int nextBlockSamples = 0;

    // interleaved sample buffers large enough to hold whatever comes
    // in from Juce.  Old code had a pair of these for each "port", we'll
    // start by assuming just one port.  
    float inputBuffer[JuceAudioMaxSamplesPerBuffer];
    float outputBuffer[JuceAudioMaxSamplesPerBuffer];
    
    void clearInterleavedBuffer(int numSamples, float* buffer);
    void interleaveInputBuffers(const juce::AudioSourceChannelInfo& bufferToFill, float* resultBuffer);
    void deinterleaveOutputBuffers(const juce::AudioSourceChannelInfo& bufferToFill, float* sourceBuffer);

    void test(int samples);

};

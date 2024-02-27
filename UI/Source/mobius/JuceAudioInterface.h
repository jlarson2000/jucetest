// implementation of AudioInterface and AudioStream that bridges Juce
// there is a static factory method defined in AudioInterface that
// could (should?) create one of these, who calls it?

#pragma once

#include "AudioInterface.h"
#include "JuceAudioInterface.h"

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


class JuceAudioInterface : public AudioInterface, public AudioStream
{
  public:

    JuceAudioInterface();
    ~JuceAudioInterface();

    // MainComponent/Supervisor will pass along the Juce notifications
        
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill);
    void releaseResources();

    // AudioInterface 

    // what is this supposed to do?
	void terminate();

    // all device management is stubbed out
	AudioDevice** getDevices() {return nullptr;};
	AudioDevice* getDevice(int id) {return nullptr;};
	AudioDevice* getDevice(const char* name, bool output) {return nullptr;};
	void printDevices() {};

    // this is the only interesting one
	AudioStream* getStream() {return this;};

    // AudioStream

    // around the circle we go
	class AudioInterface* getInterface() {return this;};

    // device management is stubbed
    bool setInputDevice(int id) {return false;};
	bool setInputDevice(const char* name) {return false;};
	AudioDevice* getInputDevice() {return nullptr;};
	bool setOutputDevice(int id) {return false;};
	bool setOutputDevice(const char* name) {return false;};
	AudioDevice* getOutputDevice() {return nullptr;};

    int getInputChannels();
    int getInputPorts();

	int getOutputChannels();
    int getOutputPorts();

    // can't set it now
	void setSampleRate(int i);
	int getSampleRate();

    // this is what Mobius calls to be told about buffers
	void setHandler(AudioHandler* h);

    // can't open or close under Mobius control
	bool open();
	void close();

	const char* getLastError();

    // PortAudiuo holdover, it's not a demand merely a "suggestion"
    // don't go to any trouble
	void setSuggestedLatencyMsec(int i);
    int getInputLatencyFrames();
    void setInputLatencyFrames(int frames);
    int getOutputLatencyFrames();
    void setOutputLatencyFrames(int frames);

	void printStatistics();

    // Stream time info, may be called outside the interrupt
    // to synchronize trigger events
    double getStreamTime();
    double getLastInterruptStreamTime();

	// these are called by the AudioHandler during an interrupt

	long getInterruptFrames();
	void getInterruptBuffers(int inport, float** input, 
                             int outport, float** output);
	
	AudioTime* getTime();

  private:

    AudioHandler* handler = nullptr;
    void test(int samples);
    
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
    void interleaveInputBuffers(juce::AudioSourceChannelInfo& bufferToFill, float* resultBuffer);
    void deinterleaveOutputBuffers(juce::AudioSourceChannelInfo& bufferToFill, float* sourceBuffer);


};

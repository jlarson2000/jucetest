/**
 * Formerly in the audio folder and defined a bunch of stuff
 * related to interfacing with audio devices when running standalone.
 * Almost none of that is still relevant, but AudioStream is used in
 * many places so that is stubbed out and will be implemented as a bridge
 * between the old and new worlds util we can retool the old world
 * to use MobiusContainer
 *
 */

#pragma once

/****************************************************************************
 *                                                                          *
 *   							  CONSTANTS                                 *
 *                                                                          *
 ****************************************************************************/

// some of these are defined elsewhere in Kernel, can migrate gradually

/**
 * The preferred number of frames in an audio interface buffer.
 * This is what we will request of PortAudio, and usually get, but
 * you should not assume this will be the case.  When using the VST
 * interface, the size is under control of the VST driver and it will
 * often be 512, but usually not higher.
 */
#define AUDIO_FRAMES_PER_BUFFER	(256)

/**
 * The maximum number of frames for an audio interface buffer.
 * This should as large as the higest expected ASIO buffer size.
 * 1024 is probably enough, but be safe.  
 * UPDATE: auval uses up to 4096, be consistent with 
 * MAX_HOST_BUFFER_FRAMES in HostInterface.h
 * !! don't like the duplication, move HostInterface over here?
 */
#define AUDIO_MAX_FRAMES_PER_BUFFER	(4096)

/**
 * The maximum number of channels we will have to deal with.
 * Since this is used for working buffer sizing, don't raise this until
 * we're ready to deal with surround channels everywhere.
 */
#define AUDIO_MAX_CHANNELS 2 


/**
 * The maximum size for an intermediate buffer used for transformation
 * of an audio interrupt buffer.  Used to pre-allocate buffers that
 * will always be large enough.
 */
#define AUDIO_MAX_SAMPLES_PER_BUFFER AUDIO_MAX_FRAMES_PER_BUFFER * AUDIO_MAX_CHANNELS

/**
 * Maximum number of ports (assumed to be stereo) we support
 * for a given audio device.
 */
#define AUDIO_MAX_PORTS 16

#define CD_SAMPLE_RATE 		(44100)

/**
 * Debugging kludge, set to disable catching stray exceptions in the
 * interrupt handler. Implementation dependent whether this works.
 */
extern bool AudioInterfaceCatchExceptions;


/****************************************************************************
 *                                                                          *
 *   							   HANDLER                                  *
 *                                                                          *
 ****************************************************************************/

/**
 * The interface of an object that may be registered with a Stream
 * to receive audio interrupts.  The handler is expected to call
 * back to the stream to retrieve the input and output buffers for
 * each of the ports supported by the stream.
 */
class AudioHandler {

  public:

	virtual void processAudioBuffers(class AudioStream* stream) = 0;

};

/****************************************************************************
 *                                                                          *
 *   								 TIME                                   *
 *                                                                          *
 ****************************************************************************/

/**
 * VST and AU streams can also include synchronization info.
 * I don't really like having this in AudioInterface, but the things
 * that need AudioTime are currently given only an AudioStream and we would
 * have to retool several interfaces so Mobius could get an AudioTime without
 * knowing that it is a plugin.
 *
 * Since the point of all this is to accurately sync with the audio stream, 
 * it feels best to  encapsulate it here.
 *
 * This is the same data in the VstTimeInfo, plus some analysis.
 */
class AudioTime {

  public:

	/**
	 * Host tempo.
	 */
	double tempo;

	/**
	 * The "beat position" of the current audio buffer.
     * 
	 * For VST hosts, this is VstTimeInfo.ppqPos.
	 * It starts at 0.0 and increments by a fraction according
	 * to the tempo.  When it crosses a beat boundary the integrer
     * part is incremented.
     *
     * For AU host the currentBeat returned by CallHostBeatAndTempo
     * works the same way.
	 */
	double beatPosition;

	/**
	 * True if the host transport is "playing".
	 */
	bool playing;

	/**
	 * True if there is a beat boundary in this buffer.
	 */
	bool beatBoundary;

	/**
	 * True if there is a bar boundary in this buffer.
	 */
	bool barBoundary;

	/**
	 * Frame offset to the beat/bar boundary in this buffer.
	 */
	long boundaryOffset;

	/**
	 * Current beat.
	 */
	int beat;

	/**
	 * Current bar.
	 */
	//int bar;

    /**
     * Number of beats in one bar.  If zero it is undefined, beat should
     * increment without wrapping and bar should stay zero.
     */
    int beatsPerBar;

    // TODO: also capture host time signture if we can
    // may need some flags to say if it is reliable

	void init() {
		tempo = 0.0;
		beatPosition = -1.0;
		playing = false;
		beatBoundary = false;
		barBoundary = false;
		boundaryOffset = 0;
		beat = 0;
//		bar = 0;
        beatsPerBar = 0;
	}

};

/****************************************************************************
 *                                                                          *
 *                                    PORT                                  *
 *                                                                          *
 ****************************************************************************/

/**
 * The channels in an AudioDevice can be arranged into ports.
 * Currently we require that ports always have 2 channels, eventuallyu
 * need a more flexible way to define ports.
 *
 * This serves both as a way to define the characteristics of a port
 * for the interface, and also some internal buffer interleaving utilities
 * for the engine.
 */
class AudioPort {

  public:

    AudioPort();
    ~AudioPort();

    void setNumber(int i);
    int getNumber();

    void setChannels(int i);
    int getChannels();

    void setFrameOffset(int i);

	void reset();
	float* extract(float* src, long frames, int channels);
	float* prepare(long frames);
	void transfer(float* dest, long frames, int channels);

  protected:

    /**
     * The number of this port.
     */
    int mNumber;

    /**
     * The number of channels in this port.
     * Currently this should always be 2.
     */
    int mChannels;

    /**
     * The offset within the the device buffer to the start
     * of this port's channels.  
     * Currently this should be port number * 2 since we only
     * have stereo ports.
     */
    int mFrameOffset;

	/**
	 * Set true once mBuffer has been prepared.
	 */
	bool mPrepared;

	/**
	 * The buffer with the extracted frames for one port.
	 */
	float* mBuffer;

};

/****************************************************************************
 *                                                                          *
 *   								STREAM                                  *
 *                                                                          *
 ****************************************************************************/

/**
 * An object representing one bi-directional audio stream.
 * These will be returned by AudioInterface.
 *
 * The stream parameters are normally set before you call open().
 * In theory they can be changed on the fly, but not all implementations
 * may support this.  
 *
 * The stream must be closed to release system resources.
 *
 * This is also used to represent a stream of IO buffers managed
 * by an AU or VST plugin host.  In that case, the device management
 * methods are ignored, it is only used for getInterruptFrames,
 * getInterruptBuffers, getTime, and getSampleRate.
 *
 */
class AudioStream {

  public:

	virtual ~AudioStream() {}

	virtual class AudioInterface* getInterface() = 0;

	//virtual bool setInputDevice(int id) = 0;
	//virtual bool setInputDevice(const char* name) = 0;
	//virtual AudioDevice* getInputDevice() = 0;

	//virtual bool setOutputDevice(int id) = 0;
	//virtual bool setOutputDevice(const char* name) = 0;
	//virtual AudioDevice* getOutputDevice() = 0;

	// NOTE: Currently assuming a stream may have several "ports"
	// each having 2 channels.
	// Need a more flexible port allocation interface.

	virtual int getInputChannels() = 0;
    virtual int getInputPorts() = 0;
    //virtual AudioPort* getInputPort(int p) = 0;

	virtual int getOutputChannels() = 0;
    virtual int getOutputPorts() = 0;
    //virtual AudioPort* getOutputPort(int p) = 0;

	virtual void setSampleRate(int i) = 0;
	virtual int getSampleRate() = 0;

	virtual void setHandler(AudioHandler* h) = 0;

	virtual bool open() = 0;
	virtual void close() = 0;

	virtual const char* getLastError() = 0;

	virtual void setSuggestedLatencyMsec(int i) = 0;
    virtual int getInputLatencyFrames() = 0;
    virtual void setInputLatencyFrames(int frames) = 0;
    virtual int getOutputLatencyFrames() = 0;
    virtual void setOutputLatencyFrames(int frames) = 0;

	virtual void printStatistics() = 0;

    // Stream time info, may be called outside the interrupt
    // to synchronize trigger events
    virtual double getStreamTime() = 0;
    virtual double getLastInterruptStreamTime() = 0;

	// these are called by the AudioHandler during an interrupt

	virtual long getInterruptFrames() = 0;
	virtual void getInterruptBuffers(int inport, float** input, 
									 int outport, float** output) = 0;
	
	virtual AudioTime* getTime() = 0;

};

/****************************************************************************
 *                                                                          *
 *   							  INTERFACE                                 *
 *                                                                          *
 ****************************************************************************/

/**
 * An interface that provides access to the audio devices.
 * A simplification of portaudio.  Added to ease the migration
 * from v18 to v19 but it's nice to have the dependencies
 * encapsulated anyway.
 */
class AudioInterface {

  public:

	/**
	 * Have to have one of these to get the subclass destructor to run.
	 */
	virtual ~AudioInterface(){}

	/**
	 * Create a bi-directional audio stream.
	 */
	virtual AudioStream* getStream() = 0;

};

//////////////////////////////////////////////////////////////////////
//
// Stubs
//
//////////////////////////////////////////////////////////////////////


class StubAudioStream : public AudioStream
{
  public:

	StubAudioStream(AudioInterface* ai) {
        mInterface = ai;
    }

	~StubAudioStream() {}

	//void setInterface(AudioInterface* ai);
	AudioInterface* getInterface() {
        return mInterface;
    }

	int getInputChannels() {
        return 2;
    }
    
    int getInputPorts() {
        return 1;
    }
    
	int getOutputChannels() {
        return 2;
    }
    
    int getOutputPorts() {
        return 1;
    }

	void setSampleRate(int i) {
    }
    
	int getSampleRate() {
        return CD_SAMPLE_RATE;
    }

	void setHandler(AudioHandler* h) {
        mHandler = h;
    }
    
	AudioHandler* getHandler() {
        return mHandler;
    }
    
	bool open() {}
	void close() {}

	const char* getLastError() {
        return nullptr;
    }
    
	void setSuggestedLatencyMsec(int msec) {
    }
    
    int getInputLatencyFrames() {
        return 1024;
    }
    
    void setInputLatencyFrames(int frames) {
    }
    
    int getOutputLatencyFrames() {
        return 1024;
    }
    
    void setOutputLatencyFrames(int frames) {
    }
    
	void printStatistics() {
    }

    double getStreamTime() {
        return 0.0f;
    }
        
    double getLastInterruptStreamTime() {
        return 0.0f;
    }
    
	long getInterruptFrames() {
        return 0;
    }
    
	void getInterruptBuffers(int inport, float** input, 
                             int outport, float** output) {
    }
	
	AudioTime* getTime() {
        return nullptr;
    }

  private:
    AudioInterface* mInterface;
    AudioHandler* mHandler;
    
};

class StubAudioInterface : public AudioInterface
{
  public:
    
    StubAudioInterface() {
    }
    
    ~StubAudioInterface() {
    }

    AudioStream* getStream() {
        return &mStream;
    }

  private:
    
    StubAudioStream mStream {this};
};


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

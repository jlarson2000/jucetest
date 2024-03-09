/**
 * Interface of an object that runs the Mobius engine
 * and provides connections to the outside world.
 *
 * This is a redesign of what used to be a combination of
 * MobusContext, AudioInterface, AudioStream, MidiInterface.
 * and to some extent MobusPlugin.
 *
 * The main thing this is provides is the forwarding of
 * audio and MIDI data sent by hardware devices, either though
 * a plugin host, or through the standalone app which uses Juce
 * to manage audio and midi devices.
 *
 * This data is not necessarily under direct control of an end user,
 * interacting with a UI, and for audio can be large and rapid.
 *
 * Most interaction with the engine happens in the UI which builds
 * UIActions and passes them to MobiusInterface.
 *
 * Audio data is different in that it can be large and continuous so
 * we use a "listener" style of interface to receive that.
 *
 * MIDI is kind of in between, it isn't as large as audio, but in the
 * case of clock pulses it can be fast so we avoid the overhead of
 * UIAction processing and provide a more direct access to the MIDI
 * data stream.
 *
 * The old code had a lot of support for the direct management of audio
 * and midi devices through this interface, this has been removed and old
 * code rewritten to either stop doing that, or get what it needs from
 * MobiusContainer instead.
 *
 * Now that we have this, some of things currently pushed through MobiusInterface
 * such as MobiusConfig could be pulled out of MobiusContainer.  Think
 * more about this as it evolves.
 *
 */

#pragma once

class MobiusContainer
{
  public:
    
    /**
     * Interface of an object to be notified when audio buffers are available.
     * The listener is expected to immedially call back to the container
     * functionos to obtaain the audio buffers and other information about
     * the audio stream.
     */
    class AudioListener {
      public:
        virtual void containerAudioAvailable(MobiusContainer*cont) = 0;
    };

    // todo: similar listener for MIDI events or push them through
    // the MobiusInterface?
    // need to figure out which thread MIDI comes in on

    /**
     * Tell the container where to send notifications of incomming audio.
     */
    virtual void setAudioListener(AudioListener* l) = 0;

    // General information provided by the container

    // this is used to monitor run times of internal components,
    // it is not expected to have any particular base value, just that
    // it always increments with an accurate milliseond interval
    virtual int getMillisecondCounter() = 0;

    // used in rare cases to synchronously delay for a short time
    virtual void sleep(int millis) = 0;
    
    // General non-real time informaton about the connected audio stream
    // these can be called at any time from any thread
    
    // We have organized audio data into one or more "ports" containing
    // two stereo audio channels.  The number of ports available is used
    // by ParameterTrack to get the high values when this used to be connected
    // to the UI to configure the selection of input/output ports in the Setup.
    // We no longer to Setup editing from the Parameter model so these can
    // probably be removed.
    virtual int getInputPorts() = 0;
    virtual int getOutputPorts() = 0;


    
    virtual int getSampleRate() = 0;
    virtual int getInputLatency() = 0;
    virtual int getOutputLatency() = 0;

    // Stream time info, may be called outside the interrupt
    // to synchronize trigger events
    // unclear why I thought these needed to be doubles, probaby
    // not necessary
    virtual double getStreamTime() = 0;
    virtual double getLastInterruptStreamTime() = 0;

    // forget what this was, but it looks importnt
    virtual class AudioTime* getAudioTime() = 0;

    // callbacks that are used by AudioListener to get
    // information about the last block of audio
	virtual long getInterruptFrames() = 0;
	virtual void getInterruptBuffers(int inport, float** input, 
                                     int outport, float** output) = 0;

};

/**
 * VST and AU streams can also include synchronization info.
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

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

	


    

    

/**
 * A basic multi-track audio recorder and player.
 *
 * This will be created by the Kernel and live for the duration of the kernel.
 * It is given a MobiusContainer during initialization where it will
 * register itself as a listener for audio events.
 * 
 * This is then the primary entry point between Juce and the engine.
 */

#pragma once

#include <stdio.h>

#include "../util/Util.h"

#include "Audio.h"
#include "MobiusContainer.h"

/****************************************************************************
 *                                                                          *
 *   							  CONSTANTS                                 *
 *                                                                          *
 ****************************************************************************/

/**
 * Default amplitude threshold to trigger recording.
 */
#define DEFAULT_RECORD_THRESHOLD  0.01f

/**
 * Maximum number of audio tracks that may be installed.
 */
#define MAX_RECORDER_TRACKS 64

/**
 * The number of latency tests to run during calibration.
 */
#define CALIBRATION_TEST_COUNT 10

/**
 * Approxomate number of frames to measure the noise floor
 * during calibration.  
 */
#define CALIBRATION_NOISE_FRAMES 10000

/**
 * The length in frames of the calibration signal.
 */
#define CALIBRATION_PING_FRAMES 1

/**
 * The amplitude of the calibration signal.
 */
#define CALIBRATION_PING_AMPLITUDE 0.7f

/**
 * The minimum level we require in the ping echo.
 * Some other code I found measured the noise floor
 * and then just looked for something that was 2x the
 * floor, but this proved to be unreliable. With
 * a floor of 0.000061 I would see very regular
 * blips of 0.000153 to 0.000183 would would be
 * falsely treated as echos.  This appears
 * to be caused by crosstalk/channel bleed
 * in the Mackie, when the Lynx output is
 * routed to the Alt bus and monitored, a small
 * amount of the signal still makes it to the main outs.
 *
 * The echo signal seems to be getting a little bit of 
 * gain and has A/D artifacts resulting in some ramp up/down
 * samples.  Not sure if we should measure the echo frame
 * from the start of the ramp up or till it reaches its maximum?
 */
#define CALIBRATION_ECHO_AMPLITUDE 0.01f

/**
 * The default latency for LynxOne Analog In/Out in milliseconds.
 * Measured using the WMME drivers.
 * This corresponds to around 212 milliseconds.
 *
 * The first time the interrupt is called the outTime is 9216 which
 * is supiciously close to this number.  
 */
#define DEFAULT_LATENCY_FRAMES 9369

/**
 * Maximum number of Audio Stream ports the recorder will support.
 */
#define MAX_OUTPUT_PORTS 8

/****************************************************************************
 *                                                                          *
 *   							RECORDER TRACK                              *
 *                                                                          *
 ****************************************************************************/

class RecorderTrack {

	friend class Recorder;

  public:

	RecorderTrack();
	RecorderTrack(Audio* a);
	virtual ~RecorderTrack();

	void setRecorder(class Recorder *r);
	void setRecording(bool b);
	void initAudio();
	void dump();
	void reset();
	virtual void inputBufferModified(float* f);
	
	Audio* getAudio() {
		return mAudio;
	}

	bool isFinished() {
		return mFinished;
	}

	void setFinished(bool b) {
		mFinished = b;
	}

	bool isRecording() {
		return mRecording;
	}

    void setMute(bool b) {
        mMute = b;
    }

    virtual bool isMute() {
        return mMute;
    }

	void setRecordThreshold(float f) {
		mThreshold = f;
	}

	float getRecordThreshold() {
		return mThreshold;
	}

	void setInputPort(int i) {
		mInputPort = i;
	}

	int getInputPort() {
		return mInputPort;
	}

	void setOutputPort(int i) {
		mOutputPort = i;
	}

	int getOutputPort() {
		return mOutputPort;
	}

	void setSelected(bool b) {
		mSelected = b;
	}

	bool isSelected() {
		return mSelected;
	}

    bool isProcessed() {
        return mProcessed;
    }

    void setProcessed(bool b) {
        mProcessed = b;
    }

  protected:

    // indicates that this track should be processed before others
    virtual bool isPriority();

	// to be called only by Recorder
	virtual void processBuffers(class MobiusContainer* stream, 
								float* input, float* output, 
								long bufferFrames, long frameOffset);

	// expected to be overloaded 
	virtual void addAudio(float* src, long newFrames, long startFrame);
	virtual void getAudio(float* out, long frames, long frameOffset);

	void initRecorderTrack();

	class Recorder *mRecorder;	// pointer back to recorder
	Audio* mAudio;			    // Audio object we're playing
	bool mFinished;			    // true when we're done
	bool mRecording;            // true when recording
    bool mRecordStarted;
	bool mMute;                 // true when muted
	float mThreshold;		    // record start threshold
	
	/**
	 * True when "selected".  Down here the selected track
	 * is the one that is used for input level metering.
	 * In Mobius, this corresponds to the selected track
	 * in the GUI.  Hmm, would be better to just do metering
	 * for each track/port combination and have them all available?
	 */
	bool mSelected;

    /**
     * Transient flag set on every interrupt to keep track of the
     * tracks that have been processed.  Necessary because tracks
     * are not processed in order, and processing one track can
     * have an effect on another.
     */
    bool mProcessed;

	// 
	// Some AudioInterfaces support more than one port.
	// Port zero is the default port, but a track may be assigned
	// to a higher port.	

	int mInputPort;
	int mOutputPort;

};

/**
 * Special track that emits a constant square wave.
 */
class SignalTrack : public RecorderTrack {

  public:

    SignalTrack();
    virtual ~SignalTrack();

    void fillOutputBuffer(float *out, long frames, long frameOffset);

};

/****************************************************************************
 *                                                                          *
 *   							   MONITOR                                  *
 *                                                                          *
 ****************************************************************************/

/**
 * Interface of an object that may be installed to monitor audio interrupts.
 * This is similar to a RecorderTrack but doesn't usually affect the output 
 * buffer.  It is intended to encapsulate code that needs to perform
 * operations on the RecorderTracks prior to the Recorder calling them
 * to process the audio buffers.  It will be called exactly once within the
 * interrupt handler before the RecorderTracks are processed.
 *
 * UPDATE: This was how OG mobius inserted prepare/finalize code around the
 * processing of audio blocks by the Tracks.  It was obscure and not used
 * any more.
 * 
 */
class RecorderMonitor {

  public:

	virtual void recorderMonitorEnter(MobiusContainer* stream) = 0;
	virtual void recorderMonitorExit(MobiusContainer* stream) = 0;

};

/****************************************************************************
 *                                                                          *
 *   						 LATENCY CALIBRATION                            *
 *                                                                          *
 ****************************************************************************/

class RecorderCalibrationResult {

  public:

	RecorderCalibrationResult() {
		timeout = false;
		noiseFloor = 0.0;
		latency = 0;
	}

	~RecorderCalibrationResult() {
	}

	bool timeout;
	float noiseFloor;
	int latency;
};

/****************************************************************************
 *                                                                          *
 *   							   RECORDER                                 *
 *                                                                          *
 ****************************************************************************/

/**
 * Not registered as a listener but MobiusKernel is and will
 * forward the notice along when it is done with it.
 */
class Recorder : public MobiusContainer::AudioListener {

  public:

	Recorder();
	~Recorder();
    
    void initialize(class MobiusContainer* cont, class AudioPool* pool);
	void shutdown();

    // MobiusContainer::AudioListener via MobiusKernel
    void containerAudioAvailable(MobiusContainer* cont);

    // Configuration

	void setAutoStop(bool b);
	void setEcho(bool b);
	void setMonitor(RecorderMonitor* m);

    // container access for internal components
    class MobiusContainer* getStream();
    class AudioPool* getAudioPool();

    // Status

    int getInputLevel();
	long getFrame();

	// Track management

    bool add(RecorderTrack* track);
    RecorderTrack* add(Audio* audio);
    bool remove(RecorderTrack* track);
    bool remove(Audio* audio);
	int getTrackCount();
	RecorderTrack* getTrack(int i);
	void select(RecorderTrack* track);
    bool replace(RecorderTrack* old, RecorderTrack* neu);
    
	// Transport

	void setFrame(long f);
    void setTime(int seconds);
	void start();
	void stop();
	bool isRunning();
	void awaitOutput();

    // Special operations

	RecorderCalibrationResult* calibrate();
	void inputBufferModified(RecorderTrack* track, float* buffer);

	// AudioHandler interface
	void processAudioBuffers(MobiusContainer* stream);

 private:

    bool checkAudio(Audio* audio);
	bool removeTrack(int n);
	void processTracks(MobiusContainer* stream);
	void calibrateInterrupt(float *input, float *output, long frames);

    class MobiusContainer* mContainer;
    class AudioPool* mAudioPool;

	RecorderMonitor* mMonitor;

	int mLatency;			// latency correction in milliseconds

	long mFrame;			// input frame counter
	long mInitialFrame;		// first frame reported by PortAudio

	// tracks being played
	RecorderTrack* mTracks[MAX_RECORDER_TRACKS];
	int mTrackCount;		// number of tracks installed

	bool mRunning;			// true when we're running
	bool mAutoStop;			// true to enable auto-stop
	bool mInInterrupt;
	bool mEcho;             // true to echo input to output

	Audio* mCalibrationInput;
	bool mCalibrating;
	float mNoiseAmplitude;
	long mPingFrame;
	int mLatencyTest;
	int mLatencyFrames[CALIBRATION_TEST_COUNT];

	long mLastInterruptTime;

};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

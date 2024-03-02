/**
 * outstanding issues:
 * 
 * SamplePlayer wants to receive notifications when the input/output
 * latencies change
 *
 * The notion of SampleTrigger might not be necessary any more,
 * comments inticate it was at least partially replaced by Actions
 *
 * SampleTrack::updateConfiguration
 *       void updateConfiguration(class MobiusConfig* config);
 *   what does it do?
 *     
 *
 * ---
 * 
 * A RecorderTrack extension that allows the playback of
 * read-only fragments of audio.
 *
 * This is built from a set of sample files defined in
 * the SampleConfig object.
 *
 * There are two execution contexts for the code in these
 * classes and I don't like it, but just get it working and
 * refine it later.
 *
 * The construction of the objects is always done in the UI
 * thread by the MobiusShell where memory allocation is allowed.
 * Once constructed the entire SampleTrack is passed to kernel
 * through a KernelMessage.
 *
 * The Kernel then installs the track and starts using it.
 * Most of the code in these classes is related to actually
 * using it, not building it.
 *
 * Loading the sample data from .wav files has been moved to
 * SampleReader and is expected to be done by the UI.  When
 * SampleConfig is passed to MobiusInterface it is expected to
 * contain loaded buffers of sample data.
 */

#pragma once

//#include <stdio.h>

#include "../model/SampleConfig.h"
#include "Recorder.h"

//////////////////////////////////////////////////////////////////////
//
// SampleTrigger
// part of the SamplePlayer model
//
//////////////////////////////////////////////////////////////////////

#define MAX_TRIGGERS 8

/**
 * Helper struct to represent one sample trigger event.
 * Each SamplePlayer maintains an array of these which are filled
 * by the ui and/or MIDI thread, and consumed by the audio thread.
 * To avoid a critical section, there are two indexes into the array.
 * The "head" index is the index of the first element that needs
 * to be processed by the audio thread.  The "tail" index is the
 * index of the next element available to be filled by the ui interrupt.
 * When the head and tail indexes are the same there is nothing in the
 * queue.  Only the audio thread advances the head index, only the ui
 * thread advances the tail index.  If the tail indexes advances to
 * the head index, there is an overflow.
 *
 * We don't handle trigger overflow gracefully, but this could only
 * happen if you were triggering more rapidly than audio interrupt
 * interval.  In practice, humans couldn't do this, though another
 * machine feeding us MIDI triggers could cause this.
 *
 * UPDATE: Sample triggering is now handled by the Action model
 * so triggers will always be done inside the interrupt, we don't
 * need the ring buffer.
 */
typedef struct {

    // true if this is a down transition
    bool down;

} SampleTrigger;

//////////////////////////////////////////////////////////////////////
//
// SamplePlayer
//
//////////////////////////////////////////////////////////////////////

/**
 * Represents one loaded sample that can be played by SampleTrack.
 *
 * old comment:
 * Might be interesting to give this capabilities like Segemnt
 * or Layer so we could dynamically define samples from loop material.
 */
class SamplePlayer
{
    friend class SampleCursor;

  public:

	SamplePlayer(class AudioPool* pool, Sample* s);
	~SamplePlayer();

    // !! revisit this
    // supposed to respond to reconfiguration of the audio device
    // while in the kernel, doesn't happen yet
    void updateConfiguration(int inputLatency, int outputLatency);

	void setNext(SamplePlayer* sp);
	SamplePlayer* getNext();

    // filename saved only for different detection
    const char* getFilename();

	void setAudio(Audio* a);
	Audio* getAudio();
	long getFrames();

	void setSustain(bool b);
	bool isSustain();

	void setLoop(bool b);
	bool isLoop();

    void setConcurrent(bool b);
    bool isConcurrent();
    
	void trigger(bool down);
	void play(float* inbuf, float* outbuf, long frames);

  protected:

    //
    // Configuration caches.
    // I don't really like having these here but I don't want to 
    // introduce a dependency on MobiusConfig or MobiusContainer at this level.
    // Although these are only used by SampleCursor, they're maintained here to 
    // make them easier to update.
    //
    
	/**
	 * Number of frames to perform a gradual fade out when ending
	 * the playback early.  Supposed to be synchronized with
	 * the MobiusConfig, but could be independent.
	 */
	long mFadeFrames;

    /**
     * Number of frames of input latency, taken from the audio stream (MobiusContainer)
     */
    long mInputLatency;

    /**
     * Number of frames of output latency.
     */
    long mOutputLatency;

  private:
	
	void init();
    class SampleCursor* newCursor();
    void freeCursor(class SampleCursor* c);

	SamplePlayer* mNext;
	Audio* mAudio;

	// flags copied from the Sample
    char* mFilename;
	bool mSustain;
	bool mLoop;
    bool mConcurrent;

    /**
     * A queue of trigger events, filled by the ui thread and
     * consumed by the audio thread.
     * jsl - comments say this is no longer used, really?
     */
    SampleTrigger mTriggers[MAX_TRIGGERS];

    int mTriggerHead;
    int mTriggerTail;

    /**
     * As the sample is triggered, we will active one or more 
     * SampleCursors.  This is the list of active cursors.
     */
    class SampleCursor* mCursors;

    class SampleCursor* mCursorPool;

    /**
     * Transient runtime trigger state to detect keyboard autorepeat.
     * This may conflict with MIDI triggering!
     */
    bool mDown;

};

//////////////////////////////////////////////////////////////////////
//
// SampleCursor
//
//////////////////////////////////////////////////////////////////////

/**
 * Encapsulates the state of one trigger of a SamplePlayer.
 * A SamplePlayer may activate more than one of these if the sample
 * is triggered again before the last one, and I assume if concurrency
 * is alloed.
 */
class SampleCursor
{
    friend class SamplePlayer;

  public:
    
    SampleCursor();
    SampleCursor(SamplePlayer* s);
    ~SampleCursor();

    SampleCursor* getNext();
    void setNext(SampleCursor* next);

    void play(float* inbuf, float* outbuf, long frames);
    void play(float* outbuf, long frames);

    void stop();
    bool isStopping();
    bool isStopped();

  protected:

    // for SamplePlayer
    void setSample(class SamplePlayer* s);

  private:

    void init();
	void stop(long maxFrames);

    SampleCursor* mNext;
    SampleCursor* mRecord;
    SamplePlayer* mSample;
	AudioCursor* mAudioCursor;

    bool mStop;
    bool mStopped;
    long mFrame;

	/**
	 * When non-zero, the number of frames to play, which may
     * be less than the number of available frames.
	 * This is used when a sustained sample is ended prematurely.  
	 * We set up a fade out and continue past the trigger frame 
	 * to this frame.  Note that this is a frame counter, 
     * not the offset to the last frame.  It will be one beyond
     * the last frame that is to be played.
	 */
	long mMaxFrames;

};

//////////////////////////////////////////////////////////////////////
//
// SampleTrack
//
//////////////////////////////////////////////////////////////////////

/**
 * The maximum number of samples that SampleTrack can manage.
 */
#define MAX_SAMPLES 8

/**
 * Makes a collection of SamplePlayers available for realtime playback
 * through the Recorder.
 */
class SampleTrack : public RecorderTrack {

  public:

    // These are constructed in the UI thread from a SampleConfig
    // and can pull information from the MobiusConfig and the container
	SampleTrack(class AudioPool* pool, class SampleConfig* samples);
	~SampleTrack();

    // return true if the contents of this track are different
    // than what is defined in the SampleConfig
    // this was an optimization when we didn't have a more granular
    // way to update the sub-parts of the MobiusConfig, should
    // no longer be necessary and could have been done by the UI
    bool isDifference(class SampleConfig* samples);

	bool isPriority();
    int getSampleCount();

    // todo: unclear what this does
    void updateConfiguration(class MobiusConfig* config);

	/**
	 * Triggering by internal sample index.
	 * TODO: Trigger by MIDI note with velocity!
	 */
    void trigger(int index, bool down);
	long getLastSampleFrames();

	/**
	 * Trigger a sustained sample.
	 * Only for use by Mobius in response to function handlers.
	 */
    void trigger(MobiusContainer* stream, int index, bool down);


    void prepareForInterrupt();
	void processBuffers(class MobiusContainer* stream,
						float* inbuf, float *outbuf, long frames, 
						long frameOffset);

  private:
	
	void init();

	// class MobiusShell* mMobius;
	SamplePlayer* mPlayerList;
	SamplePlayer* mPlayers[MAX_SAMPLES];
	int mSampleCount;
	int mLastSample;
	bool mTrackProcessed;

};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

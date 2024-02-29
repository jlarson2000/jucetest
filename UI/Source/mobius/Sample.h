/**
 * Classes related to shell/kernel management of samples
 * defined in the SampleConfig.  Heavily modified from the original..
 *
 * Loading the sample data from .wav files has been moved to
 * SampleReader and is expected to be done by the UI.  When
 * SampleConfig is passed to MobiusInterface it is expected to
 * contain loaded buffers of sample data.
 *
 * MobiusShell first constructs a SamplePack containing
 * SamplePlayer objects for each Sample.
 *
 * The SamplePack is then passed through the shell/kernel boundary.
 *
 * The Kernel places the list of SamplePlayers into the special
 * SampleTrack which is installed in the Recorder.
 *
 * NOTE:
 *
 * There is not a clean separation of shell/kernel code in here.
 * Shell needs only to build the SamplePlayer and pass it down,
 * the majority of what that class does is intended to be run
 * only within the kernel.
 *
 * What The shell mostly does is convert the unadorned float*
 * buffers from the Sample the UI passed in, into pooled Audio
 * objects for use within the kernel.  But the number of
 * SamplePlayers needed will vary based on the number of samples
 * so those have to be allocated and initialized before passing
 * them to the kernel.
 */

#pragma once

//#include <stdio.h>

#include "Recorder.h"

//////////////////////////////////////////////////////////////////////
//
// SamplePack
//
//////////////////////////////////////////////////////////////////////

/**
 * A temporary structure used to pass a list of SamplePlayers
 * from the UI thread that has been editing samples into the 
 * audio interrupt handler.  A SamplePack may be empty which
 * indicates to the kernel that current sample state should be discarded.
 */
class SamplePack {
  public:

    SamplePack();
    SamplePack(class AudioPool* pool, class SampleConfig* samples);
    ~SamplePack();

    class SamplePlayer* getSamples();
    class SamplePlayer* stealSamples();

  private:
    
    class SamplePlayer* mSamples;
};

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
 * These are built from the SampleConfig in the MobiusConfig and do not
 * retain any references to it.  A list of these is phased into the
 * kernel with a SamplePack object.
 *
 * old comment:
 * Might be interesting to give this capabilities like Segemnt
 * or Layer so we could dynamically define samples from loop material.
 * 
 */
class SamplePlayer
{
    friend class SampleCursor;

  public:

	SamplePlayer(class AudioPool* pool, Sample* s);
	~SamplePlayer();

    // !! revisit this
    // supposed to respond to reconfiguration of the audio device
    // while in the kernel, can't happen yet
    void updateConfiguration(int inputLatency, int outputLatency);

	void setNext(SamplePlayer* sp);
	SamplePlayer* getNext();

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
    // introduce a dependency on Mobius at this level.  Although these
    // are only used by SampleCursor, they're maintained here to 
    // make them easier to update.
    // jsl - not sure what that means
    //

	/**
	 * Number of frames to perform a gradual fade out when ending
	 * the playback early.  Supposed to be synchronized with
	 * the MobiusConfig, but could be independent.
	 */
	long mFadeFrames;

    /**
     * Number of frames of input latency.
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
    char* mFilename;
	Audio* mAudio;

	// flags copied from the Sample
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

    // should not be needing a Mobius handle
	//SampleTrack(class Mobius* mob);
	SampleTrack();
	~SampleTrack();

    // no longer need difference detction at this level, should
    // have been done by the UI
    bool isDifference(class SampleConfig* samples);

	bool isPriority();
    int getSampleCount();
	void setSamples(class SamplePack* pack);

    // todo: unclear what this does
    void updateConfiguration(class MobiusConfig* config);

	/**
	 * Triggering by internal sample index.
	 * TODO: Trigger by MIDI note with velocity!
	 */
	void trigger(int index);
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

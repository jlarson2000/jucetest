//
// issues
// 
// The hacky inputBufferModified call that SampleTrack was awful
// and removed, revisit when we get to unit test sample injection
//
// calibrate wants to write a .wav file and Audio no longer supports write()
// either add it back or do calibration a different way
//

/**
 * A basic multi-track audio recorder and player.
 *
 * This is mostly the same as it used to be with significant retooling
 * to replace AudioInterface, AudioStream, and MidiInterface with
 * MobiusContainer.
 *
 * There will be references in the code to "stream" which used to be
 * an AudioStream, but is now a MobiusContainer.
 *
 * This will be created by the Kernel and live for the duration of the kernel.
 * It is given a MobiusContainer during initialization where it will
 * register itself as a listener for audio events.
 * 
 * This is then the primary entry point between Juce and the engine.
 * The kernel itself doesn't do much beyond managing configuration and
 * communication with the shell.
 *
 * This could use some redesign but it's still a relatively simple and
 * useful layer between the container and the looping tracks where
 * all the complexity lies.  It is more general than I ever used it for
 * the main thing it provides beyond a container for the loopers
 * is the SampleTrack which is how we inject pre-loaded samples into the
 * audio stream.
 *
 * NOTE: There is memory management in here, but it should only be happening
 * during initialization outside of the audio thread so it's safe.
 * Need more work to make this more flexible, but it has never done
 * much within the engine and doesn't need to be redesigned right now.
 *
 */

//#include <stdio.h>
//#include <memory.h>

#include "../util/Util.h"
#include "../util/Trace.h"

#include "AudioPool.h"
#include "Audio.h"
#include "SampleTrack.h"

#include "Recorder.h"

/**
 * Emit warnings if we take too long between interrupts.
 *
 * old comments:
 * This is usually good to have on but I started getting it
 * frequently after upgrading to the monster graphics card
 * on my Windows dev machine.  Need to figure out what that is doing,
 * but disable for now so we can debug things.
 */
static bool TraceInterruptTime = false;

//////////////////////////////////////////////////////////////////////
//
// MobiusContainer::AudioListener
//
//////////////////////////////////////////////////////////////////////

/**
 * This is where the party starts.
 * After registering ouselves with MobiusContainer as the AudioListener
 * we'll start receiving callbacks as audio blocks come in.
 * The container provides model transformation between Juce buffers
 * and the old model we expect.
 *
 * The container is passed, which we don't need since we saved
 * a pointer during initialization.  Call back to the container to
 * get the buffers and other information about the stream.
 *
 * The container organizes audio data into "ports" which are collections
 * of mono channels, either input or output.  We have only ever supported
 * ports with two stereo channels.  The MobiusConfig/TrackSetup can
 * set which ports tracks receive on and send to.
 *
 * The acutal number of channels currently configured in the hardware may not
 * match that so if we ask for port data that doesn't exist the container
 * is expected to return something empty.  We should probably allow it
 * to return nullptr to indiciate port misconfiguration.  At the moment,
 * we're only supporting a single port, which will always be there.
 * We'll ask for whatever port happens to be configured, but the container
 * may not give us that one.
 */
void Recorder::containerAudioAvailable(MobiusContainer* cont)
{
	if (mInInterrupt) 
	  Trace(1, "Recorder::interrupt reentry!\n");
	mInInterrupt = true;

	// long start = ((mMidi != NULL) ? mMidi->getMilliseconds() : 0);
    long start = cont->getMillisecondCounter();
    
	if (TraceInterruptTime && mLastInterruptTime > 0) {
		long delta = start - mLastInterruptTime;
		if (delta > 5)
		  Trace(2, "%ld msec between audio interrupts\n", delta);
	}
	mLastInterruptTime = start;

    // I'm forgetting what RecorderMonitor was for
	long frames = cont->getInterruptFrames();
	if (mMonitor != NULL)
	  mMonitor->recorderMonitorEnter(cont);

    // the audio stream is live all the time, the running flag
    // determines whether we actually do anything

    if (mRunning) {
        if (mCalibrating) {
			// calibration only on the first port
			float* input = NULL;
			float* output = NULL;
			cont->getInterruptBuffers(0, &input, 0, &output);
			calibrateInterrupt(input, output, frames);
		}
        else
		  processTracks(cont);

    }

    if (TraceInterruptTime) {
		// long end = ((mMidi != NULL) ? mMidi->getMilliseconds() : 0);
        long end = cont->getMillisecondCounter();
		long elapsed = end - start;
		if (elapsed > 1) {
			// happens commonly in debugging so make it level 2, 
			// though in production should be 1
			Trace(2, "!!! %ld milliseconds to process audio interrupt\n",
				  elapsed);
		}
	}

	if (mMonitor != NULL)
	  mMonitor->recorderMonitorExit(cont);

	mFrame += frames;
	mInInterrupt = false;
}

/**
 * To assist with brother sync, we will allow tracks to say
 * they are "priority tracks" and will be processed before the
 * non-priority tracks.   This ensures that sync events for the master
 * sync track will be detected before the empty tracks waiting for
 * sync events.
 *
 * Note that we can't trust the isPriority value to be the same
 * for both passes, since processing one track can result modifications
 * to other tracks (via scripts for example).  So have to keep
 * a processed flag of our own.
 */
void Recorder::processTracks(MobiusContainer* stream)
{
	RecorderTrack* selected = NULL;
	bool allFinished = true;
	long frames = stream->getInterruptFrames();
    int i;

    // process all priority tracks first
	for (i = 0 ; i < mTrackCount ; i++) {
		RecorderTrack* track = mTracks[i];
		float* input = NULL;
		float* output = NULL;

        if (track->isPriority()) {
            stream->getInterruptBuffers(track->getInputPort(), &input, 
                                        track->getOutputPort(), &output);

            track->processBuffers(stream, input, output, frames, mFrame);

            if (!track->isFinished() || track->isRecording())
              allFinished = false;

            track->setProcessed(true);
        }
        else {
            // always reset previus state
            track->setProcessed(false);
        }
    }

    // then the rest
	for (i = 0 ; i < mTrackCount ; i++) {
		RecorderTrack* track = mTracks[i];
		float* input = NULL;
		float* output = NULL;

        if (!track->isProcessed()) {
            stream->getInterruptBuffers(track->getInputPort(), &input, 
                                        track->getOutputPort(), &output);

            track->processBuffers(stream, input, output, frames, mFrame);
            track->setProcessed(true);

            if (!track->isFinished() || track->isRecording())
              allFinished = false;
        }
    }

	// stop automatically if we're not recording, and all the tracks
	// have finished
	if (mAutoStop && allFinished)
	  mRunning = false;
}

/**
 * Hack for testing.  In Mobius there is a special track class SampleTrack
 * that can inject pre-recorded audio into the input stream when called
 * from a script.  It does this by remembering the input buffer passed to 
 * the track at the last interrupt, and modifying it.
 *
 * When the introduction of rate scaling, this simple approach no longer 
 * worked because each Track would make a copy of the input buffer and
 * no longer see any modifications made by SampleTrack.
 *
 * To fix this, SampleTrack will call this method after it has modified
 * the buffer.  Recorder will then call the inputBufferModified method
 * for all tracks that received the same buffer, and the tracks are expected
 * to assimilate the modified content.  
 *
 * It's a rather ugly flow of control but this is a very specicalized
 * unit testing feature, it doesn't need to be generalized any further.
 */
void Recorder::inputBufferModified(RecorderTrack* track, float* buffer)
{
	for (int i = 0 ; i < mTrackCount ; i++) {
		RecorderTrack* t = mTracks[i];
		if (t != track)
		  t->inputBufferModified(buffer);
	}
}

void Recorder::calibrateInterrupt(float *input, float *output, long frames)
{
    // !! assuming 2 channel ports
	int channels = 2;
	int rate = mContainer->getSampleRate();
	long samples = frames * channels;

	// capture inputs for offline analysis
	if (input != NULL && mCalibrationInput != NULL)
	  mCalibrationInput->put(input, frames, mFrame);

	// initialize to silence
	if (output != NULL) {
		for (int i = 0 ; i < samples ; i++)
		  output[i] = 0;
	}

	// meature a few buffers of noise to determine the noise floor
	// for simplicity, the frame count will be rounded up to a buffer boundary
	if (mFrame < CALIBRATION_NOISE_FRAMES) {

		// only pay attention to the left channel
		if (input != NULL) {
			for (int i = 0 ; i < samples ; i += channels) {
				float sample = input[i];
				if (sample < 0)
				  sample = -sample;
				if (sample > mNoiseAmplitude)
				  mNoiseAmplitude = sample;
			}
		}
		//printf("Noise sample\n");
	}
	else {
		if (mPingFrame > 0) {
			// we're waiting for a response
			long echoFrame = 0;
			if (input != NULL) {
				// formerly tried to base this on a multiple
				// of the measured noise floor, but that was too low
				//float signalAmplitude = mNoiseAmplitude * 2;
				float signalAmplitude = CALIBRATION_ECHO_AMPLITUDE;

				for (int i = 0 ; i < samples ; i += channels) {
					float sample = input[i];
					if (sample < 0)
					  sample = -sample;
					if (sample > signalAmplitude) {
						// found it
						echoFrame = mFrame + (i / channels);
						//printf("Echo amplitude %f\n", sample);
						break;
					}
				}
			}

			if (echoFrame > 0) {
				int latency = echoFrame - mPingFrame;
				mLatencyFrames[mLatencyTest++] = latency;
				if (mLatencyTest == CALIBRATION_TEST_COUNT)
				  mCalibrating = false;

				//printf("Latency measurement %d, %ld - %ld = %ld\n",
				//mLatencyTest, mPingFrame, echoFrame, latency);

				// set to zero to setup another ping
				mPingFrame = 0;

				// save the first buffer so we can get some idea
				// of what the return wave looks like	
				// wait, now we just save the entire input
#if 0
				if (mLatencyTest == 1) {
					Audio *a = mAudioPool->newAudio();
					a->add(input, frames, 0);
					a->write("temp.wav");
					mAudioPool->freeAudio(a);
				}
#endif				
			}
			else {
				// still haven't found it, wait at most one second
				long delta = mFrame - mPingFrame;
                // hiding previous declaration warning
				long samples = delta / channels;
				int seconds = samples / rate;
				if (seconds > 0) {
					mCalibrating = false;
					trace("Calibration timeout!\n");
				}
			}
		}

		// if we're still calibrating, setup a ping 
		if (mCalibrating && mPingFrame == 0) {
			if (output != NULL) {
				int pinglen = channels * CALIBRATION_PING_FRAMES;
				for (int i = 0 ; i < pinglen ; i++)
				  output[i] = CALIBRATION_PING_AMPLITUDE;
			}
			mPingFrame = mFrame;
			//printf("Ping %ld\n", mPingFrame);
		}
	}
}

/****************************************************************************
 *                                                                          *
 *   							RECORDER TRACK                              *
 *                                                                          *
 ****************************************************************************/

RecorderTrack::RecorderTrack() {
	initRecorderTrack();
}

RecorderTrack::RecorderTrack(Audio* a) {
	initRecorderTrack();
	mAudio = a;
}

void RecorderTrack::initRecorderTrack() 
{
	mRecorder = NULL;
	mAudio = NULL;
	mFinished = false;
	mRecording = false;
	mMute = false;
	mInputPort = 0;
	mOutputPort = 0;
	mSelected = false;

	// mThreshold = DEFAULT_RECORD_THRESHOLD;
	mThreshold = 0.0f;
	mRecordStarted = false;
}

/**
 * Called by the Recorder when we're about to start
 * recording.
 */
void RecorderTrack::initAudio()
{
	if (mAudio == NULL && mRecorder == NULL) {
		MobiusContainer* stream = mRecorder->getStream();
        AudioPool* p = mRecorder->getAudioPool();
		mAudio = p->newAudio();
        // !! assuming 2 channel ports
		mAudio->setChannels(2);
		mAudio->setSampleRate(stream->getSampleRate());
	}
}

RecorderTrack::~RecorderTrack() 
{
	delete mAudio;
}

void RecorderTrack::setRecorder(class Recorder *r) {
	mRecorder = r;
}

// removed this since it wanted to create an Audio without a pool
// I think this is a leftover from when we thought this could become
// a more full featured recorder
#if 0
void RecorderTrack::setRecording(bool b)
{
	mRecording = b;
	if (mRecording && mAudio == NULL && mRecorder != NULL) {
		MobiusContainer* s = mRecorder->getStream();
        AudioPool* p = mRecorder->getAudioPool();
		mAudio = p->newAudio();
        // !! assuming 2 channel ports
		mAudio->setChannels(2);
		mAudio->setSampleRate(s->getSampleRate());
	}
}
#endif

void RecorderTrack::dump() 
{
}

void RecorderTrack::reset() 
{
	if (mAudio != NULL)
	  mAudio->reset();
}

bool RecorderTrack::isPriority()
{
    return false;
}

/**
 * Must be overloaded in the subclass if it cares.
 */
void RecorderTrack::inputBufferModified(float* buffer)
{
}

void RecorderTrack::processBuffers(MobiusContainer* stream, 
                                   float *input, float* output, long frames, 
                                   long startFrame)
{
	if (mAudio == NULL)
	  mFinished = true;

	else if (output != NULL && !isMute()) {

		getAudio(output, frames, startFrame);

		// detect when we're at the end
		if (!mRecording && (startFrame + frames > mAudio->getFrames()))
		  mFinished = true;
	}

	if (!mFinished && input != NULL && mRecording) {

		if (!mRecordStarted) {
			if (mThreshold == 0.0f)
			  mRecordStarted = true;	
			else {
				// whip up to the first frame that exceeds the threshold
				// !! this will only work while looping, for normal
				// recording we will still have to either add zero
				// samples or set the relative start time so the
				// recording plays back in sync
				int samples = frames * mAudio->getChannels();
				for (int i = 0 ; i < samples ; i += 2) {
					float s1 = input[i];
					float s2 = input[i+1];
					if (s1 < 0) s1 = -s1;
					if (s2 < 0) s2 = -s2;
					if (s1 > mThreshold || s2 > mThreshold) {
						mRecordStarted = true;
						break;
					}
					else {
						input += 2;
						frames--;
					}
				}
			}
		}

		if (mRecordStarted && frames > 0) {
			//debug("adding frames %d\n", frames);
			// if we're looping, add based on our relative position
			// not where the audio device thinks we are?
			// this will work provided Audio wraps properly

			addAudio(input, frames, startFrame);
		}
	}
}

/**
 * In a base RecorderTrack, we just let Audio extract the block.
 */
void RecorderTrack::getAudio(float* out, long frames, long frameOffset)
{
	if (mAudio != NULL) {
		AudioBuffer b;
		b.buffer = out;
		b.frames = frames;
		b.channels = 2;
		mAudio->get(&b, frameOffset);
	}
}

/**
 * In a base RecorderTrack, we simply append the new frames.
 */
void RecorderTrack::addAudio(float* src, long newFrames, long startFrame)
{
	// won't this always be the same as append() ?
	if (mAudio != NULL) {
		AudioBuffer b;
		b.buffer = src;
		b.frames = newFrames;
		b.channels = 2;
		mAudio->put(&b, startFrame);
	}
}

/****************************************************************************
 *                                                                          *
 *                                SIGNAL TRACK                              *
 *                                                                          *
 ****************************************************************************/

SignalTrack::SignalTrack()
{
    initRecorderTrack();
}

SignalTrack::~SignalTrack()
{
}

void SignalTrack::fillOutputBuffer(float *out, long frames, long frameOffset)
{
    // CD sample rate in stereo
    // 8K cycles per second
    int freq = 8000;

	// technically this needs to be adjusted based upon the
	// starting frameOffset, assume for now that everything falls
	// on a nice cycle boundary
    int psn = 0;
    float sample = 0.0f;

    for (int i = 0 ; i < frames ; i++) {
        for (int c = 0 ; c < 2 ; c++) {
            if (psn == freq) {
                // toggle the sample
                if (sample == 0.0f)
                  sample = 0.9f;
                else
                  sample = 0.0f;
                psn = 0;
            }
            *out++ = sample;
            psn++;
        }
    }
}

/****************************************************************************
 *                                                                          *
 *   							   RECORDER                                 *
 *                                                                          *
 ****************************************************************************/

/**
 * Build out the Recorder with a container and a pool
 * What is now MobiusContainer used to be AudioInterface and we
 * had more control over how it was configured.  Now we take what we get.
 * Defer complex initialization to initialize()
 */
Recorder::Recorder()
{
    mContainer = nullptr;
    mAudioPool = nullptr;

	mMonitor = NULL;
	mLatency = 0;
	mFrame = 0;
	mRunning = false;
	mAutoStop = false;
	mInInterrupt = false;
	mEcho = false;
	mCalibrationInput = NULL;
	mCalibrating = false;
	mNoiseAmplitude = 0.0;
	mPingFrame = 0;
	mLatencyTest = 0;
	mLastInterruptTime = 0;

	mTrackCount = 0;
	for (int i = 0 ; i < MAX_RECORDER_TRACKS ; i++)
	  mTracks[i] = NULL;
}

/**
 * We're doing memory management here, so this can only be done
 * outside the audio thread.  In practice the kernel creates
 * one Recorder and uses it for it's lifespan so we're safe.
 */
Recorder::~Recorder() 
{
	shutdown();

	for (int i = 0 ; i < MAX_RECORDER_TRACKS ; i++) {
		if (mTracks[i] != NULL) {
			delete mTracks[i];
			mTracks[i] = NULL;
		}
	}
}

/**
 * Initialize the Recorder after construction.
 * Here the relevant objects have finished construction (static or dynamic)
 * and it's safe to start wiring things together.
 * Formerly done in the constructor, which is probably still safe.
 *
 * Where are we supposed to know about the track configuration?
 */
void Recorder::initialize(MobiusContainer* cont, AudioPool* pool)
{
    mContainer = cont;
    mAudioPool = pool;

    // wait on this till we're ready
    //if (mContainer != nullptr)
    //mContainer->setAudioListener(this);
}

/**
 * Unclear where this was called in old code.  I don't think
 * you could just randomly initialize and shutdown the Recorder,
 * but if you can there is more state here that would need to be reset.
 * It formerly closed the audio streams if we were in control over them.
 */
void Recorder::shutdown() 
{
	stop();
	mMonitor = nullptr;
    mContainer = nullptr;
}

void Recorder::setMonitor(RecorderMonitor* m)
{
	mMonitor = m;
}

/**
 * What once once just a stream, has metamorphasized into
 * a beatiful container.
 */
MobiusContainer* Recorder::getStream()
{
	return mContainer;
}

AudioPool* Recorder::getAudioPool()
{
    return mAudioPool;
}

long Recorder::getFrame() 
{
    return mFrame;
}

int Recorder::getTrackCount() 
{
    return mTrackCount;
}

RecorderTrack* Recorder::getTrack(int i) 
{
    return mTracks[i];
}

void Recorder::setEcho(bool b) {
	mEcho = b;
}

void Recorder::setAutoStop(bool b) {
	mAutoStop = b;
}

/****************************************************************************
 *                                                                          *
 *                                   TRACKS                                 *
 *                                                                          *
 ****************************************************************************/

//
// NOTE: There is memory management going on here, but this can't
// in practice after initialization so it's safe though it looks
// scary for what is mostly audio-thread code in this file.  Shold
// try to clean this up so it is clearer when things happen
//

/**
 * Mark one of the tracks as selected.  Down here, this will
 * be used to determine which input port should be echoed.
 */
void Recorder::select(RecorderTrack* t)
{
	for (int i = 0 ; i < mTrackCount ; i++) {
		RecorderTrack* track = mTracks[i];
		track->setSelected(track == t);
	}
}

/**
 * Add a preconstructed track.
 */
bool Recorder::add(RecorderTrack *t) 
{
    bool added = false;
	if (t != NULL && mTrackCount < MAX_RECORDER_TRACKS) {
        Audio* audio = t->getAudio();
        if (audio == NULL || checkAudio(audio)) {
            t->setRecorder(this);
            mTracks[mTrackCount] = t;
            mTrackCount++;
            added = true;
        }
    }
    return added;
}

/**
 * Verify that an Audio can be played by this recorder configuration.
 * This is proably more anal than it needs to be, but in practice we never
 * dealt with Audio data that didn't conform.
 */
bool Recorder::checkAudio(Audio *a) 
{
    bool ok = true;

    if (mTrackCount == 0) {
        // first one in gets to determine the configuration
        // !! assuming 2 channel ports
        if (a->getChannels() != 2) {
            trace("ERROR: Audio with %d channels!!\n", a->getChannels());
            fflush(stdout);
        }

        // formerly tried to make the stream follow the sample rate
        // stored in the audio file, I don't think this ever really worked
		//mStream->setSampleRate(a->getSampleRate());
    }
    else if (a->getChannels() != 2) {
        trace("Unable to load audio, incompatible channels.\n");
        ok = false;
    }
    else if (mContainer->getSampleRate() != a->getSampleRate()) {
        // we used to bail here, now just ignore it
        trace("Warning: RecorderTrack audio had ideas about sample rate\n");
        // ok = false;
    }

	return ok;
}

/**
 * Create and add a track for an Audio.
 */
RecorderTrack* Recorder::add(Audio *a) 
{
	RecorderTrack* track = new RecorderTrack(a);
    if (!add(track)) {
        delete track;
        track = NULL;
    }
    return track;
}

/**
 * New for SampleTrack in the kernel.
 * We've built an entirely new SampleTrack object containing
 * samples and need to replace the old one.  This is the only
 * track type that will have this treatment at the moment.
 * The old track is NOT deleted.
 * Kind of inconsistent with the way other Juce containers
 * work that normally own things when they delete them.
 * Would be more accurate to say replaceNoDelete
 * or have a deleteIt=false argument to specify this.
 */
bool Recorder::replace(RecorderTrack* old, RecorderTrack* neu)
{
    bool replaced = false;
    
    for (int i = 0 ; i < mTrackCount ; i++) {
        RecorderTrack *rt = mTracks[i];
        if (rt == old) {
            mTracks[i] = neu;
            replaced = true;
            break;
        }
    }
    return replaced;
}

/**
 * Remove a track by number.
 */
bool Recorder::removeTrack(int n) 
{
    bool removed = false;
	if (n >= 0 && n < mTrackCount) {
		int last = mTrackCount - 1;
		for (int i = n ; i < last ; i++)
		  mTracks[i] = mTracks[i+1];
        mTracks[last] = NULL;
		mTrackCount--;
        removed = true;
	}
    return removed;
}

/**
 * Remove a previously installed track.
 */
bool Recorder::remove(RecorderTrack* t) 
{
    bool removed = false;
	if (t != NULL) {
		for (int i = 0 ; i < mTrackCount ; i++) {
			if (mTracks[i] == t) {
				removed = removeTrack(i);
				break;
			}
		}
	}
    return removed;
}

/**
 * Remove the track containing an Audio.
 */
bool Recorder::remove(Audio *a) 
{
    bool removed = false;

	if (a != NULL) {
		for (int i = 0 ; i < mTrackCount ; i++) {
			if (mTracks[i]->getAudio() == a) {
				removed = removeTrack(i);
				break;
			}
		}
	}
    return removed;
}

/****************************************************************************
 *                                                                          *
 *   							  TRANSPORT                                 *
 *                                                                          *
 ****************************************************************************/

void Recorder::setFrame(long f) {
	
	stop();
	mFrame = f;
}

void Recorder::setTime(int seconds) 
{
    setFrame(seconds * mContainer->getSampleRate());
}

bool Recorder::isRunning() 
{
	return mRunning;
}

void Recorder::start() 
{
	if (!mRunning) {
		// Be sure every track that is enabled for recording
		// has an audio object.  
		for (int i = 0 ; i < mTrackCount ; i++) {
			RecorderTrack* t = mTracks[i];
			if (t->isRecording())
			  t->initAudio();
		}
        mRunning = true;
	}
}

void Recorder::stop() 
{
	if (mRunning) {
        // Note that we do NOT stop the stream, just set a flag 
        // to suppress activity in the interrupt handler
		mRunning = false;
		mCalibrating = false;
	}
}

/****************************************************************************
 *                                                                          *
 *   							 CALIBRATION                                *
 *                                                                          *
 ****************************************************************************/

RecorderCalibrationResult* Recorder::calibrate() 
{
	RecorderCalibrationResult* result = new RecorderCalibrationResult();

	stop();

	mNoiseAmplitude = 0.0f;
	mLatencyTest = 0;
	mFrame = 0;
	mCalibrationInput = mAudioPool->newAudio();
	mCalibrating = true;

	start();

    // dire interwebz warnings about using sleep of any kind, which are true
    // but calibration is a very special case that is not used normally
    // and I don't want to mess with Timer events
	for (int i = 0 ; i < 5 && mCalibrating ; i++) {
        mContainer->sleep(1000);
    }

	result->noiseFloor = mNoiseAmplitude;

	if (mLatencyTest == 0 || mCalibrating)
	  result->timeout = true;
	else {
		int latency = 0;
		for (int i = 0 ; i < mLatencyTest ; i++)
		  latency += mLatencyFrames[i];
		latency /= mLatencyTest;
		result->latency = latency;
	}

    // don't have write() anymore, revisit this
	//mCalibrationInput->write("calibration.wav");
    
    mAudioPool->freeAudio(mCalibrationInput);
    mCalibrationInput = NULL;
    mCalibrating = false;

	return result;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

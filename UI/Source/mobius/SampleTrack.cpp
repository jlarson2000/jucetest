/**
 * outstanding issues:
 *   SampleTrack::updateConfiguration
 * 
 * This file contains the implementation of SampleTrack methods
 * related to runtime execution.
 *
 * Code for building a SampleTrack is in SampleTrackBuilder
 *
 */

#include "../util/Trace.h"
#include "../util/Util.h"
#include "../model/MobiusConfig.h"
#include "../model/SampleConfig.h"

#include "MobiusContainer.h"

#include "Audio.h"
#include "Recorder.h"

#include "SampleTrack.h"

//////////////////////////////////////////////////////////////////////
//
// SamplePlayer
//
//////////////////////////////////////////////////////////////////////

const char* SamplePlayer::getFilename()
{
    return mFilename;
}

void SamplePlayer::setNext(SamplePlayer* sp)
{
	mNext = sp;
}

SamplePlayer* SamplePlayer::getNext()
{
	return mNext;
}

void SamplePlayer::setAudio(Audio* a)
{
	mAudio = a;
}

Audio* SamplePlayer::getAudio()
{
	return mAudio;
}

void SamplePlayer::setSustain(bool b)
{
	mSustain = b;
}

bool SamplePlayer::isSustain()
{
	return mSustain;
}

void SamplePlayer::setLoop(bool b)
{
	mLoop = b;
}

bool SamplePlayer::isLoop()
{
	return mLoop;
}

void SamplePlayer::setConcurrent(bool b)
{
	mConcurrent = b;
}

bool SamplePlayer::isConcurrent()
{
	return mConcurrent;
}

long SamplePlayer::getFrames()
{
	long frames = 0;
	if (mAudio != nullptr)
	  frames = mAudio->getFrames();
	return frames;
}

/**
 * Incorporate changes made to the global configuration.
 * Trying to avoid a Mobius dependency here so pass in what we need.
 * Keep this out of SampleCursor so we don't have to mess with
 * synchronization between the UI and audio threads.
 *
 * UPDATE: Sample triggers are handled by Actions now so we will
 * always be in the audio thread.
 */
void SamplePlayer::updateConfiguration(int inputLatency, int outputLatency)
{
    mInputLatency = inputLatency;
    mOutputLatency = outputLatency;
}

/**
 * If this is bound to the keyboard, auto-repeat will keep
 * feeding us triggers rapidly.  If this isn't a sustain sample,
 * then assume this means we're supposed to restart.  If it is a
 * sustain sample, then we need to wait for an explicit up trigger.
 * This state has to be held even after a non-loop sample has finished
 * playing and become inactive.
 */
void SamplePlayer::trigger(bool down)
{

	// !! still having the auto-repeat problem with non-sustained
	// concurrent samples

    bool doTrigger = false;
    if (down) {
        if (!mDown || !mSustain)
          doTrigger = true;
        mDown = true;
    }
    else {
        // only relevant for sustained samples
        if (mSustain)
          doTrigger = true;
        mDown = false;
    }

    if (doTrigger) {
        int nextTail = mTriggerTail + 1;
        if (nextTail >= MAX_TRIGGERS)
          nextTail = 0;

        if (nextTail == mTriggerHead) {
            // trigger overflow, audio must be unresponsive or
            // we're receiving triggers VERY rapidly, would be nice
            // to detect unresponse audio and just start ignoring
            // triggers
            Trace(1, "SamplePlayer::trigger trigger overflow\n");
        }
        else {
            // eventually have other interesting things here, like key
            mTriggers[mTriggerTail].down = down;
            mTriggerTail = nextTail;
        }

    }
}

/**
 * Play/Record the sample.
 * 
 * Playback is currently inaccurate in that we'll play from the beggining
 * when we should logically start from mOutputLatency in order to synchronize
 * the recording with the output.  
 *
 * Recording is done accurately.  The frame counter is decremented by
 * mInputLatency, and when this goes positive we begin filling the input 
 * buffer.
 * 
 */
void SamplePlayer::play(float* inbuf, float* outbuf, long frames)
{
    // process triggers
    while (mTriggerHead != mTriggerTail) {
        SampleTrigger* t = &mTriggers[mTriggerHead++];
		if (mTriggerHead >= MAX_TRIGGERS)
		  mTriggerHead = 0;

        if (!t->down) {
            if (mConcurrent) {
                // the up transition belongs to the first cursor
                // that isn't already in the process of stopping
                for (SampleCursor* c = mCursors ; c != nullptr ; 
                     c = c->getNext()) {
                    if (!c->isStopping()) {
                        c->stop();
                        break;
                    }
                }
            }
            else {
                // should be only one cursor, make it stop
                if (mCursors != nullptr)
                  mCursors->stop();
            }
        }
        else if (mConcurrent) {
            // We start another cursor and let the existing ones finish
            // as they may.  Keep these ordered.
            SampleCursor* c = newCursor();
			SampleCursor* last = nullptr;
            for (last = mCursors ; last != nullptr && last->getNext() != nullptr ; 
                 last = last->getNext());
            if (last != nullptr)
              last->setNext(c);
            else
              mCursors = c;
        }
        else {
            // stop existing cursors, start a new one
            // the effect is similar to a forced up transition but
            // we want the current cursor to end cleanly so that it
            // gets properly recorded and fades nicely.

            SampleCursor* c = newCursor();
			SampleCursor* last = nullptr;
            for (last = mCursors ; last != nullptr && last->getNext() != nullptr ; 
                 last = last->getNext()) {
                // stop them while we look for the last one
                last->stop();
            }
            if (last != nullptr)
              last->setNext(c);
            else
              mCursors = c;
        }
    }

    // now process cursors

    SampleCursor* prev = nullptr;
    SampleCursor* next = nullptr;
    for (SampleCursor* c = mCursors ; c != nullptr ; c = next) {
        next = c->getNext();

        c->play(inbuf, outbuf, frames);
        if (!c->isStopped())
          prev = c;
        else {
            // splice it out of the list
            if (prev == nullptr)
              mCursors = next;
            else
              prev->setNext(next);
            freeCursor(c);
        }
    }
}

/**
 * Allocate a cursor.
 * Keep these pooled since there are several things in them.
 * Ideally there should be only one pool, but we would have
 * to root it in SampleTrack and pass it down.
 */
SampleCursor* SamplePlayer::newCursor()
{
    SampleCursor* c = mCursorPool;
    if (c == nullptr) {
        c = new SampleCursor(this);
    }
    else {
        mCursorPool = c->getNext();
		c->setNext(nullptr);
        c->setSample(this);
    }
    return c;
}

/**
 * Return a cursor to the pool.
 */
void SamplePlayer::freeCursor(SampleCursor* c)
{
    c->setNext(mCursorPool);
    mCursorPool = c;
}

//////////////////////////////////////////////////////////////////////
//
// SampleCursor
//
//////////////////////////////////////////////////////////////////////

/*
 * Each cursor represents the playback of one trigger of the
 * sample.  To implement the insertion of the sample into
 * the recorded audio stream, we'll actually maintain two cursors.
 * The outer cursor handles the realtime playback of the sample, 
 * the inner cursor handles the "recording" of the sample into the
 * input stream.  
 *
 * Implementing this as cursor pairs was easy since they have to 
 * do almost identical processing, and opens up some interesting
 * possibilities.
 */

void SampleCursor::setNext(SampleCursor* c)
{
    mNext = c;
}

SampleCursor* SampleCursor::getNext()
{
    return mNext;
}

/**
 * Reinitialize a pooled cursor.
 *
 * The logic is quite contorted here, but every cursor appears to 
 * have an embedded record cursor.  
 */
void SampleCursor::setSample(SamplePlayer* s)
{
    mSample = s;
	mAudioCursor->setAudio(mSample->getAudio());
    mStop = false;
    mStopped = false;
    mMaxFrames = 0;

    if (mRecord != nullptr) {
        // we're a play cursor
        mRecord->setSample(s);
        mFrame = 0;
    }
    else {
        // we're a record cursor

		// !! This stopped working after the great autorecord/sync 
		// rewrite.  Scripts are expecting samples to play into the input 
		// buffer immediately, at least after a Wait has been executed
		// and we're out of latency compensation mode.  We probably need to 
		// be more careful about passing the latency context down
		// from SampleTrack::trigger, until then assume we're not
		// compensating for latency

        //mFrame = -(mSample->mInputLatency);
		mFrame = 0;
    }

}

bool SampleCursor::isStopping()
{
    return mStop;
}

bool SampleCursor::isStopped()
{
    bool stopped = mStopped;
    
    // if we're a play cursor, then we're not considered stopped
    // until the record cursor is also stopped
    if (mRecord != nullptr)
      stopped = mRecord->isStopped();

    return stopped;
}

/**
 * Called when we're supposed to stop the cursor.
 * We'll continue on for a little while longer so we can fade
 * out smoothly.  This is called only for the play cursor,
 * the record cursor lags behind so we call stop(frame) when
 * we know the play frame to stop on.
 * 
 */
void SampleCursor::stop()
{
    if (!mStop) {
		long maxFrames = 0;
        Audio* audio = mSample->getAudio();
		long sampleFrames = audio->getFrames();
		maxFrames = mFrame + AudioFade::getRange();
		if (maxFrames >= sampleFrames) {
			// must play to the end assume it has been trimmed
			// !! what about mLoop, should we set this
			// to sampleFrames so it can end?
			maxFrames = 0;
		}

		stop(maxFrames);
		if (mRecord != nullptr)
		  mRecord->stop(maxFrames);
	}
}

/**
 * Called for both the play and record cursors to stop on a given
 * frame.  If the frame is before the end of the audio, then we set
 * up a fade.
 */
void SampleCursor::stop(long maxFrames)
{
    if (!mStop) {
        Audio* audio = mSample->getAudio();
		if (maxFrames > 0)
		  mAudioCursor->setFadeOut(maxFrames);
		mMaxFrames = maxFrames;
        mStop = true;
    }
}

/**
 * Play/Record more frames in the sample.
 */
void SampleCursor::play(float* inbuf, float* outbuf, long frames)
{
	// play
	if (outbuf != nullptr)
	  play(outbuf, frames);

	// record
	if (mRecord != nullptr && inbuf != nullptr)
	  mRecord->play(inbuf, frames);
}

/**
 * Play more frames in the sample.
 */
void SampleCursor::play(float* outbuf, long frames)
{
    Audio* audio = mSample->getAudio();
    if (audio != nullptr && !mStopped) {

        // consume dead input latency frames in record cursors
        if (mFrame < 0) {
            mFrame += frames;
            if (mFrame > 0) {
                // we advanced into "real" frames, back up
                int ignored = frames - mFrame;
                outbuf += (ignored * audio->getChannels());
                frames = mFrame;
                mFrame = 0;
            }
            else {
                // nothing of interest for this buffer
                frames = 0;
            }
        }

        // now record if there is anything left in the buffer
        if (frames > 0) {

			// !! awkward interface
			AudioBuffer b;
			b.buffer = outbuf;
			b.frames = frames;
			b.channels = 2;
			mAudioCursor->setAudio(audio);
			mAudioCursor->setFrame(mFrame);

            long sampleFrames = audio->getFrames();
            if (mMaxFrames > 0)
              sampleFrames = mMaxFrames;
            
            long lastBufferFrame = mFrame + frames - 1;
            if (lastBufferFrame < sampleFrames) {
				mAudioCursor->get(&b);
                mFrame += frames;
            }
            else {
                long avail = sampleFrames - mFrame;
                if (avail > 0) {
					b.frames = avail;
					mAudioCursor->get(&b);
                    mFrame += avail;
                }

                // if we get to the end of a sustained sample, and the
                // trigger is still down, loop again even if the loop 
                // option isn't on

                if (!mSample->mLoop &&
                    !(mSample->mDown && mSample->mSustain)) {
                    // we're done
                    mStopped = true;
                }
                else {
                    // loop back to the beginning
                    long remainder = frames - avail;
                    outbuf += (avail * audio->getChannels());

                    // should already be zero since if we ended a sustained
                    // sample early, it would have been handled in stop()?
                    if (mMaxFrames > 0)
                      Trace(1, "SampleCursor::play unexpected maxFrames\n");
                    mMaxFrames = 0;
                    mFrame = 0;

                    sampleFrames = audio->getFrames();
                    if (sampleFrames < remainder) {
                        // sample is less than the buffer size?
                        // shouldn't happen, handling this would make this
                        // much more complicated, we'd have to loop until
                        // the buffer was full
                        remainder = sampleFrames;
                    }

					b.buffer = outbuf;
					b.frames = remainder;
					mAudioCursor->setFrame(mFrame);
					mAudioCursor->get(&b);
                    mFrame += remainder;
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////
//
// SampleTrack
//
//////////////////////////////////////////////////////////////////////

/**
 * Don't need this.  mSampleCount was calculated at constructed and you
 * can't change the list after that.
 */
int SampleTrack::getSampleCount()
{
    int count = 0;
    for (SamplePlayer* p = mPlayerList ; p != nullptr ; p = p->getNext())
      count++;
    return count;
}

/**
 * Must overload this so we're processed first and can insert audio
 * into the input buffer.
 */
bool SampleTrack::isPriority()
{
    return true;
}

/**
 * Called whenever a new MobiusConfig is installed in the interrupt
 * handler.  Check for changes in latency overrides.
 * Note that we have to go to Mobius rather than look in the MobiusConfig
 * since the config is often zero so we default to what the device tells us.
 * !! This will end up using the "master" MobiusConfig rather than
 * mInterruptConfig.  For this it shouldn't matter but it still feels
 * unclean.  
 */
void SampleTrack::updateConfiguration(MobiusConfig* config)
{
    // jsl - commenting this out because I want to understand where
    // it is called, we don't have a mMobius pointer any more
    // and if we're going to get latencies should be getting them
    // from the MobiusContainer
/*    
    // config is ignored since we're only interested in latencies right now
	for (int i = 0 ; i < mSampleCount ; i++)
	  mPlayers[i]->updateConfiguration(mMobius->getEffectiveInputLatency(),
									   mMobius->getEffectiveOutputLatency());
*/
}

/**
 * Trigger a sample to begin playing.
 * Called by the SamplePlay action in the interrupt.
 *
 * There used to be some extremely contorted logic in here to do processing
 * of the AudioStream buffers early, which would possibly (always) modify the
 * input buffer to inject the sample into the input then call back to
 * Recorder::inputBufferModified for some obscure reason to cause reprocessing
 * of tracks for the new input, which shouldn't have been necessary if we were
 * processed first.  I gave up trying to understand it all, ripped it out and
 * started over.
 *
 * Comments in old code
 *
 * !!! This feels full of race conditions.  The unit tests do this
 * in scripts so often we will be inside the interrupt.  But the
 * SampleTrigger function is declared as global so if triggered
 * from MIDI it will be run outside the interrupt.
 * 
 * If we are being run during the script pass at the start of the interrupt,
 * then the sample will be immediately played into the input/output buffers in
 * the processBufffers interrupt handler below.  This is normally done
 * for testing with scripts using the "Wait block" statement to ensure
 * that the sample is aligned with an interrupt block.
 *
 * For scripts triggered with MIDI, keys, or buttons, we will trigger them
 * but won't actually begin playing until the next interrupt when 
 * processBuffers is run again.  This means that for predictable content
 * in the unit tests you must use "Wait block" 
 *
 * KLUDGE: Originally triggering always happened during processing of a Track
 * after we had called the SamplerTrack interrupt handler.  So we could begin
 * hearing the sample in the current block we begin proactively playing it here
 * rather than waiting for the next block.  This is arguably incorrect, we should
 * just queue it up and wait for the next interrupt.  But unfortunately we have
 * a lot of captured unit tests that rely on this.  Until we're prepared
 * to recapture the test files, follow the old behavior.  But note that we have
 * to be careful *not* to do this twice in one interupt.  Now that scripts
 * are called before any tracks are processed, we may not need to begin playing, 
 * but wait for SampleTrack::processBuffers below.
 */

void SampleTrack::trigger(int index, bool down)
{
	if (index < mSampleCount) {
		mPlayers[index]->trigger(down);
		mLastSample = index;

        // see old code about filling buffers early if you think
        // this is necessary again
	}
	else {
		// this is sometimes caused by a misconfiguration of the
		// the unit tests
		Trace(1, "ERROR: No sample at index %ld\n", (long)index);
	}

}

long SampleTrack::getLastSampleFrames()
{
	long frames = 0;
	if (mLastSample >= 0)
	  frames = mPlayers[mLastSample]->getFrames();
	return frames;
}

//////////////////////////////////////////////////////////////////////
//
// Interrupt Handler
//
//////////////////////////////////////////////////////////////////////

void SampleTrack::prepareForInterrupt()
{
	// kludge see comments in trigger()
	mTrackProcessed = false;
}


void SampleTrack::processBuffers(MobiusContainer* stream, 
								 float* inbuf, float *outbuf, long frames, 
								 long frameOffset)
{
	for (int i = 0 ; i < mSampleCount ; i++)
	  mPlayers[i]->play(inbuf, outbuf, frames);

	mTrackProcessed = true;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

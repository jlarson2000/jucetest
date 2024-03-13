/**
 * This isn't used any more, but might want to resurrect it in the
 * new MobiusContainer model?
 */

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
 *   							 PORT BUFFERS                               *
 *                                                                          *
 ****************************************************************************/

// don't think this is used, but it's fairly self contained to leave it around

AudioPort::AudioPort()
{
    mNumber = 0;
    mChannels = 2;
    mFrameOffset = 0;
	mPrepared = false;
	mBuffer = new float[AUDIO_MAX_SAMPLES_PER_BUFFER];
}

AudioPort::~AudioPort()
{
	delete mBuffer;
}

void AudioPort::setNumber(int i)
{
    mNumber = i;
}

int AudioPort::getNumber()
{
    return mNumber;
}

void AudioPort::setChannels(int i)
{
	mChannels = i;
}

int AudioPort::getChannels()
{
    return mChannels;
}

void AudioPort::setFrameOffset(int i)
{
	mFrameOffset = i;
}

void AudioPort::reset()
{
	mPrepared = false;
}

/**
 * Extract the left and right channels for one audio port from the
 * combined buffer given on each interrupt.
 *
 * A 4 channel PortAudio interrupt buffers look like this:
 *
 *   ch1,ch2,ch3,ch4|ch1,ch2,ch3,ch4
 *
 * We logically group channel pairs into ports for:
 *
 *   p1l,p1r,p2l,p2r|p1l,p1r,p2l,p2r
 *
 */
float* AudioPort::extract(float* src, long frames, int channels)
{
	if (!mPrepared) {

		float* dest = mBuffer;

        // the last port on a device may have only one if this
        // is a mono device, duplicate the 
        bool mono = ((mFrameOffset + 1) == channels);

		for (int i = 0 ; i < frames ; i++) {

			float* portSrc = src + mFrameOffset;
            float sample = *portSrc++;

            *dest++ = sample;
            if (mono)
              *dest++ = sample;
            else
              *dest++ = *portSrc++;

			src += channels;
		}

		mPrepared = true;
	}

	return mBuffer;
}

/**
 * Prepare an output buffer.
 */
float* AudioPort::prepare(long frames)
{
	if (!mPrepared) {
		int samples = frames * mChannels;
		memset(mBuffer, 0, (sizeof(float) * samples));
		mPrepared = true;
	}
	return mBuffer;
}

/**
 * Copy the contents of one port's output into the multi-port 
 * interrupt buffer.
 */
void AudioPort::transfer(float* dest, long frames, int channels)
{
	if (!mPrepared) {
		// assume the target buffer was already initialized to zero
	}
	else {
		float* src = mBuffer;

        // shouldn't see mono ports on output but support
        // I guess we could be summing these but if we had a mono
        // that was split by extract() summing would end up doubling
        // the input
        bool mono = ((mFrameOffset + 1) == channels);

		for (int i = 0 ; i < frames ; i++) {

			float* portDest = dest + mFrameOffset;

            *portDest++ = *src++;
            if (!mono)
              *portDest++ = *src++;

			dest += channels;
		}
	}
}


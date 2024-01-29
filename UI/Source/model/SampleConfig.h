/*
 * Copyright (c) 2010 Jeffrey S. Larson  <jeff@circularlabs.com>
 * All rights reserved.
 * See the LICENSE file for the full copyright and license declaration.
 * 
 * ---------------------------------------------------------------------
 * 
 * Sample is a model for sample files that can be loaded for triggering.
 *
 * SampleTrack is an extension of RecorderTrack that adds basic sample
 * playback capabilities to Mobius
 *
 */

#ifndef SAMPLE_CONFIG_H
#define SAMPLE_CONFIG_H

//////////////////////////////////////////////////////////////////////
//
// Samples
//
//////////////////////////////////////////////////////////////////////

/**
 * Encapsulates a collection of samples for configuration storage.
 * One of these can be the MoibusConfig as well as local to a Project.
 * This will also be given to SampleTrack.
 */
class SampleConfig
{
  public:

	SampleConfig();
	SampleConfig(class XmlElement* e);
	~SampleConfig();

	void clear();
	void add(Sample* s);

	Sample* getSamples();

	void parseXml(class XmlElement* e);
	void toXml(class XmlBuffer* b);

  private:

	Sample* mSamples;
	
};

//////////////////////////////////////////////////////////////////////
//
// Sample
//
//////////////////////////////////////////////////////////////////////

/**
 * The definition of a sample that can be played by SampleTrack.
 * A list of these will be found in a Samples object which in turn
 * will be in the MobiusConfig.
 *
 * We convert these to SamplePlayers managed by one SampleTrack.
 */
class Sample
{
  public:

	Sample();
	Sample(const char* file);
	Sample(class XmlElement* e);
	~Sample();

	void setNext(Sample* s);
	Sample* getNext();

	void setFilename(const char* file);
	const char* getFilename();

	void setSustain(bool b);
	bool isSustain();

	void setLoop(bool b);
	bool isLoop();

    void setConcurrent(bool b);
    bool isConcurrent();

	void parseXml(class XmlElement* e);
	void toXml(class XmlBuffer* b);

  private:
	
	void init();

	Sample* mNext;
	char* mFilename;

    //
    // NOTE: These were experimental options that have never
    // been used.  It doesn't feel like these should even be
    // Sample-specific options but maybe...
    // 

	/**
	 * When true, playback continues only as long as the trigger
	 * is sustained.  When false, the sample always plays to the end
	 * and stops.
	 */
	bool mSustain;

	/** 
	 * When true, playback loops for as long as the trigger is sustained
	 * rather than stopping when the audio ends.  This is relevant
	 * only if mSustain is true.
     *
     * ?? Is this really necessary, just make this an automatic
     * part of mSustain behavior.  It might be fun to let this
     * apply to non sustained samples, but then we'd need a way
     * to shut it off!  Possibly the down transition just toggles
     * it on and off.
	 */
	bool mLoop;

    /**
     * When true, multiple overlapping playbacks of the sample
     * are allowed.  This is really meaningful only when mSustain 
     * is false since you would have to have an up transition before
     * another down transition.  But I suppose you could configure
     * a MIDI controller to do that.  This is also what you'd want
     * to implement pitch adjusted playback of the same sample.
     */
    bool mConcurrent;

};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
#endif

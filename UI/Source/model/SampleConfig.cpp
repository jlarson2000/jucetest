/**
 * Configuration model for Samples that can be sent to the Mobius engine
 * for playback.
 */

#include "../util/Util.h"

#include "SampleConfig.h"

//////////////////////////////////////////////////////////////////////
//
// SampleConfig
//
//////////////////////////////////////////////////////////////////////

SampleConfig::SampleConfig()
{
	mSamples = nullptr;
}

SampleConfig::~SampleConfig()
{
	delete mSamples;
}

Sample* SampleConfig::getSamples()
{
	return mSamples;
}

void SampleConfig::setSamples(Sample* list)
{
    delete mSamples;
    mSamples = list;
}

void SampleConfig::clear()
{
	delete mSamples;
	mSamples = nullptr;
}

void SampleConfig::add(Sample* neu)
{
	Sample* last = nullptr;
	for (Sample* s = mSamples ; s != nullptr ; s = s->getNext())
	  last = s;

	if (last == nullptr)
	  mSamples = neu;
	else
	  last->setNext(neu);
}

//////////////////////////////////////////////////////////////////////
//
// Sample
//
//////////////////////////////////////////////////////////////////////

Sample::Sample()
{
	init();
}

Sample::Sample(const char* file)
{
	init();
	setFilename(file);
}

void Sample::init()
{
	mNext = nullptr;
	mFilename = nullptr;
	mSustain = false;
	mLoop = false;
	mConcurrent = false;
    mData =  nullptr;
}

Sample::~Sample()
{
	delete mFilename;
    delete mData;
	
    Sample* next = nullptr;
    for (Sample* s = mNext ; s != nullptr ; s = next) {
        next = s->getNext();
		s->setNext(nullptr);
        delete s;
    }
}

void Sample::setNext(Sample* s) 
{
	mNext = s;
}

Sample* Sample::getNext()
{
	return mNext;
}

void Sample::setFilename(const char* s)
{
	delete mFilename;
	mFilename = CopyString(s);
}

const char* Sample::getFilename()
{
	return mFilename;
}

void Sample::setSustain(bool b)
{
	mSustain = b;
}

bool Sample::isSustain()
{
	return mSustain;
}

void Sample::setLoop(bool b)
{
	mLoop = b;
}

bool Sample::isLoop()
{
	return mLoop;
}

void Sample::setConcurrent(bool b)
{
	mConcurrent = b;
}

bool Sample::isConcurrent()
{
	return mConcurrent;
}

float* Sample::getData()
{
    return mData;
}

void Sample::setData(float* data)
{
    delete mData;
    mData = data;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

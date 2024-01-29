/*
 * Copyright (c) 2010 Jeffrey S. Larson  <jeff@circularlabs.com>
 * All rights reserved.
 * See the LICENSE file for the full copyright and license declaration.
 * 
 * ---------------------------------------------------------------------
 * 
 * Sample is a model for sample files that can be loaded for triggering.
 *
 */

#include "SampleConfig.h"

//////////////////////////////////////////////////////////////////////
//
// SampleConfig
//
//////////////////////////////////////////////////////////////////////

SampleConfig::SampleConfig()
{
	mSamples = NULL;
}

SampleConfig::SampleConfig(XmlElement* e)
{
	mSamples = NULL;
	parseXml(e);
}

SampleConfig::~SampleConfig()
{
	delete mSamples;
}

Sample* SampleConfig::getSamples()
{
	return mSamples;
}

void SampleConfig::clear()
{
	delete mSamples;
	mSamples = NULL;
}

void SampleConfig::add(Sample* neu)
{
	Sample* last = NULL;
	for (Sample* s = mSamples ; s != NULL ; s = s->getNext())
	  last = s;

	if (last == NULL)
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

Sample::Sample(XmlElement* e)
{
	init();
	parseXml(e);
}

void Sample::init()
{
	mNext = NULL;
	mFilename = NULL;
	mSustain = false;
	mLoop = false;
	mConcurrent = false;
}

Sample::~Sample()
{
	delete mFilename;
	
    Sample* next = NULL;
    for (Sample* s = mNext ; s != NULL ; s = next) {
        next = s->getNext();
		s->setNext(NULL);
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

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
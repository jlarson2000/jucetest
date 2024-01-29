/*
 * Copyright (c) 2010 Jeffrey S. Larson  <jeff@circularlabs.com>
 * All rights reserved.
 * See the LICENSE file for the full copyright and license declaration.
 * 
 * ---------------------------------------------------------------------
 * 
 * Configuration related to OSC
 * This file defines only the configuration objects
 * The runtime handling of bindings and event processing is elsewhere
 * and to be ported.
 */

#include "OscConfig.h"

//////////////////////////////////////////////////////////////////////
//
// OscConfig
//
//////////////////////////////////////////////////////////////////////

PUBLIC OscConfig::OscConfig()
{
	init();
}

PRIVATE void OscConfig::init()
{
	mInputPort = 0;
	mOutputHost = NULL;
	mOutputPort = 0;
	mBindings = NULL;
    mWatchers = NULL;
    // this is only for XML parsing, get rid of it
    mError[0] = 0;
}

PUBLIC OscConfig::~OscConfig()
{
	delete mOutputHost;
	delete mBindings;
    delete mWatchers;
}

PUBLIC const char* OscConfig::getError()
{
    return (mError[0] != 0) ? mError : NULL;
}

PUBLIC int OscConfig::getInputPort()
{
	return mInputPort;
}

PUBLIC void OscConfig::setInputPort(int i)
{
	mInputPort = i;
}

PUBLIC const char* OscConfig::getOutputHost()
{
	return mOutputHost;
}

PUBLIC void OscConfig::setOutputHost(const char* s)
{
	delete mOutputHost;
	mOutputHost = CopyString(s);
}

PUBLIC int OscConfig::getOutputPort()
{
	return mOutputPort;
}

PUBLIC void OscConfig::setOutputPort(int i)
{
	mOutputPort = i;
}

PUBLIC OscBindingSet* OscConfig::getBindings()
{
	return mBindings;
}

PUBLIC OscWatcher* OscConfig::getWatchers()
{
    return mWatchers;
}

//////////////////////////////////////////////////////////////////////
//
// OscBindingSet
//
//////////////////////////////////////////////////////////////////////

PUBLIC OscBindingSet::OscBindingSet()
{
	init();
}

PRIVATE void OscBindingSet::init()
{
	mNext = NULL;
    mName = NULL;
    mComments = NULL;
    mActive = false;
	mInputPort = 0;
	mOutputHost = NULL;
	mOutputPort = 0;
	mBindings = NULL;
}

PUBLIC OscBindingSet::~OscBindingSet()
{
	OscBindingSet *el, *next;

    delete mName;
    delete mComments;
	delete mOutputHost;
	delete mBindings;

	for (el = mNext ; el != NULL ; el = next) {
		next = el->getNext();
		el->setNext(NULL);
		delete el;
	}
}

PUBLIC OscBindingSet* OscBindingSet::getNext()
{
    return mNext;
}

PUBLIC void OscBindingSet::setNext(OscBindingSet* s)
{
    mNext = s;
}

PUBLIC const char* OscBindingSet::getName()
{
    return mName;
}

PUBLIC void OscBindingSet::setName(const char* s)
{
    delete mName;
    mName = CopyString(s);
}

PUBLIC const char* OscBindingSet::getComments()
{
    return mComments;
}

PUBLIC void OscBindingSet::setComments(const char* s)
{
    delete mComments;
    mComments = CopyString(s);
}

PUBLIC bool OscBindingSet::isActive()
{
    // ignore the active flag for now until we have a UI
    return true;
}

PUBLIC void OscBindingSet::setActive(bool b)
{
    mActive = b;
}

PUBLIC int OscBindingSet::getInputPort()
{
	return mInputPort;
}

PUBLIC void OscBindingSet::setInputPort(int i)
{
	mInputPort = i;
}

PUBLIC const char* OscBindingSet::getOutputHost()
{
	return mOutputHost;
}

PUBLIC void OscBindingSet::setOutputHost(const char* s)
{
	delete mOutputHost;
	mOutputHost = CopyString(s);
}

PUBLIC int OscBindingSet::getOutputPort()
{
	return mOutputPort;
}

PUBLIC void OscBindingSet::setOutputPort(int i)
{
	mOutputPort = i;
}

PUBLIC Binding* OscBindingSet::getBindings()
{
	return mBindings;
}

//////////////////////////////////////////////////////////////////////
//
// OscWatcher
//
//////////////////////////////////////////////////////////////////////

PUBLIC OscWatcher::OscWatcher()
{
    init();
}

void OscWatcher::init()
{
    mNext = NULL;
    mPath = NULL;
    mName = NULL;
    mTrack = 0;
}

PUBLIC OscWatcher::~OscWatcher()
{
    delete mName;
    delete mPath;
    
	OscWatcher *el, *next;
	for (el = mNext ; el != NULL ; el = next) {
		next = el->getNext();
		el->setNext(NULL);
		delete el;
	}
}

PUBLIC OscWatcher* OscWatcher::getNext()
{
    return mNext;
}

PUBLIC void OscWatcher::setNext(OscWatcher* w)
{
    mNext = w;
}

PUBLIC const char* OscWatcher::getPath()
{
    return mPath;
}

PUBLIC void OscWatcher::setPath(const char* path)
{
    delete mPath;
    mPath = CopyString(path);
}

PUBLIC const char* OscWatcher::getName()
{
    return mName;
}

PUBLIC void OscWatcher::setName(const char* name)
{
    delete mName;
    mName = CopyString(name);
}

PUBLIC int OscWatcher::getTrack()
{
    return mTrack;
}

PUBLIC void OscWatcher::setTrack(int t)
{
    mTrack = t;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

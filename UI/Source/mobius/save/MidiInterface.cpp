// abstract implementation of MidiInterface extended by JuceMidiInterface
// formerly contained the default implementation based on MidiEnv, that
// was removed but kept AbstractMidiInterface, not really much in here now
//
// cleaned up includes
// remove PUBLIC and PRIVATE
// changed NULL to nullptr

/*
 * Copyright (c) 2010 Jeffrey S. Larson  <jeff@circularlabs.com>
 * All rights reserved.
 * See the LICENSE file for the full copyright and license declaration.
 * 
 * ---------------------------------------------------------------------
 * 
 * Implementation of the AbstractMidiInterface and various concrete
 * support classes for the MIDI abstraction layer.
 *
 * Originally I was going to have a MacMidiInterface and WinMidiInterface
 * but now that the MidiEnv model does the device encapsulation we
 * can share the implementation.  As such we don't really need
 * AbstractMidiInterface any more.
 */

// for strlen
#include <string.h>

//#include <stdio.h>
//#include <string.h>

//#include "Port.h"
//#include "Util.h"
//#include "List.h"
//#include "Trace.h"
//#include "Thread.h"

//#include "MidiEnv.h"
//#include "MidiPort.h"
//#include "MidiTimer.h"
//#include "MidiInput.h"
//#include "MidiOutput.h"
//#include "MidiListener.h"

#include "MidiInterface.h"

//////////////////////////////////////////////////////////////////////
//
// AbstractMidiInterface
//
//////////////////////////////////////////////////////////////////////

AbstractMidiInterface::AbstractMidiInterface()
{
	mListener = nullptr;
	mClockListener = nullptr;

	mInputSpec = nullptr;
	mOutputSpec = nullptr;
	mThroughSpec = nullptr;

	mInputError[0] = 0;
	mOutputError[0] = 0;
	mThroughError[0] = 0;
	mError[0] = 0;
}

AbstractMidiInterface::~AbstractMidiInterface()
{
	delete mInputSpec;
	delete mOutputSpec;
	delete mThroughSpec;
}


void AbstractMidiInterface::setListener(MidiEventListener* l)
{
	mListener = l;
}

void AbstractMidiInterface::setClockListener(MidiClockListener* h)
{
	mClockListener = h;
}

const char* AbstractMidiInterface::getInput()
{
	return mInputSpec;
}

const char* AbstractMidiInterface::getInputError()
{
	return (strlen(mInputError) > 0) ? mInputError : nullptr;
}

const char* AbstractMidiInterface::getOutput()
{
	return mOutputSpec;
}

const char* AbstractMidiInterface::getOutputError()
{
	return (strlen(mOutputError) > 0) ? mOutputError : nullptr;
}

const char* AbstractMidiInterface::getThrough()
{
	return mThroughSpec;
}

const char* AbstractMidiInterface::getThroughError()
{
	return (strlen(mThroughError) > 0) ? mThroughError : nullptr;
}

const char* AbstractMidiInterface::getLastError()
{
	return mError;
}

void AbstractMidiInterface::printStatistics()
{
}

void AbstractMidiInterface::printEnvironment()
{
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

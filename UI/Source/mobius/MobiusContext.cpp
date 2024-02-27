// factored out of Mobius.cpp
// NULL to nullptr, PRIVATE etc

// string stuff
#include "../util/Util.h"

// formerly in Mobius.h
#include "AudioInterface.h"
#include "MidiInterface.h"

#include "MobiusContext.h"

/****************************************************************************
 *                                                                          *
 *   							MOBIUS CONTEXT                              *
 *                                                                          *
 ****************************************************************************/

MobiusContext::MobiusContext()
{
	mCommandLine = nullptr;
	mInstallationDirectory = nullptr;
	mConfigurationDirectory = nullptr;
    mConfigFile = nullptr;
	mAudio = nullptr;
	mMidi = nullptr;
    mHostMidi = nullptr;
    mPlugin = false;
	mDebugging = false;
}

MobiusContext::~MobiusContext()
{
	delete mCommandLine;
	delete mInstallationDirectory;
	delete mConfigurationDirectory;
    delete mConfigFile;
}

void MobiusContext::setCommandLine(const char* s)
{
	delete mCommandLine;
	mCommandLine = CopyString(s);
}

const char* MobiusContext::getCommandLine()
{
	return mCommandLine;
}

void MobiusContext::setPlugin(bool b)
{
	mPlugin = b;
}

bool MobiusContext::isPlugin()
{
	return mPlugin;
}

void MobiusContext::setDebugging(bool b)
{
	mDebugging = b;
}

bool MobiusContext::isDebugging()
{
	return mDebugging;
}

void MobiusContext::setInstallationDirectory(const char* s)
{
	delete mInstallationDirectory;
	mInstallationDirectory = CopyString(s);
}

const char* MobiusContext::getInstallationDirectory()
{
	return mInstallationDirectory;
}

void MobiusContext::setConfigurationDirectory(const char* s)
{
	delete mConfigurationDirectory;
	mConfigurationDirectory = CopyString(s);
}

const char* MobiusContext::getConfigurationDirectory()
{
	return mConfigurationDirectory;
}

void MobiusContext::setConfigFile(const char* s)
{
	delete mConfigFile;
	mConfigFile = CopyString(s);
}

const char* MobiusContext::getConfigFile()
{
	return mConfigFile;
}

void MobiusContext::setAudioInterface(AudioInterface* a)
{
	mAudio = a;
}

AudioInterface* MobiusContext::getAudioInterface()
{
	return mAudio;
}

void MobiusContext::setMidiInterface(MidiInterface* m)
{
	mMidi = m;
}

MidiInterface* MobiusContext::getMidiInterface()
{
	return mMidi;
}

void MobiusContext::setHostMidiInterface(HostMidiInterface* m)
{
	mHostMidi = m;
}

HostMidiInterface* MobiusContext::getHostMidiInterface()
{
	return mHostMidi;
}

/**
 * Kludge to look for a few special command line args for debugging.
 * Normally the only command line arg is the name of a config file.
 * Really need to have a real command line parser.
 */
void MobiusContext::parseCommandLine()
{
	mDebugging = StringEqualNoCase(mCommandLine, "debugging");
}

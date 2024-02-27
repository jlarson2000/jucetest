// factored out of MobiusInterface.h
// this is the original

#pragma once

/**
 * Encapsulates a few things about the runtime environment that
 * are passed into the Mobius engine.
 * 
 * Do not to depend on qwin/Context here.
 *
 * Might want to evolve this into a package of OS specific methods, 
 * sort of like the util functions only encapsulated?
 *
 * One of these must be built by the application that wraps the
 * Mobius engine, currently there are three: 
 * Windows standalone (WinMain), Mac standalone (MacMain), 
 * VST or AU plugin (MobiusPlugin).
 */
class MobiusContext {

  public:

	MobiusContext();
	~MobiusContext();

	void setCommandLine(const char* s);
	const char* getCommandLine();

	void setInstallationDirectory(const char* s);
	const char* getInstallationDirectory();

	void setConfigurationDirectory(const char* s);
	const char* getConfigurationDirectory();

	void setConfigFile(const char* s);
	const char* getConfigFile();

	void setAudioInterface(class AudioInterface* a);
	class AudioInterface* getAudioInterface();

	void setMidiInterface(class MidiInterface* mi);
	class MidiInterface* getMidiInterface();

	void setHostMidiInterface(class HostMidiInterface* mi);
	class HostMidiInterface* getHostMidiInterface();

	void setDebugging(bool b);
	bool isDebugging();

    void setPlugin(bool b);
    bool isPlugin();

	void parseCommandLine();

  private:

    /**
     * The command line arguments, set when Mobius is run from
     * the command line.
     */
	char *mCommandLine;

    /**
     * The directory where Mobius is installed.
     * On Mac this is derived from the application package directory,
     * on Windows it is stored in the registry.
     */
	char* mInstallationDirectory;

    /**
     * The directory where the Mobius configuration files are stored.
     * On Windows this will be the same as mInstallationDirectory,
     * On Mac this is normally /Library/Application Support/Mobius. 
     */
	char* mConfigurationDirectory;

    /**
     * This full path name of the mobius.xml file.
     * This is not set when the context is created, it is set by
     * Mobius after it locates the mobius.xml file from one of the
     * above directories.  This is only used by the UI so that it
     * can locate the ui.xml file which by convention will always be
     * taken from the same directory as mobius.xml.
     */
    char* mConfigFile;
    
    /**
     * The object providing audio streams.
     * When running standalone this will be a platform-specific class
     * that interact directly with the audio devices.
     * When running as a plugin this will be a proxy to the host
     * application's audio buffers.
     */
	class AudioInterface* mAudio;

    /**
     * The object providing access to MIDI devices.
     * When running standalone this will be a platform-specific class
     * that interacts directly with the MIDI devices.  
     */
	class MidiInterface* mMidi;

    /**
     * The object providing access to MIDI devices when running as a plugin.
     * This is a temporary kludge, see comments in HostMidiInterface for
     * more information.
     */
    class HostMidiInterface* mHostMidi;

    /**
     * Flag set if we're a plugin.
     */
    bool mPlugin;

    /**
     * Special flag that when true enables some unspecified debugging
     * behavior.  Should only be used by Mobius developers.
     */
	bool mDebugging;

};

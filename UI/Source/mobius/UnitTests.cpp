/**
 * Code related to running the unit tests.
 * This does some sensitive reach-arounds to Mobius without
 * going through KernelCommunicator so be careful...
 *
 * There is one singleton UnitTests object containing code to
 * implement various unit testing features that would not normally be active.
 * It is a part of MobiusShell.
 * 
 * The engine may be placed in "unit test mode" during which it forces the
 * installation of a Preset and a Setup with a known configuration, sets
 * a few global parameters, loads a set of Samples, and loads a set of Scripts.
 * The scripts may in turn add buttons to the UI.  This configuration takes the
 * place of the copy of MobiusConfig managed by the MobiusKernel and shared
 * with the Mobius core.
 *
 * Note that it does not replace the config managed by MobiusShell.  There was no
 * good reason for that other than it wasn't immediately necessary and saves some work
 * but might want to do that for consistency.  That also gives us a way to restore
 * the original config when unit test mode is canceled.
 *
 * This modified configuration is only active in memory, it is not saved
 * on the file system, and it will be lost if you edit the configuration in the UI
 * and push a new MobiusConfig down.  That effectively disables unit test mode.
 *
 * Unit test mode can be enabled in one of two ways.  First, by binding an action in
 * the UI to the UnitTestMode function.  This would typically be bound to a UI
 * action button.  Second, it may be enabled from a script with the old UnitTestSetup
 * statement.  This was a built-in script statement and not a Function.  Now that
 * we have the Function it could be replaced.
 *
 * While in unit test mode, the behavior of the SaveCapture, SaveLoop, and LoadLoop
 * functions are different.  It will load and save files relative to the "unit test root"
 * rather than normal installation root.  This is used so that the large library
 * of unit test sample and captured recording files can be located in a directory
 * apart from the main installation.  This file redirect is handled by KernelEventHandler.
 *
 * Configuration of samples and scripts is done with a configuration "overlay".  This
 * is an xml file found in the unit test root named mobius-overlay.xml.  It contains
 * a sparse MobiusConfig object with just the information that the tests need, primarily
 * this is a SampleConfig, a ScriptConfig, and a few global parameters.  When unit
 * test mode is activated, this file is ready and it's contents are merged with
 * the full MobiusConfig read from the mobius.xml file from the installation root.
 *
 * There is not currently a way to cancel unit test mode without restarting.
 * The test scripts call UnitTestSetup in every script, so we only do setup once
 * and then assume it remains stable.  May need to do partial reinitialization
 * of the Preset and the Setup but the samples and scripts can be reused.
 *
 * The same applies to the UnitTestMode function from the UI, though there it would
 * be convenient for the function to act as a toggle turning it on and off.
 * 
 */

#include "../util/Util.h"
#include "../util/Trace.h"

#include "../model/MobiusConfig.h"
#include "../model/Preset.h"
#include "../model/Setup.h"
#include "../model/SampleConfig.h"
#include "../model/ScriptConfig.h"
#include "../model/XmlRenderer.h"

#include "Audio.h"
#include "AudioFile.h"
#include "SampleReader.h"
#include "SampleManager.h"

#include "MobiusContainer.h"
#include "MobiusShell.h"
#include "MobiusKernel.h"

#include "core/Mobius.h"
#include "core/Mapper.h"

#include "WaveFile.h"

#include "UnitTests.h"

// implementation of the static declaration
// the object will be a member object of MobiusShell
UnitTests* UnitTests::Instance = nullptr;

/**
 * This one is unusual in that we want it to be a singleton but don't
 * have the usual factory method.
 * Could just as well have a static objecte and use that.
 */
UnitTests::UnitTests(MobiusShell* argShell)
{
    shell = argShell;

    if (Instance != nullptr) {
        Trace(1, "UnitTests: duplicate instance encountered\n");
    }
    else {
        Instance = this;
    }
}

UnitTests::~UnitTests()
{
    // this is normally a static object on MobiusShell
    Instance = nullptr;
}

//////////////////////////////////////////////////////////////////////
//
// Setup
//
//////////////////////////////////////////////////////////////////////

/**
 * The entry point for the UnitTestSetup script statement.
 * Enable unit test mode if it is not already on, and leave it on.
 * We will be here through a KernelEvent sent from the script interpreter.
 *
 * The core will be left in a state of GlobalReset so we can modify
 * it directly.
 */
void UnitTests::scriptSetup(KernelEvent* e)
{
    // only do this once so scripts can call it multiple times
    if (!enabled)
      setup();
}

/**
 * The entry point for the UnitTestMode function.
 * Here we're comming from a bound action in the UI.
 * May want this to behave as a toggle.
 *
 * The core is not necessarily in a state of GlobalReset.  If it isn't
 * we have two options:
 *
 *    - force global reset, and wait for the kernel to handle that request
 *    - return an error to the UI and make them do it
 *    - live dangerously and proceed
 *
 * Forcing global reset would require the scheduling of a UIAction and a sleep
 * while we wait for the kernel to respond.  Doable but annoying.  A warning
 * message would be the safest thing.
 *
 * Living dangeriously is my business, and since this is only for my unit testing
 * I'll trust you to do the right thing.
 */
void UnitTests::actionSetup(UIAction* a)
{
    // may want this to be a toggle?
    if (!enabled)
      setup();
}

/**
 * Names that used to live somewhere and need someplace better
 */
#define UNIT_TEST_SETUP_NAME "UnitTestSetup"
#define UNIT_TEST_PRESET_NAME "UnitTestPreset"

/**
 * Prepare the system for unit tests
 *
 * We are operating at the shell level, either in the UI thread or
 * the maintenance thread.
 *
 * To avoid KernelCommunicator and conflicts with the audio thread
 * the system must be in a state of GlobalReset allowing us to directly
 * modify objects in the kernel/core safely.  We don't normally do this
 * but saves a lot of communication headaches just for the unit tests.
 */
void UnitTests::setup()
{
    // this starts KernelEventHandler doing file system redirects
    enabled = true;

    // now we need to dive down and fuck with the core's MobiusConfig
    MobiusKernel* kernel = shell->getKernel();
    MobiusConfig* kernelConfig = kernel->getMobiusConfig();
    
    // special Preset and Setup
    installPresetAndSetup(kernelConfig);

    MobiusConfig* overlay = readConfigOverlay();
    if (overlay != nullptr) {
        installOverlayParameters(kernelConfig, overlay);

        // todo: rather than just replace the set of samples/scripts
        // with what is in the overlay, could merge them with
        // the active configs so we don't lose anything

        // load and install the samples, note the use
        // of the special "safe mode"
        // shell does the hard work I want to keep in one place
        SampleManager* manager = shell->loadSamples(overlay->getSampleConfig());
        shell->sendSamples(manager, true);

        // load and install the scripts
        Scriptarian* scriptarian = shell->loadScripts(overlay->getScriptConfig());
        shell->sendScripts(scriptarian, true);

        // if we decide to defer DynamicConfigChanged notification
        // this is where you would do it
        // hmm, since all the send() functions do is updateDynamicConfig
        // and send notifications could just do that here and
        // avoid the kludgey "saveMode" flag

        delete overlay;
    }
}

/**
 * Initialize the unit test setup and preset within a config object.
 * The MobiusConfig here is the one actually installed in MobiusKernel
 * and Mobius.
 */
void UnitTests::installPresetAndSetup(MobiusConfig* config)
{
    // boostrap a preset
    Preset* p = config->getPreset(UNIT_TEST_PRESET_NAME);
    if (p != NULL) {
        p->reset();
    }
    else {
        p = new Preset();
        p->setName(UNIT_TEST_PRESET_NAME);
        config->addPreset(p);
    }
    config->setDefaultPresetName(UNIT_TEST_PRESET_NAME);

    // boostrap a setup
    Setup* s = config->getSetup(UNIT_TEST_SETUP_NAME);
    if (s != NULL) {
        s->reset(p);
    }
    else {
        s = new Setup();
        s->setName(UNIT_TEST_SETUP_NAME);
        s->reset(p);
        config->addSetup(s);
    }
    config->setStartingSetupName(UNIT_TEST_SETUP_NAME);

    // this will propagate the changes to the tracks
    MobiusKernel* kernel = shell->getKernel();
    Mobius* mobius = kernel->getCore();
    mobius->setActiveSetup(UNIT_TEST_SETUP_NAME);
}

/**
 * Read the unit test overlay configuration.
 * Within the SampleConfig and ScriptConfig, adjust the
 * relative path names to be absolute in wherever the
 * unit test root is.
 */
MobiusConfig* UnitTests::readConfigOverlay()
{
    MobiusConfig* overlay = nullptr;
    
    juce::File root = getTestRoot();
    juce::File file = root.getChildFile("mobius-overlay.xml");
    if (file.existsAsFile()) {
        juce::String xml = file.loadFileAsString();
        XmlRenderer xr;
        overlay = xr.parseMobiusConfig(xml.toUTF8());

        // resolve sample paths
        SampleConfig* samples = overlay->getSampleConfig();
        if (samples != nullptr) {
            Sample* sample = samples->getSamples();
            while (sample != nullptr) {
                const char* path = sample->getFilename();
                // these are expected to be relative to UnitTestRoot
                // could be smarter about absolute paths or $ references
                // but don't really need that yet
                file = root.getChildFile(path);
                if (!file.existsAsFile()) {
                    Trace(1, "Unable to resolve sample file %s\n",
                          file.getFullPathName().toUTF8());
                }
                else {
                    sample->setFilename(file.getFullPathName().toUTF8());
                }
                sample = sample->getNext();
            }
        }

        // same for scripts
        ScriptConfig* scripts = overlay->getScriptConfig();
        if (scripts != nullptr) {
            ScriptRef* script = scripts->getScripts();
            while (script != nullptr) {
                // weirdly doesn't use the same method name
                const char* path = script->getFile();
                file = root.getChildFile(path);
                if (!file.existsAsFile()) {
                    Trace(1, "Unable to resolve script file %s\n",
                          file.getFullPathName().toUTF8());
                }
                else {
                    script->setFile(file.getFullPathName().toUTF8());
                }
                script = script->getNext();
            }
        }
    }

    return overlay;
}

/**
 * Overwrite selected global parameters from the overlay config
 * into Kernel's config.  Not that we do NOT install the
 * full SampleConfig and ScriptConfig here, those have
 * a more complex installation process and are handled elsewhere.
 *
 * We could also be doing the special Preset and Setup this way
 * rather than building them in memory and forcing them in.
 */
void UnitTests::installOverlayParameters(MobiusConfig* dest, MobiusConfig* overlay)
{
    // just hard code them for now, could iterate over the UIParameter
    // Instances to get anything that is found
    dest->setInputLatency(overlay->getInputLatency());
    dest->setOutputLatency(overlay->getOutputLatency());
}
        
//////////////////////////////////////////////////////////////////////
//
// Files
//
//////////////////////////////////////////////////////////////////////

/**
 * Derive where the root of the unit test files are.
 * For initial testing, I'm wiring it under the source tree
 * which won't last long.
 */
juce::File UnitTests::getTestRoot()
{
    MobiusContainer* cont = shell->getContainer();
    
    juce::File root = cont->getRoot();

    // hack, if we're using mobius-redirect and have already
    // redirected to a directory named "unittest" don't add
    // an additional subdir
    juce::String last = root.getFileNameWithoutExtension();
    if (last != juce::String("unittest"))
      root = root.getChildFile("unittest");

    return root;
}

/**
 * Given a base file name from a script, locate the full
 * path name to that file from the "results" folder
 * of the unit test root where the result files will be written.
 */
juce::File UnitTests::getOutputFile(const char* name)
{
    juce::File file = getTestRoot().getChildFile("results").getChildFile(name);

    // tests don't usually have an extension so add it
    // assuming a .wav file, will need more when we start dealing with projects
    if (file.getFileExtension().length() == 0)
      file = file.withFileExtension(".wav");

    return file;
}

/**
 * Given a base file name from a script, locate the full
 * path name to that file from the "master" folder
 * of the unit test root where the comparision files are read
 *
 * We have historically called this "expected" rather than
 * "master" which I ususlly say in comments.
 */
juce::File UnitTests::getInputFile(const char* name)
{
    juce::File file = getTestRoot().getChildFile("expected").getChildFile(name);

    // tests don't usually have an extension so add it
    // assuming a .wav file, will need more when we start dealing with projects
    if (file.getFileExtension().length() == 0)
      file = file.withFileExtension(".wav");

    return file;
}

/**
 * Get the path to where the file used with SaveCapture
 * (in scripts the SaveAudioRecording statement)
 * is written.
 *
 * Scripts should always pass a name so we don't default
 * to "capture" like we do for user initiated saves.
 */
juce::File UnitTests::getSaveCaptureFile(KernelEvent* e)
{
    const char* name = e->arg1;

    if (strlen(name) == 0) {
        Trace(1, "UnitTests: SaveCapture file not specified, defaulting\n");
        name = "testcapture";
    }
    
    return getOutputFile(name);
}

juce::File UnitTests::getSaveLoopFile(KernelEvent* e)
{
    const char* name = e->arg1;

    if (strlen(name) == 0) {
        Trace(1, "UnitTests: SaveLoop file not specified, defaulting\n");
        name = "testloop";
    }
    
    return getOutputFile(name);
}

/**
 * Locate a file to be used in one of the diff handlers.
 */
juce::File UnitTests::getDiffFile(const char* name, bool master)
{
    juce::File file;

    if (master)
      file = getTestRoot().getChildFile("expected").getChildFile(name);
    else
      file = getTestRoot().getChildFile("results").getChildFile(name);
      
    // tests don't usually have an extension so add it
    // assuming a .wav file, will need more when we start dealing with projects
    if (file.getFileExtension().length() == 0)
      file = file.withFileExtension(".wav");
    
    return file;
}

//////////////////////////////////////////////////////////////////////
//
// Differencing
//
//////////////////////////////////////////////////////////////////////

/**
 * Slightly modified version of the old diff utility for .wav files.
 * juce::File has a binary diff method, but I'd like more info on the
 * specific sampel that is different and where it is so we can look
 * in a wave editor to see what's wrong.
 *
 */
void UnitTests::diffAudio(KernelEvent* e)
{
    const char* name1 = e->arg1;
    const char* name2 = e->arg2;
    bool reverse = StringEqualNoCase(e->arg3, "reverse");

    // the input file
    juce::File file1 = getDiffFile(name1, false);

    // output file  is optional, and if not set uses the same base file name
    // as the input file, but reads from a different directory
    juce::File file2;
    if (strlen(name2) == 0)
      file2 = getDiffFile(name1, true);
    else
      file2 = getDiffFile(name2, true);

    // hmm, getFullPathName().toUTF() seems to become unstable
    // after you call anything else on the FIle, like existsAsFile
    // so can't capture those early, have to wait until needed
    // and not expect them to live long

    if (!file1.existsAsFile()) {
        const char* path = file1.getFullPathName().toUTF8();
        Trace(1, "Diff file not found: %s\n", path);
    }
    else if (!file2.existsAsFile()) {
        // expected file not there, could bootstrap it?
        const char* path = file2.getFullPathName().toUTF8();
        Trace(1, "Diff file not found: %s\n", path);
    }
    else if (file1.getSize() != file2.getSize()) {
        const char* path1 = file1.getFullPathName().toUTF8();
        const char* path2 = file2.getFullPathName().toUTF8();
        Trace(1, "Diff files differ in size: %s, %s\n", path1, path2);
    }
    else {
        // reading files requires a pool
        AudioPool* pool = shell->getAudioPool();
        Audio* a1 = AudioFile::read(file1, pool);
        Audio* a2 = AudioFile::read(file2, pool);
        
        const char* path1 = file1.getFullPathName().toUTF8();
        const char* path2 = file2.getFullPathName().toUTF8();

        if (a1->getFrames() != a2->getFrames()) {
            Trace(1, "Diff file frame counts differ %s, %s\n", path1, path2);
            Trace(1, "  Frames %ld %ld\n", (long)a1->getFrames(), (long)a2->getFrames());
        }
        else if (a1->getChannels() != 2) {
            Trace(1, "Diff file channel count not 2: %s\n", path1);
        }
        else if (a2->getChannels() != 2) {
            Trace(1, "Diff file channel count not 2: %s\n", path2);
        }
        else {
            // formerly checked channel counts, which were always 2 and in
            // newer code may be unset, so just ass
            int channels = 2;
            AudioBuffer b1;
            float f1[MaxAudioChannels];
            b1.buffer = f1;
            b1.frames = 1;
            b1.channels = channels;

            AudioBuffer b2;
            float f2[MaxAudioChannels];
            b2.buffer = f2;
            b2.frames = 1;
            b2.channels = channels;

            bool stop = false;
            int psn2 = (reverse) ? a2->getFrames() - 1 : 0;

            bool different = false;
            bool checkFloats = false;
            for (int i = 0 ; i < a1->getFrames() && !different ; i++) {

                memset(f1, 0, sizeof(f1));
                memset(f2, 0, sizeof(f2));
                a1->get(&b1, i);
                a2->get(&b2, psn2);
			
                for (int j = 0 ; j < channels && !different ; j++) {
                    // sigh, due to rounding errors it is 
                    // impossible to reliably assume that
                    // x + y - y = x with floats, so coerce
                    // to an integer to do the comparion
                    if (checkFloats) {
                        if (f1[j] != f2[j]) {
                            // sigh, don't have Trace signatures that use floats
                            char msg[1024];
                            sprintf(msg, "WARNING: files differ at frame %d: %f %f: %s, %s\n",
                                    i, f1[j], f2[j], name1, name2);
                            Trace(1, msg);
                        }
                    }

                    // 24 bit is too much, but 16 is too small
                    // 16 bit signed (2^15)
                    //float precision = 32767.0f;
                    // 24 bit signed (2^23)
                    //float precision = 8388608.0f;
                    // 20 bit
                    float precision = 524288.0f;
								
                    int i1 = (int)(f1[j] * precision);
                    int i2 = (int)(f2[j] * precision);

                    if (i1 != i2) {
                        char msg[1024];
                        sprintf(msg, "WARNING: files differ at frame %d: %d %d: %s, %s\n",
                                i, i1, i2, name1, name2);
                        Trace(1, msg);
                        different = true;
                    }
                }

                if (reverse)
                  psn2--;
                else
                  psn2++;
            }
        }
        
        // deleting returns them to the pool
        delete a1;
        delete a2;
    }
}

/**
 * I think this was used to difference Project files.
 * Old code did a binary comparision even though it was text.
 * Use juce::File for this since these are far less touchy than
 * Audio files.
 */
void UnitTests::diffText(KernelEvent* e)
{
    const char* name1 = e->arg1;
    const char* name2 = e->arg2;

    // the input file
    juce::File file1 = getDiffFile(name1, false);

    // output file  is optional, and if not set uses the same base file name
    // as the input file, but reads from a different directory
    juce::File file2;
    if (strlen(name2) == 0)
      file2 = getDiffFile(name1, true);
    else
      file2 = getDiffFile(name2, true);

    // see comments above for why getFullPathName is not stable

    if (!file1.existsAsFile()) {
        const char* path = file1.getFullPathName().toUTF8();
        Trace(1, "Diff file not found: %s\n", path);
    }
    else if (!file2.existsAsFile()) {
        // expected file not there, could bootstrap it?
        const char* path = file2.getFullPathName().toUTF8();
        Trace(1, "Diff file not found: %s\n", path);
    }
    else if (file1.getSize() != file2.getSize()) {
        const char* path1 = file1.getFullPathName().toUTF8();
        const char* path2 = file2.getFullPathName().toUTF8();
        Trace(1, "Diff files differ in size: %s, %s\n", path1, path2);
    }
    else {
        // old tool did a byte-by-byte comparison and printed
        // the byte number where they differed, hasIdenticalContent
        // just returns true/false, go with that unless you need it
        if (!file1.hasIdenticalContentTo(file2)) {
            const char* path1 = file1.getFullPathName().toUTF8();
            const char* path2 = file2.getFullPathName().toUTF8();
            Trace(1, "Diff files are not identical: %s\n", path1, path2);
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

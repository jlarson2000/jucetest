/**
 * Code related to running the unit tests.
 * This does some sensitive reach-arounds to Mobius without
 * going through KernelCommunicator so be careful...
 * 
 * The structure of the old test direcrory was:
 * 
 *     ./            script source files and output .wav files from the tests
 *     ./expected    correct .wav files for comparision
 *
 * There was a global UnitTestsParameter that I think was used to specify
 * the location of the scripts and the master files, with test runtime results
 * being deposited in the CWD.
 *
 * Whatever we did, I'd like it to work so that the unit test scripts and
 * master comparision files live in a different Git repository as they did before.
 * To avoid CWD dependencies, this directory will contain test run results.
 *
 *    ./results
 *
 * This impacts the behavior of any Save and Load functions that might
 * be of general use outside the unit tests, since those need to have
 * more control over where the files go.
 *
 * By default these functions assume files are in the root directory
 * which will either be the installation directory, or an OS specific
 * folder used to contain user settings.
 *
 * With mobius-redirect we can now change the entire root to go somewhere
 * else, but wondering if there should be a "unit test mode" or a parameter
 * to point to where the unit tests are so we don't have to use
 * redirects just to run the tests.  Because mobius.xml is accessed so
 * early, and the unit tests need their own stable mobius.xml, there
 * are advantages to using mobius-redirect.  Still it would be nice
 * if the redirect could go to a folder under the main source tree for
 * easier maintenance, but still have the test files located somewhere else.
 *
 * The scripts are small so I don't mind having them in the main source tree
 * but the audio files are massive and need to go somewhere else.
 *
 * Scripts need a way to say "I'm a unit test" which changes the behavior
 * of these functions:
 *
 *     SaveAudio, SaveLoop, SaveProject
 *       Unless the file path is absolute it will save the file
 *       under $UNIT_TEST_ROOT/results
 *
 *     Load
 *        read the file from $UNIT_TEST_ROOT/expected
 *
 *     Diff, DiffAudio
 *       first file is in $UNIT_TEST_ROOT/results
 *       second file is in $UNIT_TEST_ROOT/expected
 *
 * Test scripts must call the UnitTestSetup function, which isn't really
 * a function right now, it's a special statement wired in to the script language.
 * This will set a flag in Mobius that can be tested here to
 * see whether the special file handling needs to be done.
 *
 * Setup Notes
 * 
 * This used to bootstrap a Preset and Setup with a stable
 * configuration, but I'm moving toward just using mobius-redirect for that
 * // Think about not requiring mobius-redirect just to run tests.
 * UnitTestSetup can just as easilly force the load of a MobiusConfig
 * from anywhere without needing to redirect for general use.
 * 
 * The tests need to know the location of the "unit test root" which
 * is different from the standard root and is where the test result
 * files go and the expected files live.  There are a few ways this could work
 * 
 * UnitTestSetup argument
 *    make the tests pass the absolute path to the directory
 *    inconvenience for the test writer and makes different machines harder
 * 
 * Root relative
 * 
 *   test files are expected to be relative to the standard system root
 *   putting it under the root is awkward for development since Git sees
 *   them and you have to add .gitignores to make it stop getting excited
 * 
 * These are going to be in a different Git repository so unwinding
 *   out of the development tree would look like this:
 * 
 *   Typical dev directory
 *      c:/dev/mobiusrepo/mobius/Source
 *   Relative test directory
 *      c:/dev/mobiustest
 * 
 * This doesn't work so well if you want to run tests from a normal
 * isntallation rather than the dev tree.  The root location the
 * is something like /Library/Application Support on Mac
 * and c:/Program Files?...  on Windows and we don't want to mess
 * with putting test files there.
 */

#include "../util/Util.h"
#include "../util/Trace.h"

#include "../model/MobiusConfig.h"
#include "../model/Preset.h"
#include "../model/Setup.h"
#include "../model/SampleConfig.h"
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

// implementation of static declaration
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
// This is the ugliest part of all this and needs serious thought
//
//////////////////////////////////////////////////////////////////////

/**
 * Names that used to live somewhere and need someplace better
 */
#define UNIT_TEST_SETUP_NAME "UnitTestSetup"
#define UNIT_TEST_PRESET_NAME "UnitTestPreset"

/**
 * Prepare the system for unit tests
 * Reachable only from the UnitTestSetup script statement.
 *
 * Originaly this was done synchronously within the core but with
 * deferred sample loading it needed to be pushed up to the shell
 * to do file handling safely.
 *
 * That now has a reverse problem where we need to do something
 * to the core from up here and don't want to mess with KernelCommunicator
 * message passing.  In particular I need the core to be in a state
 * of GlobalReset so we can do things to it without messing up the audio thread.
 * UnitTestSetupt will do a Mobius::globalReset on the way out, so when
 * we reach this event handler it can be assumed the kernel is quiet.
 * 
 * In the olden times, this used to bootstrap a special Setup and Preset
 * so the tests would have something stable to use.  We can now use
 * mobius-redirect to accomplish that on startup, but I'd still like
 * to be able to run in either mode without a redirect.
 *
 * We'll reach down and do that, but needs more thought.
 *
 */
void UnitTests::setup(KernelEvent* e)
{
    // this is the main thing we need to do to get KernelEventHandler
    // to redirect file handling to us so we can use alternate locations
    // note that there is no way to clear this after it is set which
    // is unfortunate, would like this to be restored automatcially when
    // the test script terminates, or use a KernelEvent arg to turn it on and off
    enabled = true;

    // now we need to dive down and fuck with the core's MobiusConfig
    MobiusKernel* kernel = shell->getKernel();
    MobiusConfig* kernelConfig = kernel->getMobiusConfig();
    
    // formerly wrote the modified config back to the file system,
    // I don't think that was necessary
    installPresetAndSetup(kernelConfig);

    loadConfigOverlay(kernelConfig);
    
    // now set and propagate the setup and preset
    // note that all loops have to be reset for the preset to be refreshed
    Setup* setup = GetSetup(kernelConfig, UNIT_TEST_SETUP_NAME);

    // this one however activates it so we have to go down to core
    // really want to get rid of this shit
    Mobius* mobius = kernel->getCore();
    mobius->setSetupInternal(setup);

    // install the unit test samples if they aren't already there
    SampleManager* samples = kernel->getSampleManager();
    if (samples != nullptr) {
        // we've already loaded them
        // !! should diff these and replace them if they differ
    }
    else {
        // load the sample files
        // The UI layer builds a "loaded" SampleConfig and passes it to
        // MobiusShell if sample loading is iniated by the UI.
        // Since we have to be able to build the SampleManager down here
        // too, might as well do all sample loading in the shell so the UI
        // doesn't have to bother with SampleReader.
        SampleConfig* srcConfig = kernelConfig->getSampleConfig();

        // take the simple config and load the data
        // interface is weird here
        SampleReader reader;
        SampleConfig* loaded = reader.loadSamples(srcConfig);

        // turn the loaded samples into a SampleManager
        AudioPool* pool = shell->getAudioPool();
        SampleManager* manager = new SampleManager(pool, loaded);

        // SampleManager copied the loaded float buffers into
        // Audio objects, it didn't actually steal the float buffers
        delete loaded;
        
        // at this point we would normally send a MsgSamples
        // down through KernelCommunicator, but we're going to play
        // fast and loose and assume kernel was left in GlobalReset
        kernel->setSampleManager(manager);
    }

    // similar thing could be done for ScriptConfig too
}

/**
 * Initialize the unit test setup and preset within a config object.
 * This is called twice, once for the master config and once for
 * the interrupt config to make sure they're both in sync without
 * having to worry about cloning and adding to the history list.
 */
void UnitTests::installPresetAndSetup(MobiusConfig* config)
{
    // boostrap a preset
    Preset* p = GetPreset(config, UNIT_TEST_PRESET_NAME);
    if (p != NULL) {
        p->reset();
    }
    else {
        p = new Preset();
        p->setName(UNIT_TEST_PRESET_NAME);
        config->addPreset(p);
    }

    // and make it the default
    // just an ordinal now
    // this no longer exists, need to refine a permanent MobiusConfig
    // notion of what this means
    Trace(1, "Mobius::unitTestSetup can't set the current preset!\n");        
    //config->setCurrentPreset(p);

    // boostrap a setup
    Setup* s = GetSetup(config, UNIT_TEST_SETUP_NAME);
    if (s != NULL) {
        s->reset(p);
    }
    else {
        s = new Setup();
        s->setName(UNIT_TEST_SETUP_NAME);
        s->reset(p);
        config->addSetup(s);
    }
    SetCurrentSetup(config, s->ordinal);
}

/**
 * Look for a mobius-overlay.xml and merge it into the main MobiusConfig.
 */
void UnitTests::loadConfigOverlay(MobiusConfig* config)
{
    juce::File root = getTestRoot();
    juce::File file = root.getChildFile("mobius-overlay.xml");
    if (file.existsAsFile()) {
        juce::String xml = file.loadFileAsString();
        XmlRenderer xr;
        MobiusConfig* overlay = xr.parseMobiusConfig(xml.toUTF8());

        // install samples
        SampleConfig* samples = overlay->getSampleConfig();
        if (samples != nullptr) {
            Sample* sample = samples->getSamples();
            Sample* resolved = nullptr;
            Sample* last = nullptr;
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
                    Sample* neu = new Sample(file.getFullPathName().toUTF8());
                    if (resolved == nullptr)
                      resolved = neu;
                    else
                      last->setNext(neu);
                    last = neu;
                }
                sample = sample->getNext();
            }

            // can't modify what is inside overlay as that will be deleted
            SampleConfig* newSamples = new SampleConfig();
            newSamples->setSamples(resolved);

            // since order is critical for trigger indexes, don't try to merge
            // these, it completely replaces it
            config->setSampleConfig(newSamples);
        }

        // install scripts?
        // if we do this, a merge might be better than an overwrite

        // selected parameters, could iterate over the UIParameter::Instances
        // list to get any of them
        config->setInputLatency(overlay->getInputLatency());
        config->setOutputLatency(overlay->getOutputLatency());

        delete overlay;
    }
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

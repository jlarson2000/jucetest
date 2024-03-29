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
#include "../model/UIAction.h"
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
#include "AudioDifferencer.h"


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

// for AudioDifferencer
AudioPool* UnitTests::getAudioPool()
{
    return shell->getAudioPool();
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
 *
 * This interface isn't necessary now that we have an action button,
 * but the old scripts call it all the time so leave it there.
 * Since this called often, only do it once.
 */
void UnitTests::scriptSetup(KernelEvent* e)
{
    // only do this once so scripts can call it multiple times
    if (!enabled) {
        Trace(2, "UnitTests::scriptSetup");
        setup();
        Trace(2, "UnitTests::scriptSetup finished");
    }
}

/**
 * The entry point for the UnitTestMode function when called
 * from a binding, usually a UI action button.
 *
 * If this is called after the initial setup, the test scripts are reloaded.
 * There is currently now way to turn off unit test mode, may want
 * a function for that.  Or this could toggle but I like using
 * secondary actions to reload the scripts after modification.
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
    // FunctionDefinitions don't have a "sustainable" flag so ActionButton
    // will send both down and up gransitions, ignore up
    if (a->down) {
        if (!enabled) {
            setup();
        }
        else {
            reloadScripts();
        }
    }
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
 *
 * Note that once unit test mode is active the MobiusConfig in use
 * by the kernel/core will be different than the one used by the Shell
 * and the UI.  This isn't usually a problem, but if you edit config
 * in the UI or do a LoadScripts, this will push a new MobiusConfig
 * down and changes will be lost.  
 */
void UnitTests::setup()
{
    Trace(2, "UnitTests: Initializing unit test mode");
    
    // this starts KernelEventHandler doing file system redirects
    enabled = true;

    // now we need to dive down and fuck with the core's MobiusConfig
    MobiusKernel* kernel = shell->getKernel();
    MobiusConfig* kernelConfig = kernel->getMobiusConfig();
    
    // special Setup and Preset
    installPresetAndSetup(kernelConfig);

    // test samples and scripts are defined in here
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
 * Reload just the test scripts when unit test mode is activated
 * after initialization.
 */
void UnitTests::reloadScripts()
{
    if (!enabled) {
        setup();
    }
    else {
        Trace(2, "UnitTests: Reloading scripts");
        MobiusConfig* overlay = readConfigOverlay();
        if (overlay != nullptr) {
            Scriptarian* scriptarian = shell->loadScripts(overlay->getScriptConfig());
            shell->sendScripts(scriptarian, true);
            delete overlay;
        }
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
 * into Kernel's config.
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
juce::File UnitTests::getResultFile(const char* name)
{
    juce::File file = getTestRoot().getChildFile("results").getChildFile(name);

    // tests don't usually have an extension so add it
    // assuming a .wav file, will need more when we start dealing with projects
    file = addExtensionWav(file);

    return file;
}

juce::File UnitTests::addExtensionWav(juce::File file)
{
    if (file.getFileExtension().length() == 0)
      file = file.withFileExtension(".wav");

    return file;
}

/**
 * Given a base file name from a script, locate the full
 * path name to that file from the "master" folder
 * of the unit test root where the comparision files are read.
 *
 * We have historically called this "expected" rather than
 * "master" which I ususlly say in comments.
 *
 * Since the database of these is large and maintained in a different
 * Github repository, we support redirection.  If the file exists under
 * TestRoot it is used, otherwise we look for a file named "redirect" and assume
 * the contents of that is the full path of the folder where the file can be found.
 *
 * I'm liking this redirect notion. Generalize this into a common utility
 * and revisit mobius-redirect to use the same code.
 * 
 */
juce::File UnitTests::getExpectedFile(const char* name)
{
    juce::File expected = getTestRoot().getChildFile("expected");
    juce::File file = expected.getChildFile(name);

    file = addExtensionWav(file);

    if (!file.existsAsFile()) {
        // not here check for redirect
        juce::File redirect = followRedirect(expected);
        file = redirect.getChildFile(name);
        file = addExtensionWav(file);
    }

    return file;
}

/**
 * This is basically the same as RootLoctor::checkRedirect
 * find a way to share.
 */
juce::File UnitTests::followRedirect(juce::File root)
{
    juce::File redirected = root;

    juce::File redirect = root.getChildFile("redirect");
    if (redirect.existsAsFile()) {
        
        juce::String content = redirect.loadFileAsString().trim();
        content = findRedirectLine(content);

        if (content.length() == 0) {
            Trace(1, "UnitTest: Redirect file found but was empty");
        }
        else {
            juce::File possible;
            if (juce::File::isAbsolutePath(content)) {
                possible = juce::File(content);
            }
            else {
                // this is the convention used by mobius-redirect
                // if the redirect file contents is relative make it
                // relative to the starting root
                possible = root.getChildFile(content);
            }
        
            if (possible.isDirectory()) {
                // RootLocator allow chains of redirection, unit tests don't
                
                //trace("RootLocator: Redirecting to %s\n", redirect.getFullPathName());
                // recursively redirect again
                // todo: this can cause cycles use a Map to avoid this
                //root = checkRedirect(redirect);
                redirected = possible;
            }
            else {
                Trace(1, "UnitTest: Redirect file found, but directory does not exist: %s\n",
                      possible.getFullPathName().toUTF8());
            }
        }
    }
    
    return redirected;
}

/**
 * Helper for followRedirect()
 * After loading the redirect file contents, look for a line
 * that is not commented out, meaning starts with a #
 */
juce::String UnitTests::findRedirectLine(juce::String src)
{
    juce::String line;

    juce::StringArray tokens;
    tokens.addTokens(src, "\n", "");
    for (int i = 0 ; i < tokens.size() ; i++) {
        line = tokens[i];
        if (!line.startsWith("#")) {
            break;
        }
    }
    
    return line;
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
    
    return getResultFile(name);
}

juce::File UnitTests::getSaveLoopFile(KernelEvent* e)
{
    const char* name = e->arg1;

    if (strlen(name) == 0) {
        Trace(1, "UnitTests: SaveLoop file not specified, defaulting\n");
        name = "testloop";
    }
    
    return getResultFile(name);
}

//////////////////////////////////////////////////////////////////////
//
// Differencing
//
//////////////////////////////////////////////////////////////////////

/**
 * Special entry point for the temporary TestDiff intrinsic function
 * so we can run AudioDifferencer tests without having to record something live.
 */
void UnitTests::analyzeDiff(UIAction* action)
{
    AudioDifferencer differ;
    differ.analyze(this, action);
}

/**
 * Slightly modified version of the old diff utility for .wav files.
 */
void UnitTests::diffAudio(KernelEvent* e)
{
    // factored this out to AudioDifferencer
    AudioDifferencer differ;

    differ.diff(this, e);
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
    juce::File file1 = getResultFile(name1);

    // expected file  is optional, and if not set uses the same base file name
    // as the input file, but reads from a different directory
    juce::File file2;
    if (strlen(name2) == 0)
      file2 = getExpectedFile(name1);
    else
      file2 = getExpectedFile(name2);

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

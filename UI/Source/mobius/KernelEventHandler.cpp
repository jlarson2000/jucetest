/**
 * Helper class for MobiusShell that encapsulates code
 * related to prcessing KernelEvents
 *
 * Most of the stuff in here is realted to unit test files and
 * moved to UnitTests.
 * 
 */

// let Juce leak in here for File handling
#include <JuceHeader.h>

#include "../util/Trace.h"
#include "../util/Util.h"
#include "../model/MobiusConfig.h"

#include "Audio.h"
#include "AudioFile.h"

#include "KernelEvent.h"
#include "MobiusShell.h"
#include "UnitTests.h"

#include "core/Mobius.h"

#include "KernelEventHandler.h"

/**
 * Process an event sent up by the Kernel.  All but oen will be from
 * scripts, and most are only relevant for the unit tests.
 * 
 * This came out of a KernelMessage which will return the event
 * to the Kernel so that it may be returned to the pool.  All we
 * need to do is process it and in rare cases set a return code.
 *
 * There aren't many of these so a switch gets the job done.
 */
void KernelEventHandler::doEvent(KernelEvent* e)
{
    if (e != nullptr) {
        switch (e->type) {
            
            case EventSaveLoop:
                doSaveLoop(e); break;
                
            case EventSaveCapture:
                doSaveCapture(e); break;

            case EventSaveProject:
                doSaveProject(e); break;

            case EventSaveConfig:
                doSaveConfig(e); break;

            case EventLoadLoop:
                doLoadLoop(e); break;

            case EventDiff:
                doDiff(e); break;

            case EventDiffAudio:
                doDiffAudio(e); break;

            case EventPrompt:
                doPrompt(e); break;

            case EventEcho:
                doEcho(e); break;

            case EventTimeBoundary:
                doTimeBoundary(e); break;
                
            case EventAlert:
                doAlert(e); break;

            case EventUnitTestSetup:
                doUnitTestSetup(e); break;
                
            default:
                Trace(1, "KernelEventHandler: Unknown type code %d\n", e->type);
                break;
        }
    }
}

/**
 * This is where we end up at the end of the SaveCapture function.
 * The event contains the file name the script wants to save it in
 * but not the actual Audio to save.  For that we have to
 * call back to Mobius.
 *
 * Not sure I like having to reach down there from the shell.
 * It would be cleaner to pass the Audio back in the KernelEvent?
 *
 * Note that the Audio object is still owned by Mobius and must
 * not be deleted.  Mobius is supposed to not be touching
 * this while we have it.
 *
 * When called from a script, the file name is in the event.
 * When called by the user, a file might have been specified
 * as a function argument in the binding/action which
 * should also have been left in the event.
 */
void KernelEventHandler::doSaveCapture(KernelEvent* e)
{
    // get the Audio to save
    MobiusKernel* kernel = shell->getKernel();
    Mobius* mobius = kernel->getCore();
    Audio* capture = mobius->getCapture();

    juce::File file;
    if (UnitTests::Instance->isEnabled()) {
        file = UnitTests::Instance->getSaveCaptureFile(e);
    }
    else {
        file = getSaveFile(e->arg1, "capture", ".wav");
    }
    
    AudioFile::write(file, capture);
}

void KernelEventHandler::doAlert(KernelEvent* e)
{
    MobiusListener* l = shell->getListener();
    if (l != nullptr) {
        juce::String alert (e->arg1);
        l->MobiusAlert(alert);
    }
}

/**
 * This is where we end up at the end of the SaveLoop function.
 *
 * Still have the old convention of not passing the loop Audio
 * in the event, but expecting the handler to call back to getPlaybackAudio.
 * See comments over there why this sucks and is dangerous.
 *
 * For any complex state file saves the problem from the UI/shell is that it
 * is unreliable to capture the state of an Audio object while the audio thread
 * is active.  The AudioCursors can be different, but there is no guarantee that
 * the internal block list won't be changing while we're cursoring.  There are only
 * two safe ways to do this:
 *
 *    1) have the kernel build the necessary Audio copies at the start of the
 *       buffer processing callback
 *    2) have the shell place the kernel in some sort of suspended state where
 *       it can't do anything to change the current memory model, then carefully
 *       walk over it
 *
 * 1 is simpler but it requires the kernel to allocate a potentially large number
 * of audio blocks.   It can get those from the pool, but unless we ensure capacity
 * before the Kernel gets control it may have to allocate.  This can also be a very
 * expensive operation, especially for Projects so it becomes possible to miss an
 * interrupt and click.
 *
 * 2 has some nice qualities but the suspended state is harder to implement.  It would
 * mess up any timing related scripts which is not that bad since you tend not to save
 * files whe're you're busy with loop building.
 *
 * No good simple solutions.
 * See Layer::flatten for more thoughts.
 *
 * If the file name is passed through the event it will be used.  If not passed
 * it used the value of the quickSave global parameter as the base file name,
 * then added a numeric suffix to make it unique.
 * Not doing uniqueness yet but need to.
 *
 * Note that the Audio returned by getPlaybackAudio becomes owned by the caller
 * and must be freed.  The blocks came from the common AudioPool.
 */
void KernelEventHandler::doSaveLoop(KernelEvent* e)
{
    // get the Audio to save
    MobiusKernel* kernel = shell->getKernel();
    Mobius* mobius = kernel->getCore();
    Audio* loop = mobius->getPlaybackAudio();

    if (loop == nullptr) {
        Trace(1, "KernelEventHandler::doSaveLoop getPlaybackAudio returned nullptr");
    }
    else {
        juce::File file;
        if (UnitTests::Instance->isEnabled()) {
            file = UnitTests::Instance->getSaveLoopFile(e);
        }
        else {
            MobiusConfig* config = shell->getConfiguration();
            const char* quickfile = config->getQuickSave();
            if (quickfile == nullptr) {
                // this is what old code used, better name might
                // just be "quicksave" to show where it came from
                quickfile = "mobiusloop";
            }
        
            file = getSaveFile(e->arg1, quickfile, ".wav");
        }

        AudioFile::write(file, loop);

        // we own this
        delete loop;
    }
}

/**
 * This was also fraught with peril
 */
void KernelEventHandler::doSaveProject(KernelEvent* e)
{
}

/**
 * This was an obscure one used to permanently save
 * the MobiusConfig file if an Action came down to change
 * the setup, and OperatorPermanent was used.
 * Took that out since it probably shouldn't be supported
 * so this handler can go away.
 */
void KernelEventHandler::doSaveConfig(KernelEvent* e)
{
}

void KernelEventHandler::doLoadLoop(KernelEvent* e)
{
}

/**
 * Here for an event scheduled by the special
 * ScriptDiffStatement which is not a formal Function.
 * Since there is no action scripts can't wait on this and
 * expect nothing in return.
 */
void KernelEventHandler::doDiffAudio(KernelEvent* e)
{
    UnitTests::Instance->diffAudio(e);
}

/** 
 * Like doDiffAudio but for non-Audio files.
 * Think this was only used for Project structure files.
 */
void KernelEventHandler::doDiff(KernelEvent* e)
{
    UnitTests::Instance->diffText(e);
}

/**
 * A partially finished feature to let scripts interactively
 * prompt the user for a yes/no decision.  Never used much
 * if at all.  This is interesting in theory so keep the
 * mechanism in place, but it needs more work to be generally
 * useful.
 */
void KernelEventHandler::doPrompt(KernelEvent* e)
{
}

/**
 * !! this does not appear to be needed
 * ScriptEchoStatement commented out the event send and
 * is just calling Trace directly
 */
void KernelEventHandler::doEcho(KernelEvent* e)
{
}

/**
 * Here we're supposed to call MobiusListener to make
 * the maintenance thread loop break out of a wait state
 * and refresh the UI early.  
 *
 * !!  don't think this works or is necessary, if we're here
 * then we're already in the maintenance thread since
 * that's where shell events are processed.  So must have
 * called this directly from the audio thread.
 */
void KernelEventHandler::doTimeBoundary(KernelEvent* e)
{
}

/**
 * Here after the UnitTestSetup script statement.
 * This can install complex configuration.
 */
void KernelEventHandler::doUnitTestSetup(KernelEvent* e)
{
    UnitTests::Instance->scriptSetup(e);
}

//////////////////////////////////////////////////////////////////////
//
// File Utilities
//
//////////////////////////////////////////////////////////////////////

/**
 * Build the path to a file for save or load.
 * 
 * Old code allowed the path to be absolute, relative
 * to the CWD if it began with "./" or relative to the
 * home directory.  Home directory was from the MOBIUS_HOME
 * environment variable or a complex process that tried to locate
 * the the .dll or use the registry.  We'll now get this from
 * MobiusContainer and let Juce locate the standard locations.
 *
 * If that didn't work we used getRecordingPath which
 * first looked for MobiusConfig::quickSaveFile, stripped
 * off the leaf file name and appended "recording.wav".
 *
 * Now we'll call this "capture.wav".  Comments say
 * Quick Save used a counter for name uniqueness, capture
 * didn't have that.
 *
 * UnitTests have their own version of this with more assumptions.
 *
 */
juce::File KernelEventHandler::getSaveFile(const char* name, const char* defaultName,
                                           const char* extension)
{
    MobiusContainer* container = shell->getContainer();
    juce::File root = container->getRoot();
    juce::File file;

    if (strlen(name) > 0) {
        // name passed as a function arg
        if (juce::File::isAbsolutePath(name)) {
            juce::File maybe = juce::File(name);
            // Ambiguity about what this could mean: c:/foo/bar
            // Is that the "bar" base file name in the c:/foo directory
            // or is it an unspecified file name in the c:/foo/bar directory?
            // If the name was not absolute we assumed it was a file name,
            // possibly without an extension, and possibly under one or more
            // parent directories to be placed relative to the config directory
            // example: SaveAudioRecording ./$(testname)$(suffix)rec
            // to be consistent with that common use assume that an absolute path
            // includes the leaf file name, but possibly without an extension
            // any reason not to just force it to .wav ?
            if (maybe.getFileExtension().length() == 0)
              maybe = maybe.withFileExtension(extension);
            
            if (maybe.existsAsFile()) {
                file = maybe;
            }
            else {
                // not a file yet, I don't want to auto create parent directories
                // if the user typed it in wrong, so make sure the parent exists
                juce::File parent = maybe.getParentDirectory();
                if (parent.isDirectory()) {
                    file = maybe;
                }
                else {
                    // this is the first time we need an alert
                    // for unit tests, just trace and move on but for
                    // user initiated will need to display something
                    Trace(1, "Invalid file path: %s\n", maybe.getFullPathName().toUTF8());
                }
            }
        }
        else {
            // not absolute, put it under the root config directory
            MobiusContainer* container = shell->getContainer();
            juce::File root = container->getRoot();
            juce::File maybe = root.getChildFile(name);
            if (maybe.getFileExtension().length() == 0)
              maybe = maybe.withFileExtension(extension);

            if (maybe.existsAsFile()) {
                file = maybe;
            }
            else {
                // since we've extended the root which must exist, can allow
                // the automatic creation of subdirectries
                file = maybe;
            }
        }
    }
    else {
        // no name passed, use the defaults
        file = root.getChildFile(defaultName).withFileExtension(extension);

        // !! todo: old code checked the file for uniqueness and if it was
        // already there added a numeric qualifier, need to do this for both
        // quickSave and capture
    }

    // seems like an odd way to test for being set
    if (file == juce::File()) {
        // unable to verify a location, should have already traced why
    }
    else {
        juce::Result result = file.create();
        if (!result.wasOk()) {
            Trace(1, "Unable to create file: %s\n", file.getFullPathName().toUTF8());
            file = juce::File();
        }
    }
    
    return file;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

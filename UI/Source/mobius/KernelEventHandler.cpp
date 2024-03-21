/**
 * Helper class for MobiusShell that encapsulates code
 * related to prcessing KernelEvents
 */

// let Juce leak in here for File handling
#include <JuceHeader.h>

#include "../util/Trace.h"

#include "../RootLocator.h"

#include "Audio.h"
#include "KernelEvent.h"
#include "MobiusShell.h"
#include "KernelEventHandler.h"

#include "core/Mobius.h"

/**
 * Process an event sent up by the Kernel.  All but one
 * are from Scripts atm.
 * 
 * This came out of a KernelMessage which will return the event
 * to the Kernel so that it may be returned to the pool.  All we
 * need to do is process it and in rare cases set a return code.
 *
 * There aren't many of these so a switch gets the job done.
 * What most of these do is under heavy redesign so they're stubs.
 * 
 */
void KernelEventHandler::doEvent(KernelEvent* e)
{
    if (e != nullptr) {
        switch (e->type) {
            
            case EventSaveLoop:
                doSaveLoop(e); break;
                
            case EventSaveAudio:
                doSaveAudio(e); break;

            case EventSaveProject:
                doSaveProject(e); break;

            case EventSaveConfig:
                doSaveConfig(e); break;

            case EventLoad:
                doLoad(e); break;

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
void KernelEventHandler::doSaveAudio(KernelEvent* e)
{
    // get the Audio to save
    MobiusKernel* kernel = shell->getKernel();
    Mobius* mobius = kernel->getCore();
    Audio* capture = mobius->getCapture();

    juce::File file = buildFilePath(e->arg1, "capture", ".wav");

    // use the old utility for now
    writeFile(capture, file);
}

/**
 * Old code scheduled this from Mobius::saveLoop which was
 * "called by the UI to save the current loop to a file".
 * From MobiusThread it would just call back to Mobius::getPlaybackAudio
 * and write the file, so this never needed to be an event in the first place.
 *
 * There was also the Save Function which could have initiated the save from
 * within and would have needed an event.
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
 */
void KernelEventHandler::doSaveLoop(KernelEvent* e)
{
    // saveLoop was taken out pending Project redesign
}

/**
 * Seems to be scheduled by a scripot, did
 * This was also fraught with peril
 */
void KernelEventHandler::doSaveProject(KernelEvent* e)
{
}

void KernelEventHandler::doSaveConfig(KernelEvent* e)
{
}

void KernelEventHandler::doLoad(KernelEvent* e)
{
}

void KernelEventHandler::doDiff(KernelEvent* e)
{
}

void KernelEventHandler::doDiffAudio(KernelEvent* e)
{
}

void KernelEventHandler::doPrompt(KernelEvent* e)
{
}

void KernelEventHandler::doEcho(KernelEvent* e)
{
}

void KernelEventHandler::doTimeBoundary(KernelEvent* e)
{
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
 */
juce::File KernelEventHandler::buildFilePath(const char* name, const char* defaultName,
                                             const char* extension)
{
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
            if (maybe.getFileExtension().length() == 0)
              maybe = juce::File(juce::String(name) + extension);
            
            if (file.existsAsFile()) {
                file = maybe;
            }
            else {
                // not a file yet, I do't want to auto create parent directories
                // so make sure that existts first
                juce::File parent = file.getParentDirectory();
                if (!parent.isDirectory()) {
                    // this is the first time we need an alert
                    // for unit tests, just trace and move on but for
                    // user initiated will need to display something
                    Trace(1, "Invalid file path: %s\n", file.getFullPathName().toUTF8());
                }
                else {
                    file = maybe;
                }
            }
        }
    }

    // this sure seems like a weird way to test "is it set?"
    if (file == juce::File()) {
        // not absolute put it under the config directory
        MobiusContainer* container = shell->getContainer();
        juce::File root = container->getRoot();
        if (strlen(name) > 0) {
            juce::File maybe = juce::File(root.getFullPathName() + name);
            if (maybe.getFileExtension().length() == 0)
              maybe = juce::File(juce::String(name) + extension);
            
            if (file.existsAsFile()) {
                file = maybe;
            }
            else {
                // can skip the parent existance in this case since we can
                // trust container->getRoot
                file = maybe;
            }
        }
        else {
            // sub the default name
            file = juce::File(root.getFullPathName() + defaultName + extension);
        }
    }

    juce::Result result = file.create();
    const char* path = file.getFullPathName().toUTF8();
    if (!result.wasOk()) {
        Trace(1, "Unable to create file: %s\n", path);
        file = juce::File();
    }

    return file;
}

/**
 * Write an audio file using the old tool.
 */
void KernelEventHandler::writeFile(Audio* a, juce::File file)
{
}

#if 0
int Audio::write(const char *name, int format) 
{
	int error = 0;

	WaveFile* wav = new WaveFile();
	wav->setChannels(mChannels);
	wav->setFrames(mFrames);
	wav->setFormat(format);
	wav->setFile(name);

	error = wav->writeStart();
	if (error) {
		Trace(1, "Error writing file %s: %s\n", name, 
			  wav->getErrorMessage(error));
	}
	else {
		// write one frame at a time not terribly effecient but messing
		// with blocking at this level isn't going to save much
		AudioBuffer b;
		float buffer[AUDIO_MAX_CHANNELS];
		b.buffer = buffer;
		b.frames = 1;
		b.channels = mChannels;

		for (long i = 0 ; i < mFrames ; i++) {
			memset(buffer, 0, sizeof(buffer));
			get(&b, i);
			wav->write(buffer, 1);
		}

		error = wav->writeFinish();
		if (error) {
			Trace(1, "Error finishing file %s: %s\n", name, 
				  wav->getErrorMessage(error));
		}
	}

	delete wav;
	return error;
}
#endif

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

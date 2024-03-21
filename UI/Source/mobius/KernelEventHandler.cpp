/**
 * Helper class for MobiusShell that encapsulates code
 * related to prcessing KernelEvents
 */

// let Juce leak in here for File handling
#include <JuceHeader.h>

#include "../util/Trace.h"
#include "../util/Util.h"
#include "../model/MobiusConfig.h"

#include "Audio.h"
#include "KernelEvent.h"
#include "MobiusShell.h"
#include "KernelEventHandler.h"
#include "WaveFile.h"

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
 *
 * If the file name is passed through the event it will be used.  If not passed
 * it used the value of the quickSave global parameter as the base file name,
 * then added a numeric suffix to make it unique.
 * Not doing uniqueness yet but need to.
 */
void KernelEventHandler::doSaveLoop(KernelEvent* e)
{
    // get the Audio to save
    MobiusKernel* kernel = shell->getKernel();
    Mobius* mobius = kernel->getCore();
    Audio* loop = mobius->getPlaybackAudio();

    MobiusConfig* config = shell->getConfiguration();
    const char* quickfile = config->getQuickSave();
    if (quickfile == nullptr) {
        // this is what old code used, better name might
        // just be "quicksave" to show where it came from
        quickfile = "mobiusloop";
    }

    juce::File file = buildFilePath(e->arg1, quickfile, ".wav");

    // use the old utility for now
    writeFile(loop, file);
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

/**
 * Here for an event scheduled by the special
 * ScriptDiffStatement which is not a formal Function.
 * Since there is no action scripts can't wait on this and
 * expect nothing in return.
 *
 * Unlike SaveLoop and SaveCapture, this is useful only
 * for the unit tests.  Tests expect it to compare two audio
 * files and emit usefful messages to the console for inspection.
 * 
 * This is useful only for the unit tests
 */
void KernelEventHandler::doDiffAudio(KernelEvent* e)
{
    const char* file1 = e->arg1;
    const char* file2 = e->arg2;
    bool reverse = StringEqualNoCase(e->arg3, "reverse");

    if (file1 != NULL && file2 != NULL) {
        // just assume these are both relative
        // !! what does that mean, relative to what?
        // first arg "type" is now an EventType not an int
        // diff(e->type, reverse, file1, file2);
    }
    else if (file1 != NULL) {
#if 0
        const char* extension = ".wav";
        const char* master = getTestPath(file1, extension);
        char newpath[1025];
        strcpy(newpath, file1);
        if (!EndsWith(newpath, extension))
          strcat(newpath, extension);
        diff(type, reverse, newpath, master);
#endif
        
    }
}

#if 0
PRIVATE const char* MobiusThread::getTestPath(const char* name, const 
											  char* extension)
{
	MobiusConfig* config = mMobius->getConfiguration();
	const char* root = config->getUnitTests();
	if (root == NULL) {
		// guess
		root = "../../../mobiustest";
	}

	sprintf(mPathBuffer, "%s/expected/%s", root, name);

	if (!EndsWith(mPathBuffer, extension))
	  strcat(mPathBuffer, extension);

	return mPathBuffer;
}
#endif

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
 * it break out of the maintenance thread waith loop
 * and refreh the UI.
 * !! actually I don't think this works, if we're here
 * then we're already in the maintenance thread since
 * that's where shell events are processed.  So must have
 * called this directly from the audio thread.
 */
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

// this used to be saved in the .wav file
// int Audio::WriteFormat = WAV_FORMAT_IEEE;

/**
 * Write an audio file using the old tool.
 * This is an adaptation of what used to be in Audio::write()
 * which no longer exists, but still sucks because it does this
 * a sample at a time rather than blocking.  Okay for initial testing
 * but you can do better.
 */
void KernelEventHandler::writeFile(Audio* a, juce::File file)
{
    // Old code gave the illusion that it supported something other than 2
    // channels but this was never tested.  Ensuing that this all stays
    // in sync and something forgot to set the channels is tedius, just
    // force it to 2 no matter what Audio says
    //int channels = a->getChannels();
    int channels = 2;
    int frames = a->getFrames();
    
	WaveFile* wav = new WaveFile();
	wav->setChannels(channels);
	wav->setFrames(frames);
    // other format is PCM, but I don't think the old writer supported that?
	wav->setFormat(WAV_FORMAT_IEEE);
    // this was how we conveyed the file path
    const char* path = file.getFullPathName().toUTF8();
	wav->setFile(path);
    
	int error = wav->writeStart();
	if (error) {
		Trace(1, "Error writing file %s: %s\n", path, 
			  wav->getErrorMessage(error));
	}
	else {
		// write one frame at a time not terribly effecient but messing
		// with blocking at this level isn't going to save much
		AudioBuffer b;
        // not sure where this constant went but we don't support more than 2 anyway
		//float buffer[AUDIO_MAX_CHANNELS];
		float buffer[4];
		b.buffer = buffer;
		b.frames = 1;
		b.channels = channels;

		for (long i = 0 ; i < frames ; i++) {
			memset(buffer, 0, sizeof(buffer));
			a->get(&b, i);
			wav->write(buffer, 1);
		}

		error = wav->writeFinish();
		if (error) {
			Trace(1, "Error finishing file %s: %s\n", path, 
				  wav->getErrorMessage(error));
		}
    }

    delete wav;
}

/**
 * This a slightly modified version of the old diff utility.
 * Rather than printf it sends to the Trace log.
 * Actually no, it will need heavy modifications because it needs
 * the old file utils I don't want to drag over and it used Audio(file)
 * constructor to read the files which no longer exists.
 * 
 * Diff two files.  Assume they are binary.
 * Could be smarter about printing differences if these turn
 * out to be non-binary project files.
 */
void KernelEventHandler::diff(int type, bool reverse,
                              const char* file1, const char* file2)
{
#if 0    
	int size1 = GetFileSize(file1);
	int size2 = GetFileSize(file2);

	if (size1 < 0) {
		printf("ERROR: File does not exist: %s\n", file1);
		fflush(stdout);
	}
	else if (size2 < 0) {
		printf("ERROR: File does not exist: %s\n", file2);
		fflush(stdout);
	}
	else if (size1 != size2) {
		printf("ERROR: Files differ in size: %s, %s\n", file1, file2);
		fflush(stdout);
	}
	else {
		FILE* fp1 = fopen(file1, "rb");
		if (fp1 == NULL) {
			printf("Unable to open file: %s\n", file1);
			fflush(stdout);
		}
		else {
			FILE* fp2 = fopen(file2, "rb");
			if (fp2 == NULL) {
				printf("Unable to open file: %s\n", file2);
				fflush(stdout);
			}
			else {
				bool different = false;
				if (type == TE_DIFF_AUDIO) {

					bool checkFloats = false;
                    AudioPool* pool = mMobius->getAudioPool();

					Audio* a1 = pool->newAudio(file1);
					Audio* a2 = pool->newAudio(file2);
					int channels = a1->getChannels();

					if (a1->getFrames() != a2->getFrames()) {
						printf("Frame counts differ %s, %s\n", file1, file2);
						fflush(stdout);
					}
					else if (channels != a2->getChannels()) {
						printf("Channel counts differ %s, %s\n", file1, file2);
						fflush(stdout);
					}
					else {
						AudioBuffer b1;
						float f1[AUDIO_MAX_CHANNELS];
						b1.buffer = f1;
						b1.frames = 1;
						b1.channels = channels;

						AudioBuffer b2;
						float f2[AUDIO_MAX_CHANNELS];
						b2.buffer = f2;
						b2.frames = 1;
						b2.channels = channels;

						bool stop = false;
						int psn2 = (reverse) ? a2->getFrames() - 1 : 0;

						for (int i = 0 ; i < a1->getFrames() && !different ; 
							 i++) {

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
										printf("WARNING: files differ at frame %d: %f %f: %s, %s\n", 
											   i, f1[j], f2[j], file1, file2);
										fflush(stdout);
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
									printf("Files differ at frame %d: %d %d: %s, %s\n", 
										   i, i1, i2, file1, file2);
									fflush(stdout);
									different = true;
								}
							}

							if (reverse)
							  psn2--;
							else
							  psn2++;
						}
					}

					delete a1;
					delete a2;
				}
				else {
					unsigned char byte1;
					unsigned char byte2;

					for (int i = 0 ; i < size1 && !different ; i++) {
						if (fread(&byte1, 1, 1, fp1) < 1) {
							printf("Unable to read file: %s\n", file1);
							fflush(stdout);
							different = true;
						}
						else if (fread(&byte2, 1, 1, fp2) < 1) {
							printf("Unable to read file: %s\n", file2);
							fflush(stdout);
							different = true;
						}
						else if (byte1 != byte2) {
							printf("Files differ at byte %d: %s, %s\n",
								   i, file1, file2);
							fflush(stdout);
							different = true;
						}
					}
				}
				fclose(fp2);
				if (!different) {
				  printf("%s - ok\n", file1);
				  fflush(stdout);
				}
			}
			fclose(fp1);
		}
	}

	fflush(stdout);

#endif
    
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

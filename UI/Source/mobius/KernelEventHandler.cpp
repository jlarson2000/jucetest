/**
 * Helper class for MobiusShell that encapsulates code
 * related to prcessing KernelEvents
 */

#include "../util/Trace.h"

#include "KernelEvent.h"
#include "MobiusShell.h"
#include "KernelEventHandler.h"

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
 * In old code this was scheduled for the SaveCapture function
 * which called Mobius::saveCapture.  In MobiusThread this was implemented
 * with Mobius::getCapture which just returned the Audio we were capturing into.
 *
 * This one is less senstiive than SaveLoop since there is less complicated going on
 * but the Audio can still be expanding from the audio thread while we're trying to save it.
 */
void KernelEventHandler::doSaveAudio(KernelEvent* e)
{
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

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

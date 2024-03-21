/**
 * Model for passing a request from the kernel up to the shell for processing
 * optionally with results sent back down.
 *
 * This differs from actions which always go from shell down to kernel.
 *
 * The name "event" is a bit unfortunate since we also have the Event class
 * which is used for scheduling things to happen on the track timeline.
 *
 * This takes the place of what the old code called ThreadEvent.
 */

#pragma once

/**
 * The types of KernelEvents that demand attention.
 */
typedef enum {

    EventNone = 0,

    // wait for what?
    EventWait,

    // Need to revisit these, do we really need the initiation
    // of state file saves from the Kernel?  Usually they would
    // come from the UI and could be processed as UIActions in the Shell
    // or just MobiusInterface method calls.  They don't need to go all the way
    // down to a Kernel Function handler, then back up.
    // Would only need events if you can ask for this from scripts.
    EventSaveLoop,

    // Used by the SaveCapture function to save the results
    // of a capture to a file.  File name to use is passed in the first argument
    EventSaveAudio,
    
    EventSaveProject,
    // this was a weird one, it was in response to the UI setting OperatorPermanent
    // on a Setup action to cause it to be saved permanently in mobius.xml
    // we shouldn't need that in an Action handler, just do it in the UI if that's
    // what you want
    EventSaveConfig,
    
    EventLoad,

    // these are designed for unit tests scripts so they do need to be events
    EventDiff,
    EventDiffAudio,

    // this was for test scripts, but could be generally useful
    EventPrompt,

    // Used by ScriptEchoStatement
    // for some reason I stopped using printf and made a MobiusThread event,
    // but that just did printf, so why bother?
    EventEcho,

    // not sure what this was for, I guess to inform the UI that we entered
    // global reset, so it can do a big repait, why not just have a more
    // generic EventRefresh?
    // never used
    //EventGlobalReset,

    // this was how we asked the UI to refresh closer to a subcycle/cycle/loop
    // boundary being crossed rather than waiting for the next 1/10th refresh cycle
    // it made the UI appear more accurate for things like the beaters that were supposed
    // to pulse at regular intervals
    // since this happens frequently and is simple, it doesn't have to be a ThreadEvent
    // it could just be a KernelMessage type
    EventTimeBoundary

} KernelEventType;

/**
 * Maximum length of the string that may be placed in a
 * KernelEvent argument array.
 *
 * This was 1024 which seems high, but we won't have many of these.
 */
const int KernelEventMaxArg = 1024;
    
/**
 * The main event.
 */
class KernelEvent
{
  public:

    KernelEvent();
    ~KernelEvent();

    // unused events are maintained in a pool
    KernelEvent* next;

    // what the Kernel wants to do
    KernelEventType type;

    // continue what ThreadEvent did by having three arguments
    // whose contents depend on the type
    char arg1[KernelEventMaxArg];
    char arg2[KernelEventMaxArg];
    char arg3[KernelEventMaxArg];

    // the return code sent back down for the EventPrompt event
    // this was the only event that could return something
    int returnCode;

    // EventSaveProject used to pass entire Projects around
    // not sure I like this
    class Project* project;

    // set an argument with the usual bounds checking
    // returns true if the value fit, calls are encouraged to bail if it doesn't
    // this was used a lot to pass file paths but we really shouldn't be doing
    // long paths in scripts anyway and they should always be relative
    // to something the container gets to decide
    bool setArg(int number, const char* value);

  private:

    void init();

};

/**
 * The usual linked list pool.
 * 
 * REALLY need to generalize this into a common base pool class
 * and stop duplicating this.  I tried that with ObjectPools in old code
 * but it never worked right, need to revisit that.
 *
 * EventPool is weird because of all the interconnections, but Action
 * could use this and probably others.  Maybe Layer.
 *
 * Flexible capacity maintenance is MUCH less important here than it
 * is for KernelMessage since kernel events are rare.  It would take
 * a rogue script vomiting Save requests to deplete it.
 *
 * Because Shell won't be checking capacity, we don't have to worry
 * about thread safety yet, only Kernel can touch this.
 */
class KernelEventPool
{
  public:

    KernelEventPool();
    ~KernelEventPool();

    KernelEvent* getEvent();
    void returnEvent(KernelEvent* e);

    void checkCapacity(int desired);
    void dump();
    
  private:

    KernelEvent* mPool;
    int mAllocated;
    int mUsed;

};


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

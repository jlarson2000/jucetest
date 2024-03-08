/**
 * Things that used to live in MobiusInterface and still referenced in code.
 * MobiusInterface no longer exists at this level, there is a new one up in kernel
 * that has a simplified interface used by Supervisor.
 *
 * Need to decide which of these still deserve to live and if they do, move them up.
 *
 * Not carrying forward MobiusContext, what we little we need from there is now
 * in MobiusContainer
 *
 * Not defining an OldMobiusInterface, just convert old code to go directly to Mobius
 *
 */

#pragma once

/****************************************************************************
 *                                                                          *
 *                                   PROMPT                                 *
 *                                                                          *
 ****************************************************************************/

// The implementation is defined in MobiusThread because it needs ThreadEvent

/**
 * A class used to pass information related to user prompting
 * between the Mobius engine and the UI.  One of these is generated
 * by the script interpreter when evaluating a Prompt statement.  
 * The prompt has an associated with a ThreadEvent that the script will
 * be waiting on.
 *
 * The listener is responsible for displaying the prompt message in
 * a suitable way and soliciting a response.  The response is then
 * set in the Prompt object, and returned by caling Mobius::finishPrompt().
 */
class Prompt {
	
  public:

	Prompt();
	~Prompt();

	Prompt* getNext();
	void setNext(Prompt* p);

	const char* getText();
	void setText(const char* text);

	bool isOk();
	void setOk(bool b);
    
	class ThreadEvent* getEvent();
	void setEvent(class ThreadEvent* e);

  private:

	Prompt* mNext;
	class ThreadEvent* mEvent;
	char* mText;
	bool mOk;
};


/****************************************************************************
 *                                                                          *
 *                                  LISTENER                                *
 *                                                                          *
 ****************************************************************************/

// renamed to OldMobiusListener since the new interface has one too

/**
 * The interface of an object that may receive notification of
 * interesting happenings within Mobius.
 *
 * The most important callback is MobiusRefresh will will be called
 * periodically to tell the UI to redisplay state.  This will be
 * called nearly once every 1/10 second but may be impacted 
 * by other things being done by the Mobius housekeeping thread.
 * It is conceptually similar to the VST "idle" callback, and saves
 * the UI from having to manage its own update timer.
 *
 * The MobiusTimeBoundary callback is called whenever a significant
 * synchronization boundary has passed: beat, bar, cycle, or loop.
 * This can be used by the UI to refresh time sensitive components
 * immediately rather than waiting for the next MobiusRefresh tick
 * or the next private timer tick.  This makes things like beat flashers
 * look more accurate.  
 *
 * MobiusRefresh was added after MobiusTimeBoundary, we could consider
 * merging them and just having MobiusRefresh be called early but I like
 * keeping them distinct for now so you can use MobiusRefresh as a relatively
 * accurate timer.
 * 
 */
class OldMobiusListener {

  public:

	/**
	 * A periodic refresh interval has been reached.
	 * This is normally called once every 1/10 second.
	 */
	virtual void MobiusRefresh() = 0;

	/**
	 * A significant time boundary has passed (beat, cycle, loop)
	 * so refresh time sensitive components now rather than waiting
	 * for the next timer event to make it look more accurate.
	 */
	virtual void MobiusTimeBoundary() = 0;

	/**
	 * Display some sort of exceptional alert message.
	 */
	virtual void MobiusAlert(const char *msg) = 0;

	/**
	 * Display a normal operational message.
	 */
	virtual void MobiusMessage(const char *msg) = 0;

	/**
	 * Receive notification of a MIDI event.
	 * Return true if Mobius is to continue processing the event.
	 */
	virtual bool MobiusMidiEvent(class MidiEvent* e) = 0;

	/**
	 * Prompt the user for information.
	 */
	virtual void MobiusPrompt(Prompt* p) = 0;

    /**
     * Notify of an internal configuration change, listener may want
     * to refresh displayed configuration state.
     */
    virtual void MobiusConfigChanged() = 0;

    /**
     * Notify of a global reset.
     * This is a hopefully temporary kludge for the message display
     * which we want to allow to persist for a long time, but still
     * clear it when you do a global reset.
     */
    virtual void MobiusGlobalReset() = 0;

    /**
     * Notify the UI of an action on a UIControl.
     */
    virtual void MobiusAction(class Action* action) = 0;


    /**
     * Notify the UI that something major has happened and it should
     * repaint the entire UI.
     */
    virtual void MobiusRedraw() = 0;

};

/****************************************************************************
 *                                                                          *
 *   						 LATENCY CALIBRATION                            *
 *                                                                          *
 ****************************************************************************/

// where was this?

/**
 * This is a duplicate of RecorderCalibrationResult from Recorder.h.
 * Think more about how we want this conveyed, or if we should share this.
 */ 
class CalibrationResult {

  public:

	CalibrationResult() {
		timeout = false;
		noiseFloor = 0.0;
		latency = 0;
	}

	~CalibrationResult() {
	}

	bool timeout;
	float noiseFloor;
	int latency;
};

/****************************************************************************
 *                                                                          *
 *                                   ALERTS                                 *
 *                                                                          *
 ****************************************************************************/

/**
 * An object containing various problems that have happened during
 * Mobius execution that should be presented to the user.
 * Originally a bunch of discrete methods on Mobius, think more about
 * using this for other severe occurrences, the kind of things we would
 * trace with level 1.
 */
class MobiusAlerts {
  public:

    MobiusAlerts();

    /**
     * True if we could not open the configured audio input device.
     */
    bool audioInputInvalid;
    
    /**
     * True if we would not open the configured audio output device.
     */
    bool audioOutputInvalid;

    const char* midiInputError;
    const char* midiOutputError;
    const char* midiThroughError;

};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


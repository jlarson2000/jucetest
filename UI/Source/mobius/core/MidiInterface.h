/**
 * Stub impelemntation of MidiInterface to get things to compile
 * before we migrate to MobiusContainer
 *
 * Removed all device handling
 */

#pragma once

// Listeners formerly in MidiListener.h

/**
 * The interface of an object that may be registered MidiInterface
 * to recieve individual MIDI events.
 * This is only used by MidiInterface so it cold be moved there.
 */
class MidiEventListener {

  public:

	virtual void midiEvent(class MidiEvent* e) = 0;

};

/**
 * The interface of an object that may be registered with a Timer or MidiInterface
 * to receive MIDI clock callbacks.  These aren't events, you will
 * be called in the timer interrupt whenever a MIDI clock is advanced,
 * and optionally when a MIDI clock is sent to the MIDI output device.
 */
class MidiClockListener {

  public:

	virtual void midiClockEvent() = 0;
	virtual void midiStartEvent() = 0;
	virtual void midiStopEvent() = 0;
	virtual void midiContinueEvent() = 0;

};

/****************************************************************************
 *                                                                          *
 *   							MIDI INTERFACE                              *
 *                                                                          *
 ****************************************************************************/

class MidiInterface {

  public:
    
	virtual ~MidiInterface() {}
	
	//virtual class MidiPort* getInputPorts() = 0;
	//virtual class MidiPort* getOutputPorts() = 0;

	virtual void setListener(class MidiEventListener* h) = 0;
	virtual void setClockListener(class MidiClockListener* l) = 0;

	//virtual bool setInput(const char* name) = 0;
	//virtual const char* getInput() = 0;
	//virtual const char* getInputError() = 0;

	//virtual bool setOutput(const char* name) = 0;
	//virtual const char* getOutput() = 0;
	//virtual const char* getOutputError() = 0;

	//virtual bool setThrough(const char* name) = 0;
	//virtual const char* getThrough() = 0;
	//virtual const char* getThroughError() = 0;
	//virtual void setThroughMap(class MidiMap* map) = 0;

    // this one we may need to implement
	virtual class MidiEvent* newEvent(int status, int chan, int value, int vel) = 0;

	virtual void send(class MidiEvent* e) = 0;
	virtual void send(unsigned char e) = 0;
	virtual void echo(class MidiEvent* e) = 0;

	// timer
	virtual bool timerStart() = 0;
	virtual long getMilliseconds() = 0;
	virtual int getMidiClocks() = 0;
	virtual float getMillisPerClock() = 0;

	// tempo monitor
	virtual float getInputTempo() = 0;
	virtual int getInputSmoothTempo() = 0;

	// sync out
	virtual void setOutputTempo(float bpm) = 0;
	virtual float getOutputTempo() = 0;
	virtual void midiStart() = 0;
	virtual void midiStop(bool stopClocks) = 0;
	virtual void midiContinue() = 0;
	virtual void startClocks(float tempo) = 0;
	virtual void stopClocks() = 0;

	// diagnostics

	virtual void printEnvironment() = 0;
	virtual void printStatistics() = 0;
	virtual const char* getLastError() = 0;

  protected:


};

//////////////////////////////////////////////////////////////////////
//
// StubMidiInterface
//
//////////////////////////////////////////////////////////////////////

class StubMidiInterface : public MidiInterface {

  public:

	StubMidiInterface() {}
	~StubMidiInterface() {}

	void setListener(class MidiEventListener* l) {
        mEventListener = l;
    }
    
	void setClockListener(class MidiClockListener* l) {
        mClockListener = l;
    }

	class MidiEvent* newEvent(int status, int chan, int value, int vel) {
        return nullptr;
    }

	void send(class MidiEvent* e) {
    }
    
	void send(unsigned char e) {
    }
    
	void echo(class MidiEvent* e) {
    }

	// timer
	bool timerStart() {
        return true;
    }
    
	long getMilliseconds() {
        return 0;
    }

	int getMidiClocks() {
        return 0;
    }
    
	float getMillisPerClock() {
        return 0.0f;
    }

	// tempo monitor
	float getInputTempo() {
        return 120.0f;
    }
    
	int getInputSmoothTempo() {
        return 120;
    }

	// sync out
	void setOutputTempo(float bpm) {
    }
    
	float getOutputTempo() {
        return 120.0f;
    }
    
	void midiStart() {}
	void midiStop(bool stopClocks) {}
	void midiContinue() {}
	void startClocks(float tempo) {}
	void stopClocks() {}

	// diagnostics

	void printEnvironment() {}
	void printStatistics() {}
	const char* getLastError() {
        return nullptr;
    }

  protected:

	class MidiEventListener* mEventListener = nullptr;
	class MidiClockListener* mClockListener = nullptr;

};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

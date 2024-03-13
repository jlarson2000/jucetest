/**
 * Random code fragments pulled out of Mobius.cpp to make it
 * cleaner.  This file doesn't do anything but I don't
 * want to delete the code just yet.
 */

/**
 * Special latency calibration interface.
 *
 * Would still like to have this but it is wound up with threads
 * that don't exist and Recorder, so need to redesign.
 */
#if 0

    // calibration needs to be moved somewhere
	class CalibrationResult* calibrateLatency();


CalibrationResult* Mobius::calibrateLatency()
{
	CalibrationResult* result = NULL;

	if (mRecorder != NULL) {
		// disable this since we won't be receiving interrupts
		// during the test
		if (mThread != NULL)
		  mThread->setCheckInterrupt(false);


        // ugh, stilly duplicate structures so the UI doesn't have to
        // be aware of Recorder and Recorder doesn't have to be aware of
        // Mobius.  Refactor this!
   
        RecorderCalibrationResult* rcr = mRecorder->calibrate();
        result = new CalibrationResult();
        result->timeout = rcr->timeout;
        result->noiseFloor = rcr->noiseFloor;
        result->latency = rcr->latency;
        delete rcr;

		// turn it back on
		mInterrupts++;
		if (mThread != NULL)
		  mThread->setCheckInterrupt(true);
	}

	return result;
}
#endif

/**
 * Return a list of Actions for each Script that used the !button declaration.
 * This is a kludge to get buttons for scripts automatically added to the UI
 * so we don't have to do it manually. I ALWAYS want this so I win.  The UI
 * is expected to call this at appropriate times, like initialization and
 * whenever the script config changes.  The actions become owned by the caller
 * and must be returned to the pool.
 *
 * UPDATE: This should be done up in the UI with other binding management.
 */
#if 0
Action* Mobius::getScriptButtonActions()
{
    Action* actions = NULL;
    Action* last = NULL;

    for (Script* script = mScriptEnv->getScripts() ; script != NULL ;
         script = script->getNext()) {

        if (script->isButton()) {
            Function* f = script->getFunction();
            if (f != NULL) {

                // resolution is still messy, need more ways
                // to get a ResolvedTarget
                Binding* b = new Binding();
                b->setTrigger(TriggerUI);
                b->setTarget(TargetFunction);
                b->setName(f->getName());
                
                ResolvedTarget* t = resolveTarget(b);
                if (t != NULL) {
                    Action* action = new Action(t);

                    resolveTrigger(b, action);

                    if (last != NULL)
                      last->setNext(action);
                    else
                      actions = action;
                    last = action;
                }
                delete b;
            }
        }
    }

    return actions;
}
#endif

//////////////////////////////////////////////////////////////////////
//
// Old stuff I omitted but needed at the end to resolve link errors
// Mostly seems to be related to MIDI export
//
//////////////////////////////////////////////////////////////////////

/**
 * Create an Export for the target of an Action.
 * Export lost the ResolvedTarget so if we still need this
 * will need another way to resolve it
 */
#if 0
Export* Mobius::resolveExport(Action* a)
{
    return resolveExport(a->getResolvedTarget());
}

/**
 * Create an Export for a ResolvedTarget.
 * This is the core export resolver used by all the other
 * resolution interfaces.
 *
 * Returns NULL if the target can't be exported.  This is 
 * okay since OscRuntime calls this for everything.
 */
Export* Mobius::resolveExport(ResolvedTarget* resolved)
{
    Export* exp = NULL;
    bool exportable = false;

    ActionType* t = resolved->getTarget();

    if (t == ActionParameter) {
        // Since OSC is configured in text, ignore some things
        // we don't want to get out
        Parameter* p = (Parameter*)resolved->getObject();
        exportable = (p->bindable || p->control);
    }
    
    if (exportable) {
        exp = new Export(this);
        exp->setTarget(resolved);
        // nothing else to save, Export has logic to call
        // back to us for interesting things
    }

    return exp;
}
#endif


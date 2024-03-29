/**
 * Encapsulates most of the core code related to scripts.
 *
 * There are two parts to this: compilation and runtime.
 * Would like to split compilation into something more self
 * contained that doesn't drag in runtime dependencies
 * and defer linkage to internal objects like Function
 * and Parameter.
 *
 */

#include "../../util/Trace.h"
#include "../../model/MobiusConfig.h"
#include "../../model/ScriptConfig.h"

#include "Script.h"
#include "ScriptCompiler.h"
#include "ScriptRuntime.h"
#include "Function.h"

#include "Scriptarian.h"
#include "Mem.h"

Scriptarian::Scriptarian(Mobius* argMobius)
{
    mMobius = argMobius;
    mLibrary = nullptr;
    mFunctions = nullptr;
    mRuntime = NEW1(ScriptRuntime, mMobius);
}

Scriptarian::~Scriptarian()
{
    delete mLibrary;
    delete mRuntime;

    // this deletes the array but not the dynamically allocated RunScriptFunctions
    // those are owned by Script and will be deleted when the Library is deleted
    delete mFunctions;
}

/**
 * Used only by Mobius for propagateFunctionPreferences
 */
Function** Scriptarian::getFunctions()
{
    return mFunctions;
}

/**
 * Compile the scripts referenced in a ScriptConfig, link
 * them to Function and Parameter objects, and build out the
 * combined Function array containing both static and script functions.
 *
 * This is used by the Shell to do all of the memory allocation and
 * syntax analysis outside the audio thread.  It will later be passed
 * down to the core for installation.
 *
 * For historical reasons, this needs a Mobius to operate for
 * reference resolution.  The compilation process must have NO side
 * effects on the core runtime state.  It is allowed to get the
 * MobiusConfig from Mobius, but this may not be where this
 * ScriptConfig came from.
 *
 */
void Scriptarian::compile(ScriptConfig* src)
{
    ScriptCompiler* sc = NEW(ScriptCompiler);

    // revisit the interface, rather than passing mMobius can
    // we pass ourselves intead?
    // it will want to look up Functions but also Parameters
    mLibrary = sc->compile(mMobius, src);
    delete sc;

    // rebuild the global Function table to include top-level scripts
    // this needs more thought
    // try to defer or avoid the build out of RunScriptFunctions,
    // and/or just keep a different array for script functions
    initializeFunctions();

    // old code had initScriptParameters here, removed since that didn't
    // seem to do anything

    // !! need a way to pass the compiler error list back up
}    

/**
 * Called during the initialization process to set up scripts.
 * Note that this is the one place core code is allowed to do
 * complex memory allocation, like compiling scripts.
 * Once initialized, new ScriptLibrary objects must be passed
 * through KernelMessages.
 *
 * This is an older interface I'm not entirely happy with.
 * Consider making scripts be more like samples and defer compilation
 * until after the initialize() process.
 */
void Scriptarian::initialize(MobiusConfig* config)
{
    ScriptConfig* scriptConfig = config->getScriptConfig();
    
    compile(scriptConfig);
}

/**
 * Set up the consolidated Function array containing both
 * static Function pointers, and generated RunScriptFunctions
 * for each of the callable scripts.
 *
 * RunScriptFunction is obscure.
 * Script owns a RunScriptFunction which we usually allocate here
 * and give it.  If there are cross references between scripts,
 * one may have already been allocated during script linking.  In both
 * cases the Script owns the RunScriptFunction and will delete it.
 */
void Scriptarian::initializeFunctions()
{
    Function** functions = NULL;
	int i;

    // should already be initialized but make sure
    Function::initStaticFunctions();

	// first count the static functions
	// eventually make loop and track triggers dynamnic too
	int staticCount = 0;
	for ( ; StaticFunctions[staticCount] != NULL ; staticCount++);

    // generate RunScriptFunctions for the scripts that can be called
    List* scriptFunctions = NEW(List);
    
    Script* scripts = mLibrary->getScripts();
    while (scripts != nullptr) {
        if (!scripts->isHide()) {
            // may already have a function if we had a cross reference
            RunScriptFunction* f = scripts->getFunction();
            if (f == NULL) {
                f = NEW1(RunScriptFunction, scripts);
                scripts->setFunction(f);
            }
            scriptFunctions->add(f);
        }
        scripts = scripts->getNext();
    }

    // allocate a new array
    int total = staticCount + scriptFunctions->size() + 1;
	functions = new Function*[total];
    MemTrack(functions, "Scriptarian:functions", total * sizeof(Function*));
    // add statics
    int psn = 0;
    for (i = 0 ; i < staticCount ; i++)
	  functions[psn++] = StaticFunctions[i];

    // add scripts
    for (i = 0 ; i < scriptFunctions->size() ; i++)
      functions[psn++] = (RunScriptFunction*)scriptFunctions->get(i);

    // and terminate it
    functions[psn] = NULL;

    // remember it
    mFunctions = functions;

    // temporary list
    delete scriptFunctions;
}

//////////////////////////////////////////////////////////////////////
//
// Function Lookup
//
//////////////////////////////////////////////////////////////////////

/**
 * Search the dynamic function list.
 * This is only used by Script for an obscure wait state.
 * Comments indiciate that "Wait function" was never used and may not
 * work, but there is other code around waiting for a Switch to happen.
 * Need to sort this out.
 *
 * Now that this is encapsulated in Scriptarian find a way for
 * Scripts to reference that instead so we don't need this
 * pass through.
 */
Function* Scriptarian::getFunction(const char * name)
{
    Function* found = Function::getFunction(mFunctions, name);
    
    // one last try with hidden functions
    // can't we just have a hidden flag for these rather than
    // two arrays?
    if (found == NULL)
      found = Function::getFunction(HiddenFunctions, name);

    return found;
}

//////////////////////////////////////////////////////////////////////
//
// Runtime Pass Throughs
//
// We've got three layers of this now, and I'm unconfortable
// Mobius is what most of the system calls and it passes to Scriptarian
// Scriptarian passes to ScriptRuntime
//
//////////////////////////////////////////////////////////////////////

void Scriptarian::doScriptMaintenance()
{
    mRuntime->doScriptMaintenance();
}

void Scriptarian::finishEvent(KernelEvent* e)
{
    mRuntime->finishEvent(e);
}

/**
 * RunScriptFunction global function handler.
 * RunScriptFunction::invoke calls back to to this.
 */
void Scriptarian::runScript(Action* action)
{
    // everything is now encapsulated in here
    mRuntime->runScript(action);
}

void Scriptarian::resumeScript(Track* t, Function* f)
{
    mRuntime->resumeScript(t, f);
}

void Scriptarian::cancelScripts(Action* action, Track* t)
{
    mRuntime->cancelScripts(action, t);
}

/**
 * Convey a message to the UI from a Script.
 * This isn't necessarily just for scripts, think about other uses
 * for this now that we have it
 *
 * !! How did this ever work, we can't just call listeners from within
 * the audio thread.
 */
void Scriptarian::addMessage(const char* msg)
{
	//if (mListener != NULL)
    //mListener->MobiusMessage(msg);
}

/**
 * Used by Mobius to phase in a new Scriptarian containing
 * a newly loaded Script model.  This can't be done if any
 * Scripts are still running.
 */
bool Scriptarian::isBusy()
{
    return mRuntime->isBusy();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/**
 * Analyze the ScriptConfig so we can return information about the scripts
 * back to the UI for binding management and to display error messages.
 * 
 * This has a mess of unusual interconnections because we're connecting
 * new UI code with very old core code, but at least it's encapsulated
 * in one place.
 *
 * The analysis is performed using ScriptCompiler which ordinarlly
 * runs deep in the kernel, but I don't want to add a bunch of transition
 * methods yet to get down there.  ScriptCompiler is relatively self contained
 * except that it requires a Mobius to do reference resolution so we have to dig
 * that out until this can be redesigned to do link/resolution later when
 * the compiled scripts are installed.
 *
 * We're only doing compilation here with no kernel side effects so it's
 * safe if dangerous looking.
 *
 */

#include "../util/Trace.h"
#include "../model/ScriptConfig.h"
#include "../model/ScriptAnalysis.h"

#include "MobiusShell.h"
#include "MobiusKernel.h"

#include "core/Script.h"
#include "core/ScriptCompiler.h"
#include "core/Mobius.h"

#include "ScriptAnalyzer.h"

ScriptAnalyzer::~ScriptAnalyzer()
{
    delete mAnalysis;
    delete mLibrary;
}

/**
 * Return the analysis
 * Ownership passes to the caller which must delete it.
 *
 * todo: do we need both get/take methods here?
 */
ScriptAnalysis* ScriptAnalyzer::takeAnalysis()
{
    ScriptAnalysis* anal = mAnalysis;
    mAnalysis = nullptr;
    return anal;
}

ScriptLibrary* ScriptAnalyzer::takeLibrary()
{
    ScriptLibrary* lib = mLibrary;
    mLibrary = nullptr;
    return lib;
}

/**
 * Do the analysis.
 * Could try to break up the analysis from the full compilation
 * but the UI is likely going to want error messages along with
 * just the script target names so do both.
 *
 * The ScriptConfig remains owned by the caller.
 * Normally takeAnalysis is immediately called to pass it back up.
 */
void ScriptAnalyzer::analyze(ScriptConfig* config)
{
    // start over if we had lingering results
    // we're rebuilding this every time so don't really need to
    // save them, but might want to cache
    delete mAnalysis;
    mAnalysis = nullptr;
    delete mLibrary;
    mLibrary = nullptr;

    ScriptCompiler* compiler = new ScriptCompiler();

    // this is the awkward part, ScriptCompiler is old and requires
    // a Mobius for reference resolution, that's supposed to be hidden
    // I don't want to add a bunch of fowarding methods just yet so
    // let's take the unusual step of asking the kernel for it
    MobiusKernel* kernel = shell->getKernel();
    Mobius* core = kernel->getCore();
    
    mLibrary = compiler->compile(core, config);
    delete compiler;

    // simplify the ScriptLibrary contents into a ScriptAnalysis
    // don't have error messages yet
    Script* scripts = mLibrary->getScripts();
    while (scripts != nullptr) {
        // Script names are obscure
        // When the compiler creates one it looks for a !name directive
        // and uses that, if not found it will try to derive one from the file name
        // Script::getName is only set if it has a !name, use getDisplayName
        // for the name that must be used to reference it
        const char* bindingName = scripts->getDisplayName();
        if (bindingName != nullptr) {
            if (mAnalysis == nullptr)
              mAnalysis = new ScriptAnalysis();
            mAnalysis->addName(bindingName);
            if (scripts->isButton())
              mAnalysis->addButton(bindingName);

            scripts = scripts->getNext();
        }
        else {
            Trace(1, "ScriptAnalyzer: Unable to determine script name!\n");
        }
    }
    
    // ScriptLibrary currently does not capture compilation errors
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

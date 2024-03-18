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
 * This started using the ScriptAnalysis function to pass back the analysis
 * but I'm now using DynamicConfig.  Get rid of ScriptAnalysis eventually.
 *
 * Keeping the ScriptLibrary in case it becomes useful but don't need
 * it currently.
 *
 */

#include "../util/Trace.h"
#include "../model/ScriptConfig.h"
#include "../model/DynamicConfig.h"

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
 * Do the analysis.
 * Could try to break up the analysis from the full compilation
 * but the UI is likely going to want error messages along with
 * just the script target names so do both.
 *
 * Both the ScriptConfig and DynamicConfig remain owned by the caller.
 * 
 * The internal ScriptLibrary ls left around in case you need it later
 * without a full recompile but not doing that right now.
 */
void ScriptAnalyzer::analyze(ScriptConfig* srcconfig, DynamicConfig* dynconfig)
{
    delete mLibrary;
    mLibrary = nullptr;

    ScriptCompiler* compiler = new ScriptCompiler();

    // this is the awkward part, ScriptCompiler is old and requires
    // a Mobius for reference resolution, that's supposed to be hidden
    // I don't want to add a bunch of fowarding methods just yet so
    // let's take the unusual step of asking the kernel for it
    MobiusKernel* kernel = shell->getKernel();
    Mobius* core = kernel->getCore();
    
    mLibrary = compiler->compile(core, srcconfig);
    delete compiler;

    Script* scripts = mLibrary->getScripts();
    while (scripts != nullptr) {
        // Script names are obscure
        // When the compiler creates one it looks for a !name directive
        // and uses that, if not found it will try to derive one from the file name
        // Script::getName is only set if it has a !name, use getDisplayName
        // for the name that must be used to reference it
        const char* bindingName = scripts->getDisplayName();
        if (bindingName != nullptr) {

            DynamicAction* action = new DynamicAction();
            // todo: need the name vs displayname difference here?
            // as long as the name that is displayed can also be
            // in a UIAction it doesn't matter
            action->name = bindingName;
            action->type = ActionScript;

            // todo: I want scripts to have ordinals for fast lookup
            // this could be just the order they are encountered in the ScriptConfig
            // but I'm not sure how embedded Procs work, should we have several
            // callable script procs per file?
            // keep it simple and have just the name for now
            action->ordinal = 0;

            // todo: one day have more flexible binding suggestions
            action->button = scripts->isButton();

            dynconfig->addAction(action);

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

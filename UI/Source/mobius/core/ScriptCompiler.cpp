/**
 * Factored out of Script to try to keep copmilation code in one place.
 *
 * Still uses stdio, which works and is portable.  Could use Juce at some point.
 * 
 * This can be called in the UI/shell threads, and in the Kernel ONLY
 * during the initialization phase.
 *
 * There was old code in here to support relative paths "so we can distribute
 * examples" and folders.
 *
 * If a folder was in the ScriptConfig we load all .mos files in that folder.
 * That was convenient for testing, but the new UI does not currently support that.
 *
 * We're only going to be getting absolute paths now, and that's the way it
 * should be, leave it to the UI/Container to resolve relative paths if thhat
 * becomes important.
 *
 * It might be nice to auto-load any .mob files left in the installation or configuration
 * folders which would requires MobiusContainer to tell us where that is.
 * 
 */

#include "../../util/Util.h"
#include "../../model/ScriptConfig.h"

#include "Mobius.h"
#include "Script.h"

#include "ScriptCompiler.h"

//////////////////////////////////////////////////////////////////////
//
// ScriptCompiler
//
//////////////////////////////////////////////////////////////////////

ScriptCompiler::ScriptCompiler()
{
    mMobius     = NULL;

    // these are dyamically allocated
    mParser     = NULL;
    mLibrary        = NULL;

    // these are intermediate compile state that will end
    // up in mLibrary
    mScripts    = NULL;
    mLast       = NULL;
    mScript     = NULL;
    mBlock      = NULL;
    mLineNumber = 0;
    strcpy(mLine, "");
}

ScriptCompiler::~ScriptCompiler()
{
    delete mParser;
    // shouldn't be dangling
    delete mLibrary;
}

/**
 * Compile a ScriptConfig into a ScriptLibrary.
 * 
 * The returned ScriptLibrary is completely self contained and only has
 * references to static objects like Functions and Parameters.
 * 
 * Mobius is only necessary to resolve references to Parameters and Functions.
 */
ScriptLibrary* ScriptCompiler::compile(Mobius* m, ScriptConfig* config)
{
    // should not try to use this more than once
    if (mLibrary != NULL)
      Trace(1, "ScriptCompiler: dangling library!\n");

    mMobius = m;
    mLibrary = new ScriptLibrary();
    mScripts = NULL;
    mLast = NULL;

    // give it a copy of the config for later diff detection
    mLibrary->setSource(config->clone());

	if (config != NULL) {
		for (ScriptRef* ref = config->getScripts() ; ref != NULL ; 
			 ref = ref->getNext()) {

            // removed path handling relative to the installation and
            // configuration folders, it must be absolute now
			const char* file = ref->getFile();

            // removed support for folders because the old file utils
            // had ugly OS conditionals in them, use Juce for this
            // if we want it again

            // IsFile uses standard library calls so it's easy
			if (IsFile(file)) {
				parse(file);
			}
            else {
                Trace(1, "Invalid script path: %s\n", file);
            }
        }
    }
    
    // Link Phase
    // todo: this I'd like to try deferring until it is
    // actually installed in the kernel
    for (Script* s = mScripts ; s != NULL ; s = s->getNext())
      link(s);
    
    mLibrary->setScripts(mScripts);

    // ownership transfers
    ScriptLibrary* retval = mLibrary;
    mLibrary = nullptr;
    
    return retval;
}

/**
 * Haven't ported this yet...
 *
 * Recompile one script declared with !autoload
 * Keep the same script object so we don't have to mess
 * with substitution in the ScriptLibrary and elsewhere.
 * This should only be called if the script is not currently
 * running, ScriptInterpreter checks that.
 * 
 * NOTE: If we ever start allowing references to Variables
 * and Procs defined between scripts, this will need to be more 
 * complicated.  Sharing Variable declarations could be easily
 * done just by duplicating them in every file with some kind of
 * !include directive, but sharing Procs is not as nice since they
 * can be large and we don't want to duplicate them in every file.
 * Either we have to disallow !autoload of Proc libraries or we
 * need to indirect through a Proc lookup table.
 * 
 */
void ScriptCompiler::recompile(Mobius* m, Script* script)
{
    mMobius = m;

	if (script->isAutoLoad()) {
        const char* filename = script->getFilename();
        if (filename != NULL) {
#if 0            
            FILE* fp = fopen(filename, "r");
            if (fp == NULL) {
                Trace(1, "Unable to refresh script %s\n", filename);
                // just leave it as it was or empty it out?
            }
            else {
                Trace(2, "Re-reading Mobius script %s\n", filename);

                // Get the environment for linking script references
                mLibrary = script->getLibrary();

                if (!parse(fp, script)) {
                    // hmm, could try to splice it out of everywhere
                    // but just leave it, we don't have a mechanism
                    // for returning inner script errors anyway
                }
                else {
                    // relink just this script
                    // NOTE: if we get around to supporting Proc refs
                    // then we may need to relink all the *other* scripts
                    // as well
                    link(script);
                }

                fclose(fp);
            }
#endif
        }
    }
}

/**
 * Final link phase for one script.
 */
void ScriptCompiler::link(Script* s)
{
    // zero means we're in the link phase
    mLineNumber = 0;
    strcpy(mLine, "");

    // save for callbacks to parseExpression and other utilities
    mScript = s;

    s->link(this);
}

/**
 * Internal helper used when processing something from the script config
 * we know is an individual file.
 */
void ScriptCompiler::parse(const char* filename)
{
    if (!IsFile(filename)) {
		Trace(1, "Unable to locate script file %s\n", filename);
	}
	else {
        FILE* fp = fopen(filename, "r");
        if (fp == NULL) {
			Trace(1, "Unable to open file: %s\n", filename);
		}
        else {
			Trace(2, "Reading Mobius script %s\n", filename);

            Script* script = new Script(mLibrary, filename);
            
			// remember the directory, for later relative references
			// within the script
			// !! don't need this any more?
            int psn = strlen(filename) - 1;
            while (psn > 0 && filename[psn] != '/' && filename[psn] != '\\')
                psn--;

            if (psn == 0) {
                // should only happen if we're using relative paths
            }
            else {
                // leave the trailing slash
                script->setDirectoryNoCopy(CopyString(filename, psn));
            }

            if (parse(fp, script)) {
                if (mScripts == NULL)
                  mScripts = script;
                else
                  mLast->setNext(script);
                mLast = script;
            }
            else {
                delete script;
            }

            fclose(fp);
        }
    }
}

bool ScriptCompiler::parse(FILE* fp, Script* script)
{
	char line[SCRIPT_MAX_LINE + 4];
	char arg[SCRIPT_MAX_LINE + 4];

    mScript = script;
    mLineNumber = 0;
    
    if (mParser == NULL)
      mParser = new ExParser();

    // if here on !authload, remove current contents
    // NOTE: If this is still being interpreted someone has to wait
    // for all threads to finish...how does THAt work?
	mScript->clear();

    // start by parsing in to the script block
    mBlock = mScript->getBlock();

	while (fgets(line, SCRIPT_MAX_LINE, fp) != NULL) {

        if (mLineNumber == 0 && 
            (line[0] == (char)0xFF || line[0] == (char)0xFE)) {
            // this looks like a Unicode Byte Order Mark, we could
            // probably handle these but skip them for now
            Trace(1, "Script %s: Script appears to contain multi-byte unicode\n", 
                  script->getTraceName());
            break;
        }

        // we may be tokenizing the line, save a copy here
        // in case the statements need it
		strcpy(mLine, line);
		mLineNumber++;

		char* ptr = line;

		// fix? while (*ptr && isspace(*ptr)) ptr++;
		while (isspace(*ptr)) ptr++;
		int len = strlen(ptr);

		if (len > 0) {
			if (ptr[0] == '!') {
				// Script directives
				ptr++;

                if (StartsWithNoCase(ptr, "name")) {
					parseArgument(ptr, "name", arg);
                    script->setName(arg);
                }
                else if (StartsWithNoCase(ptr, "hide") ||
                         StartsWithNoCase(ptr, "hidden"))
                  script->setHide(true);

                else if (StartsWithNoCase(ptr, "autoload")) {
                    // until we work out the dependencies, autoload
                    // and parameter are mutually exclusive
                    if (!script->isParameter())
                      script->setAutoLoad(true);
                }

                else if (StartsWithNoCase(ptr, "button"))
				  script->setButton(true);

                else if (StartsWithNoCase(ptr, "focuslock"))
				  script->setFocusLockAllowed(true);
                
                else if (StartsWithNoCase(ptr, "quantize"))
				  script->setQuantize(true);

                else if (StartsWithNoCase(ptr, "switchQuantize"))
				  script->setSwitchQuantize(true);

                // old name
                else if (StartsWithNoCase(ptr, "controller"))
				  script->setContinuous(true);

                // new preferred name
                else if (StartsWithNoCase(ptr, "continous"))
				  script->setContinuous(true);

                else if (StartsWithNoCase(ptr, "parameter")) {
                    script->setParameter(true);
                    // make sure this stays out of the binding windows
                    script->setHide(true);
                    // issues here, ignore autoload
                    script->setAutoLoad(false);
                }

                else if (StartsWithNoCase(ptr, "sustain")) {
					// second arg is the sustain unit in msecs
					parseArgument(ptr, "sustain", arg);
					int msecs = ToInt(arg);
                    if (msecs > 0)
					  script->setSustainMsecs(msecs);
				}
                else if (StartsWithNoCase(ptr, "multiclick")) {
					// second arg is the multiclick unit in msecs
					parseArgument(ptr, "multiclick", arg);
					int msecs = ToInt(arg);
					if (msecs > 0)
					  script->setClickMsecs(msecs);
				}
                else if (StartsWithNoCase(ptr, "spread")) {
					// second arg is the range in one direction,
					// e.g. 12 is an octave up and down, if not specified
					// use the global default range
					script->setSpread(true);
					parseArgument(ptr, "spread", arg);
					int range = ToInt(arg);
					if (range > 0)
					  script->setSpreadRange(range);
				}
			}
			else if (ptr[0] != '#' && len > 1) {
				if (ptr[len-1] == '\n')
				  ptr[len-1] = 0;
				else {
					// actually this is common on the last line of the file
					// if it wasn't terminated
					//Trace(1, "Script line too long!\n");
					//Trace(1, "--> '%s'\n", ptr);
				}

                ScriptStatement* s = parseStatement(ptr);
                if (s != NULL) {
                    s->setLineNumber(mLineNumber);

                    if (s->isEndproc() || s->isEndparam()) {
                        // pop the stack
                        // !! hey, should check to make sure we have the
                        // right ending, currently Endproc can end a Param
                        if (mBlock != NULL && mBlock->getParent() != NULL) {
                            mBlock = mBlock->getParent();
                        }
                        else {
                            // error, mismatched block ending
                            const char* msg;
                            if (s->isEndproc())
                              msg = "Script %s: Mismatched Proc/Endproc line %ld\n";
                            else
                              msg = "Script %s: Mismatched Param/Endparam line %ld\n";

                            Trace(1, msg, script->getTraceName(), (long)mLineNumber);
                        }

                        // we don't actually need these since the statements 
                        // are nested
                        delete s;
                    }
                    else {
                        // add the statement to the block
                        mBlock->add(s);

                        if (s->isProc() || s->isParam()) {
                            // push a new block 
                            ScriptBlockingStatement* bs = (ScriptBlockingStatement*)s;
                            ScriptBlock* block = bs->getChildBlock();
                            block->setParent(mBlock);
                            mBlock = block;
                        }
                    }
                }
            }
		}
	}

    // do internal resolution
    script->resolve(mMobius);

    // TODO: do some sanity checks, like looking for Param
    // statements in a script that isn't declared with !parameter

    // don't have an error return case yet, safer to ignore harmless
    // parse errors and let the script do the best it can?
    return true;
}

/**
 * Helper for script declaration argument parsing.
 * Given a line with a declaration and the keyword, skip over the keyword 
 * and copy the argument into the given buffer.
 * Be sure to trim both leading and trailing whitespace, this is especially
 * important when moving scripts from Windows to Mac because Widnows
 * uses the 0xd 0xa newline convention but Mac file streams only strip 0xa.
 * Failing to strip the 0xd can cause script name mismatches.
 */
void ScriptCompiler::parseArgument(char* line, const char* keyword, 
                                           char* buffer)
{
	char* ptr = line + strlen(keyword);

	ptr = TrimString(ptr);

	strcpy(buffer, ptr);
}

ScriptStatement* ScriptCompiler::parseStatement(char* line)
{
    ScriptStatement* stmt = NULL;
	char* keyword;
	char* args;

    // parse the initial keyword
	keyword = parseKeyword(line, &args);

	if (keyword != NULL) {

        if (StartsWith(keyword, "!") || EndsWith(keyword, ":"))
          parseDeclaration(keyword, args);

        else if (StringEqualNoCase(keyword, "echo"))
          stmt = new ScriptEchoStatement(this, args);

        else if (StringEqualNoCase(keyword, "message"))
          stmt = new ScriptMessageStatement(this, args);

        else if (StringEqualNoCase(keyword, "prompt"))
          stmt = new ScriptPromptStatement(this, args);

        else if (StringEqualNoCase(keyword, "end"))
          stmt = new ScriptEndStatement(this, args);
        
        else if (StringEqualNoCase(keyword, "cancel"))
          stmt = new ScriptCancelStatement(this, args);
        
        else if (StringEqualNoCase(keyword, "wait"))
          stmt = new ScriptWaitStatement(this, args);

        else if (StringEqualNoCase(keyword, "set"))
          stmt = new ScriptSetStatement(this, args);

        else if (StringEqualNoCase(keyword, "use"))
          stmt = new ScriptUseStatement(this, args);
        
        else if (StringEqualNoCase(keyword, "variable"))
          stmt = new ScriptVariableStatement(this, args);
        
        else if (StringEqualNoCase(keyword, "jump"))
          stmt = new ScriptJumpStatement(this, args);
        
        else if (StringEqualNoCase(keyword, "label"))
          stmt = new ScriptLabelStatement(this, args);
        
        else if (StringEqualNoCase(keyword, "for"))
          stmt = new ScriptForStatement(this, args);
        
        else if (StringEqualNoCase(keyword, "repeat"))
          stmt = new ScriptRepeatStatement(this, args);

        else if (StringEqualNoCase(keyword, "while"))
          stmt = new ScriptWhileStatement(this, args);

        else if (StringEqualNoCase(keyword, "next"))
          stmt = new ScriptNextStatement(this, args);
        
        else if (StringEqualNoCase(keyword, "setup"))
          stmt = new ScriptSetupStatement(this, args);

        else if (StringEqualNoCase(keyword, "preset"))
          stmt = new ScriptPresetStatement(this, args);

        else if (StringEqualNoCase(keyword, "unittestsetup"))
          stmt = new ScriptUnitTestSetupStatement(this, args);
        
        else if (StringEqualNoCase(keyword, "initpreset"))
          stmt = new ScriptInitPresetStatement(this, args);

        else if (StringEqualNoCase(keyword, "break"))
          stmt = new ScriptBreakStatement(this, args);
        
        else if (StringEqualNoCase(keyword, "interrupt"))
          stmt = new ScriptInterruptStatement(this, args);
        
        else if (StringEqualNoCase(keyword, "load"))
          stmt = new ScriptLoadStatement(this, args);
        
        else if (StringEqualNoCase(keyword, "save"))
          stmt = new ScriptSaveStatement(this, args);

        else if (StringEqualNoCase(keyword, "call"))
          stmt = new ScriptCallStatement(this, args);

        else if (StringEqualNoCase(keyword, "start"))
          stmt = new ScriptStartStatement(this, args);

        else if (StringEqualNoCase(keyword, "proc"))
          stmt = new ScriptProcStatement(this, args);

        else if (StringEqualNoCase(keyword, "endproc"))
            stmt = new ScriptEndprocStatement(this, args);

        else if (StringEqualNoCase(keyword, "param"))
          stmt = new ScriptParamStatement(this, args);

        else if (StringEqualNoCase(keyword, "endparam"))
            stmt = new ScriptEndparamStatement(this, args);

        else if (StringEqualNoCase(keyword, "if"))
          stmt = new ScriptIfStatement(this, args);
        
        else if (StringEqualNoCase(keyword, "else"))
          stmt = new ScriptElseStatement(this, args);
        
        else if (StringEqualNoCase(keyword, "elseif"))
          stmt = new ScriptElseStatement(this, args);
        
        else if (StringEqualNoCase(keyword, "endif"))
          stmt = new ScriptEndifStatement(this, args);
        
        else if (StringEqualNoCase(keyword, "diff"))
          stmt = new ScriptDiffStatement(this, args);
        
        else {
            // assume it must be a function reference
            stmt = new ScriptFunctionStatement(this, keyword, args);
        }

    }

    return stmt;
}

/**
 * Isolate the initial keyword token.
 * The line buffer is modified to insert a zero between the
 * keyword and the arguments.  Also return the start of the arguments.
 */
char* ScriptCompiler::parseKeyword(char* line, char** retargs)
{
	char* keyword = NULL;
	char* args = NULL;

	// skip preceeding whitespace
	while (*line && isspace(*line)) line++;
	if (*line) {
		keyword = line;
		while (*line && !isspace(*line)) line++;
		if (*line != 0) {
			*line = 0;
			args = line + 1;
		}
		if (strlen(keyword) == 0)
		  keyword = NULL;
	}

	// remove trailing carriage-return from Windows
	args = TrimString(args);
	
	*retargs = args;

	return keyword;
}

/**
 * Parse a declaration found within a block.
 * Should eventually use this for parsing the script declarations?
 */
void ScriptCompiler::parseDeclaration(const char* keyword, const char* args)
{
    // mBlock has the containing ScriptBlock
    if (mBlock != NULL) {
        // let's defer complicated parsing since it may be block specific?
        // well it shouldn't be...
        ScriptDeclaration* decl = new ScriptDeclaration(keyword, args);
        mBlock->addDeclaration(decl);
    }
    else {
        // error, mismatched block end
        Trace(1, "Script %s: Declaration found outside block, line %ld\n",
              mScript->getTraceName(), (long)mLineNumber);
    }
}

//////////////////////////////////////////////////////////////////////
//
// Parse/Link Callbacks
//
//////////////////////////////////////////////////////////////////////

Mobius* ScriptCompiler::getMobius()
{   
    return mMobius;
}

/**
 * Return the script currently being compiled or linked.
 */
Script* ScriptCompiler::getScript()
{
    return mScript;
}

/** 
 * Consume a reserved token in an argument list.
 * Returns NULL if the token was not found.  If found it returns
 * a pointer into "args" after the token.
 *
 * The token may be preceeded by whitespace and MUST be followed
 * by whitespace or be on the end of the line.
 *
 * For example when looking for "up" these are valid tokens
 *
 *     something up
 *     something up arg
 *
 * But this is not
 *
 *     something upPrivateVariable
 * 
 * Public so it may be used by the ScriptStatement constructors.
 */
char* ScriptCompiler::skipToken(char* args, const char* token)
{
    char* next = NULL;

    if (args != NULL) {
        char* ptr = args;
        while (*ptr && isspace(*ptr)) ptr++;
        int len = strlen(token);

        if (StringEqualNoCase(args, token, len)) {
            ptr += len;
            if (*ptr == '\0' || isspace(*ptr)) 
              next = ptr;
        }
    }

    return next;
}

/**
 * Internal utility to parse an expression.  This may be called during both
 * the parse and link phases.  During the parse phase mLine and mLineNumber
 * will be valid.  During the link phase these are obtained from the 
 * ScriptStatement which must have saved them.
 *
 * This is only called during the link phase for ScriptFunctionStatement
 * and I'm not sure why. 
 */
ExNode* ScriptCompiler::parseExpression(ScriptStatement* stmt,
                                               const char* src)
{
	ExNode* expr = NULL;

    // should have one by now
    if (mParser == NULL)
      mParser = new ExParser();

	expr = mParser->parse(src);

    const char* error = mParser->getError();
	if (error != NULL) {
		// !! need a console or something for these
		char buffer[1024];
		const char* earg = mParser->getErrorArg();
		if (earg == NULL || strlen(earg) == 0)
		  strcpy(buffer, error);
		else 
		  sprintf(buffer, "%s (%s)", error, earg);

        int line = mLineNumber;
        if (line <= 0) {
            // we must be linking get it from the statement
            line = stmt->getLineNumber();
        }

		Trace(1, "ERROR: %s at line %ld\n", buffer, line);
		Trace(1, "--> file: %s\n", mScript->getFilename());
        // we'll have this during parsing but not linking!
        if (strlen(mLine) > 0)
          Trace(1, "--> line: %s", mLine);
		Trace(1, "--> expression: %s\n", src);
	}

	return expr;
}

/**
 * Generic syntax error callback.
 */
void ScriptCompiler::syntaxError(ScriptStatement* stmt, const char* msg)
{
    int line = mLineNumber;
    if (line <= 0) {
        // we must be linking get it from the statement
        line = stmt->getLineNumber();
    }

    Trace(1, "ERROR: %s at line %ld\n", msg, (long)line);
    Trace(1, "--> file: %s\n", mScript->getFilename());
    // we'll have this during parsing but not linking!
    if (strlen(mLine) > 0)
      Trace(1, "--> line: %s", mLine);
}

/**
 * Resolve references to other scripts during the link phase.
 *
 * !! Would also be interesting to resolve to procs in other scripts
 * so we could have "library" scripts.  This will be a problem for 
 * autoload though because we would have a reference to something
 * within the Script that will become invalid if it loads again.
 * Need an indirect reference on the Script.  Maybe a hashtable
 * keyed by name?
 *
 * Originally it assumed the argument was either an absolute or relative
 * file name.  If relative we would look in the directory containing the
 * parent script.  
 *
 * Now scripts may be referenced in these ways:
 *
 *    - leaf file name without extension: foo
 *    - leaf file name with extension: foo.mos
 *    - value of !name
 *
 * We don't care which directory the referenced script came from, this
 * makes it slightly easier to have duplicate names but usually there
 * will be only one directory. 
 *
 * This can be called in two contexts: when compiling an entire ScriptConfig
 * and when recompiling an individual script with !autoload.  In the
 * first case we only look at the local mScripts list since mLibrary
 * may have things that are no longer configured.  In the second
 * case we must use what is in mLibrary.
 */
Script* ScriptCompiler::resolveScript(const char* name)
{
	Script* found = NULL;

    if (mScripts != NULL) {
        // must be doing a full ScriptConfig compile
        found = resolveScript(mScripts, name);
    }
    else if (mLibrary != NULL) {
        // fall back to ScriptLibrary
        found = resolveScript(mLibrary->getScripts(), name);
    }

    return found;
}

Script* ScriptCompiler::resolveScript(Script* scripts, const char* name)
{
	Script* found = NULL;

    if (name != NULL)  {
        for (Script* s = scripts ; s != NULL ; s = s->getNext()) {

            // check the !name
            // originally case sensitive but since we're insensntive
            // most other places be here too
            const char* sname = s->getName();
            if (StringEqualNoCase(name, sname))
              found = s;
            else {
                // check leaf filename
                const char* fname = s->getFilename();
                if (fname != NULL) {
                    char lname[1024];
                    GetLeafName(fname, lname, true);
						
                    if (StringEqualNoCase(name, lname)) {
                        // exact name match
                        found = s;
                    }
                    else if (EndsWithNoCase(lname, ".mos") && 
                             !EndsWithNoCase(name, ".mos")) {

                        // tolerate missing extensions in the call
                        int dot = LastIndexOf(lname, ".");
                        lname[dot] = 0;
							
                        if (StringEqualNoCase(name, lname))
                          found = s;
                    }
                }
            }
        }
    }

    if (found != NULL) {
		Trace(2, "ScriptLibrary: Reference %s resolved to script %s\n",
			  name, found->getFilename());
	}

	return found;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/**
 * Primary Mobius sub component for managing scripts.
 */

#pragma once

class Scriptarian
{
  public:

    Scriptarian(class Mobius* m);
    ~Scriptarian();

    void initialize(class MobiusConfig* config);

    // advance the runtime on each audio interrupt
    void doScriptMaintenance();

    // notifications about a previously scheduled event finishing
    void finishEvent(class KernelEvent* e);

    // various runtime control
    void runScript(Action* action);
    void resumeScript(Track* t, Function* f);
    void cancelScripts(Action* action, Track* t);
    void addMessage(const char* msg);

    // Function lookup
    Function** getFunctions();
    Function* getFunction(const char * name);
    
  private:

    class Mobius* mMobius;
    class ScriptLibrary* mLibrary;
    class Function** mFunctions;
    class List* mAllocatedFunctions;

    // consider whether this needs to be distinct
    // or can we just merge it with Scriptarian
    class ScriptRuntime* mRuntime;
    
    void initializeFunctions();

};

/**
 * Primary Mobius sub component for managing scripts.
 */

#pragma once

class Scriptarian
{
  public:

    Scriptarian(class Mobius* m);
    ~Scriptarian();

    // this is where compilation happens
    void compile(class ScriptConfig* src);

    // older interface for compile/install during Mobius construction 
    void initialize(class MobiusConfig* config);

    // access compilation artifacts
    class Function** getFunctions();
    class Function* getFunction(const char * name);

    // Runtime control
    
    void runScript(class Action* action);
    void resumeScript(class Track* t, class Function* f);
    void cancelScripts(class Action* action, class Track* t);
    void addMessage(const char* msg);
    bool isBusy();

    // advance the runtime on each audio interrupt
    void doScriptMaintenance();

    // notifications about a previously scheduled event finishing
    void finishEvent(class KernelEvent* e);

    // library access just for Shell to build the DynamicConfig
    class ScriptLibrary* getLibrary() {
        return mLibrary;
    }

    
  private:

    class Mobius* mMobius;

    // compilation artifacts
    class ScriptLibrary* mLibrary;
    class Function** mFunctions;
    class List* mAllocatedFunctions;

    // consider whether this needs to be distinct
    // or can we just merge it with Scriptarian
    class ScriptRuntime* mRuntime;
    
    void initializeFunctions();

};

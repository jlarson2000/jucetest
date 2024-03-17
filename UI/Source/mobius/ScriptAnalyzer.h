/**
 * Emerging tool to perform script analysis and compilation
 */

#pragma once

class ScriptAnalyzer
{
  public:

    ScriptAnalyzer(class MobiusShell* argShell) {
        shell = argShell;
    }

    ~ScriptAnalyzer();

    void analyze(class ScriptConfig* config);

    class ScriptAnalysis* takeAnalysis();
    class ScriptLibrary* takeLibrary();
                                    
  private:
    
    class MobiusShell* shell;
    class ScriptAnalysis* mAnalysis = nullptr;
    class ScriptLibrary* mLibrary = nullptr;

};

/**
 * Utility class used that encapsultes special code
 * related to the unit tests.
 *
 * Currently there are some special ScriptStatements built-in for
 * testing as well, going forward try to make these normal Functions
 * or DynamicActions and put everything in here.
 */

#pragma once

#include <JuceHeader.h>

class UnitTests
{
  public:

    static UnitTests* Instance;

    UnitTests(class MobiusShell* shell);
    ~UnitTests();

    // entry point from the UnitTestSetup script statement
    void scriptSetup(class KernelEvent* e);

    // entry point from the UnitTestMode global function
    void functionSetup(class UIAction* a);
    
    // called by KernelEventHandler to see if we need to
    // play games with load/save files
    bool isEnabled() {
        return enabled;
    }

    juce::File getSaveLoopFile(class KernelEvent* e);
    juce::File getSaveCaptureFile(class KernelEvent* e);

    void diffAudio(class KernelEvent* e);
    void diffText(class KernelEvent* e);
    
  private:

    void setup();
    void installPresetAndSetup(class MobiusConfig* config);
    void loadConfigOverlay(MobiusConfig* main);

    juce::File getTestRoot();
    juce::File getOutputFile(const char* name);
    juce::File getInputFile(const char* name);

    juce::File getDiffFile(const char* name, bool master);

    class MobiusShell* shell;
    bool enabled = false;
    
};


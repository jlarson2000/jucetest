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

    // called in response to the UnitTestSetups script statement
    // which will schedule an event
    void setup(class KernelEvent* e);

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

    void installPresetAndSetup(class MobiusConfig* config);
    void loadConfigOverlay(MobiusConfig* main);

    juce::File getTestRoot();
    juce::File getOutputFile(const char* name);
    juce::File getInputFile(const char* name);

    juce::File getDiffFile(const char* name, bool master);

    class MobiusShell* shell;
    bool enabled = false;
    
};


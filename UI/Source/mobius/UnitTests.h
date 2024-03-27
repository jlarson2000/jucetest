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
    void actionSetup(class UIAction* a);
    
    // called by KernelEventHandler to see if we need to
    // play games with load/save files
    bool isEnabled() {
        return enabled;
    }

    juce::File getSaveLoopFile(class KernelEvent* e);
    juce::File getSaveCaptureFile(class KernelEvent* e);

    // KernelEventHandler forwards diff events here
    void diffAudio(class KernelEvent* e);
    void diffText(class KernelEvent* e);

    // handler for IntrinsicAnalyzeDiff
    void analyzeDiff(class UIAction* a);
    
    // needed by AudioDifferencer
    class AudioPool* getAudioPool();
    juce::File getResultFile(const char* name);
    juce::File getExpectedFile(const char* name);
    
  private:

    void setup();
    void reloadScripts();
    void installPresetAndSetup(class MobiusConfig* dest);
    void installOverlayParameters(class MobiusConfig* dest, class MobiusConfig* overlay);
    class MobiusConfig* readConfigOverlay();

    juce::File getTestRoot();
    juce::File addExtensionWav(juce::File file);
    juce::File followRedirect(juce::File root);
    juce::String findRedirectLine(juce::String src);

    class MobiusShell* shell;
    bool enabled = false;
    
};


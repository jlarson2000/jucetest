/**
 * Simple utility to locate the root directory where configuration
 * files are expected to be stored.  If a path in a configuration
 * object is relative, use the derived root directory to calculate
 * the full paths to things.
 *
 * Besides trying a few ways to guess the configuration directory
 * once it does it can use a "redirect" file to change the effective
 * root to a random location.  This is how Mobius operates in "unit test mode".
 *
 * The root will normally be a standard configuration directory, but with the
 * addition of a single redirect file, the Mobius configuration files, mobius.xml
 * ui.xml, ScriptConfig paths, SampleConfig paths, etc.  can be redirected elsewhere,
 * typically a folder with a special configuration designed for unit tests without
 * distrupting the configuration for normal use.
 *
 * Typical output of whereAmI on a Windows machine:
 *
 * Current working directory: C:\dev\jucetest\UI\Builds\VisualStudio2022
 * Current executable file: C:\dev\jucetest\UI\Builds\VisualStudio2022\x64\Debug\App\UI.exe
 * Current application file: C:\dev\jucetest\UI\Builds\VisualStudio2022\x64\Debug\App\UI.exe
 * Invoked executable file: C:\dev\jucetest\UI\Builds\VisualStudio2022\x64\Debug\App\UI.exe
 * Host application path: C:\dev\jucetest\UI\Builds\VisualStudio2022\x64\Debug\App\UI.exe
 * User home directory: C:\Users\Jeff
 * User application data directory: C:\Users\Jeff\AppData\Roaming
 * Common application data directory: C:\ProgramData
 * Common documents directory: C:\Users\Public\Documents
 * Temp directory: c:\temp
 * Windows system directory: C:\WINDOWS\system32
 * Global applications directory: C:\Program Files
 * Windows local app data directory: C:\Users\Jeff\AppData\Local
 *
 * Of these current working and executable file are good places to start.
 * User application data and Global appliations are good for when we do installations.
 *     
 *
 */

#include <JuceHeader.h>

// don't like this dependency but VStudio makes printf useless
#include "util/Trace.h"

#include "RootLocator.h"

RootLocator::RootLocator()
{
}

RootLocator::~RootLocator()
{
}

/**
 * Trace various paths we could take.
 */
void RootLocator::whereAmI()
{
    juce::File cwd = juce::File::getCurrentWorkingDirectory();
    trace("Current working directory: %s\n", cwd.getFullPathName().toUTF8());

    juce::File f;

    f = juce::File::getSpecialLocation(juce::File::currentExecutableFile);
    trace("Current executable file: %s\n", f.getFullPathName().toUTF8());
    
    f = juce::File::getSpecialLocation(juce::File::currentApplicationFile);
    trace("Current application file: %s\n", f.getFullPathName().toUTF8());

    f = juce::File::getSpecialLocation(juce::File::invokedExecutableFile);
    trace("Invoked executable file: %s\n", f.getFullPathName().toUTF8());

    f = juce::File::getSpecialLocation(juce::File::hostApplicationPath);
    trace("Host application path: %s\n", f.getFullPathName().toUTF8());

    f = juce::File::getSpecialLocation(juce::File::userHomeDirectory);
    trace("User home directory: %s\n", f.getFullPathName().toUTF8());

    f = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    trace("User application data directory: %s\n", f.getFullPathName().toUTF8());

    f = juce::File::getSpecialLocation(juce::File::commonApplicationDataDirectory);
    trace("Common application data directory: %s\n", f.getFullPathName().toUTF8());

    f = juce::File::getSpecialLocation(juce::File::commonDocumentsDirectory);
    trace("Common documents directory: %s\n", f.getFullPathName().toUTF8());

    f = juce::File::getSpecialLocation(juce::File::tempDirectory);
    trace("Temp directory: %s\n", f.getFullPathName().toUTF8());

    f = juce::File::getSpecialLocation(juce::File::windowsSystemDirectory);
    trace("Windows system directory: %s\n", f.getFullPathName().toUTF8());

    f = juce::File::getSpecialLocation(juce::File::globalApplicationsDirectory);
    trace("Global applications directory: %s\n", f.getFullPathName().toUTF8());

    f = juce::File::getSpecialLocation(juce::File::windowsLocalAppData);
    trace("Windows local app data directory: %s\n", f.getFullPathName().toUTF8());

}

/**
 * This can eventually support
 */
juce::String RootLocator::getRootPath()
{
    // todo: check environment variables
    
    juce::String root = checkRedirect(juce::File::getCurrentWorkingDirectory());

    if (root.length() == 0)
      root = checkRedirect(juce::File::currentExecutableFile);

    if (root.length() == 0) {
        root = juce::File::getCurrentWorkingDirectory().getFullPathName();
        trace("Unable to locate root redirect file\n");
        trace("Defaulting to %s\n", root);
    }
    
    return root;
}

juce::String RootLocator::checkRedirect(juce::File::SpecialLocationType type)
{
    juce::File f = juce::File::getSpecialLocation(type);
    return checkRedirect(f);
}

juce::String RootLocator::checkRedirect(juce::File path)
{
    juce::String root;
    
    juce::File f = path.getChildFile("mobius-redirect");
    if (f.existsAsFile()) {
        trace("RootLocator: Redirect file found %s\n", f.getFullPathName());
        
        juce::String content = f.loadFileAsString().trim();
        juce::File redirect (content);
        if (redirect.isDirectory()) {
            root = redirect.getFullPathName();
            trace("RootLocator: Redirecting root %s\n", root);
        }
        else {
            trace("RootLocator: Redirect file found, but directory does not exist: %s\n",
                  redirect.getFullPathName());
        }
    }

    return root;
}

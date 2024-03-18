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
 * This we can consider:
 *
 *     juce::File::getCurrentWorkingDirectory
 *       seems to be the .exe file location under VisualStudio
 *       probably the shell cwd from the command line
 *
 *     juce::File::getSpecialLocation(juce::File::userHomeDirectory)
 *     juce::File::getSpecialLocation(juce::File::currentExecutableFile)
 *     juce::File::getSpecialLocation(juce::File::currentApplicationFile)
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
}

/**
 * This can eventually support
 */
juce::String RootLocator::getRootPath()
{
    // this is almost never what you want, but if there is a redirect
    // file there, use it
    juce::File cwd = juce::File::getCurrentWorkingDirectory();
    
    return cwd.getFullPathName();
}

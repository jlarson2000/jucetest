/**
 * Utility to find information about the application runtime
 * environment and locate where configuration files might be.
 */

#pragma once

class RootLocator
{
  public:
    
    RootLocator();
    ~RootLocator();

    static void whereAmI();

    juce::String getRootPath();
    juce::File getRoot();
    
  private:

    juce::File verifiedRoot;

    juce::File checkRedirect(juce::File path);
    juce::File checkRedirect(juce::File::SpecialLocationType type);
    juce::String findRelevantLine(juce::String src);
};


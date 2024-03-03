
#include <JuceHeader.h>

#include "CodeGenerator.h"

class ParameterGenerator
{
  public:
    ParameterGenerator();
    ~ParameterGenerator();

    bool generate(juce::String fileName, bool testMode = false);

  private:

    CodeGenerator code;

    bool expect(juce::XmlElement* el, const char* elementName);
    juce::String require(juce::XmlElement* el, const char* attname);

    bool parseParameters(juce::XmlElement* el);
    bool parseParameter(juce::XmlElement* el);

    void generateOldCode(juce::XmlElement* el);
    void addInitializer(juce::XmlElement* el, const char* name);
    void addOption(juce::XmlElement* el, const char* name);

    juce::String formatCodeName(juce::String xmlName);
    juce::String formatDisplayName(juce::String xmlName);
    juce::String formatScope(juce::String xmlName);
    juce::String formatScopeEnum(juce::String xmlName);
    juce::String formatType(juce::String xmlName);
    juce::String capitalize(juce::String xmlName);

};

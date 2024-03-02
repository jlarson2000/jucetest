/*
  ==============================================================================

    This file contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include <JuceHeader.h>

//==============================================================================
int main (int argc, char* argv[])
{
    juce::String xml = "<parameter name='foo'/>";
    juce::XmlDocument doc(xml);

    std::unique_ptr<juce::XmlElement> el = doc.getDocumentElement();
    printf("Element %s\n", el->getTagName().toUTF8());


    return 0;
}

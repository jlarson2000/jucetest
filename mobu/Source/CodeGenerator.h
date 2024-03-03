
#pragma once

#include <JuceHeader.h>

class CodeGenerator
{
  public:
    
    CodeGenerator();
    ~CodeGenerator();

    void targetHeader() {
        target = &header;
    }

    void targetCode() {
        target = &code;
    }

    void incIndent() {
        indentLevel += 1;
    }

    void decIndent() {
        if (indentLevel > 0) indentLevel -= 1;
    }

    void noIndent() {
        indentLevel = 0;
    }

    void indent(juce::String src);
    void add(juce::String src);
    void clear();
    
    // file generation
    bool generate(juce::String filename, bool testMode = false);

  private:

    const int indentSpaces = 4;

    juce::String header;
    juce::String code;
    juce::String* target = &header;
    
    int indentLevel = 0;

    bool generate(juce::String fileName, bool testMode, bool header);
    juce::String readFile(juce::String fileName);
    void writeFile(juce::String fileName, juce::String filebuf);
    void addIndent();
};


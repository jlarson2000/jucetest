
#include <iostream>
using std::cout;
using std::endl;

#include <JuceHeader.h>

#include "CodeGenerator.h"

CodeGenerator::CodeGenerator()
{
}

CodeGenerator::~CodeGenerator()
{
}

//////////////////////////////////////////////////////////////////////
//
// Code Buffer Building
//
//////////////////////////////////////////////////////////////////////

void CodeGenerator::clear()
{
    header.clear();
    code.clear();
}

void CodeGenerator::add(juce::String src)
{
    *target += src;
}

void CodeGenerator::indent(juce::String line)
{
    addIndent();
    *target += line;
}

void CodeGenerator::addIndent()
{
    int spaces = indentLevel * indentSpaces;
    for (int i = 0 ; i < spaces ; i++)
      *target += " ";
}

//////////////////////////////////////////////////////////////////////
//
// File Generation
//
//////////////////////////////////////////////////////////////////////

const char* GeneratedToken = "/*** GENERATED ***/";

bool CodeGenerator::generate(juce::String filename, bool testMode)
{
    bool success = false;

    success = generate(filename, testMode, true);
    
    if (success)
      success = generate(filename, testMode, false);
    
    return success;
}

bool CodeGenerator::generate(juce::String fileName, bool testMode, bool isHeader)
{
    juce::String inFile = fileName;
    inFile += (isHeader ? ".h" : ".cpp");

    juce::String filebuf = readFile(inFile);

    filebuf += GeneratedToken;
    filebuf += "\n\n";
    
    if (isHeader)
      filebuf += header;
    else
      filebuf += code;

    juce::String outFile = fileName;
    if (testMode)
      outFile += "Test";
    outFile += (isHeader ? ".h" : ".cpp");

    writeFile(outFile, filebuf);

    return true;
}

juce::String CodeGenerator::readFile(juce::String path)
{
    juce::File file (path);
    juce::String filebuf;
    
    if (!file.existsAsFile()) {
        cout << "Source file empty: " + file.getFullPathName() << endl;
    }
    else {
        cout << "Reading file: " + file.getFullPathName() << endl;
        // so at this point file.loadFileAsString() would read the entire
        // thing, but I'd like to read it a line at a time and stop when we get
        // to the token
        juce::FileInputStream in (file);
        if (!in.openedOk()) {
            cout << "Failed to open file\n";
        }
        else {
            bool tokenFound = false;
            while (!in.isExhausted() && !tokenFound) {
                juce::String line = in.readNextLine();
                if (line.startsWith(GeneratedToken)) {
                    tokenFound = true;
                }
                else {
                    // line does not contain newline terminator
                    filebuf += line;
                    filebuf += "\n";
                }
            }
        }
    }
    
    return filebuf;
}

void CodeGenerator::writeFile(juce::String fileName, juce::String filebuf)
{
    juce::File file (fileName);

    if (!file.existsAsFile()) {
        cout << "Creating file: " + file.getFullPathName() << endl;
    }
    else {
        cout << "Replacing file: " + file.getFullPathName() << endl;
    }
    
    juce::Result result = file.create();
    if (result.failed()) {
        cout << "File creation failed\n";
    }
    else {
        file.replaceWithText(filebuf);
    }
}


#include <iostream>
using std::cout;
using std::endl;

#include <JuceHeader.h>

#include "ParameterGenerator.h"

ParameterGenerator::ParameterGenerator()
{
}

ParameterGenerator::~ParameterGenerator()
{
}

bool ParameterGenerator::generate(juce::String fileName, bool testMode)
{
    // unfortunately when running under the debugger the working directory
    // is where the .exe is, not Source which is where you are when you run
    // it interactively.  Probably a way to set this in VisualStudio but it might
    // get lost when Projucer regenerates the solution
    // juce::File file(juce::File::getCurrentWorkingDirectory().getChildFile(fileName));
    juce::String rootPath = "c:/dev/jucetest/mobu/Source";
    
    juce::File file(juce::File(rootPath).getChildFile(fileName));
    bool success = false;
    
    cout << "Reading definition file: " + file.getFullPathName() << endl;

    // this is where parsing actually happens, if nullptr is returned
    // use getLastParseError
    juce::XmlDocument doc(file);
    std::unique_ptr<juce::XmlElement> el = doc.getDocumentElement();
    if (el == nullptr) {
        cout << "Parse error:" << doc.getLastParseError() << endl;
    }
    else {
        code.clear();
        // two options here, can build a memory model of the parameter definitions
        // then generate code when it parses fully or just generate code on the fly
        // skip having an intermediate model 
        success = parseParameters(el.get());
        if (success) {
            juce::String basePath = rootPath + "/UIParameter";
            success = code.generate(basePath, testMode);
            if (!success)
              cout << "Failed to generate files" << endl;
        }
    }
    return success;
}

bool ParameterGenerator::expect(juce::XmlElement* el, const char* elementName)
{
    bool ok = el->hasTagName(elementName);
    if (!ok) {
        // should be able to get the parent for context
        cout << "Unexpected element name: " + el->getTagName() << endl;
    }
    return ok;
}

juce::String ParameterGenerator::require(juce::XmlElement* el, const char* attname)
{
    juce::String value = el->getStringAttribute(attname);
    if (value.length() == 0) {
        cout << "Missing required " + el->getTagName() + " attribute " + attname << endl;
    }
    return value;
}

bool ParameterGenerator::parseParameters(juce::XmlElement* el)
{
    bool success = false;
    
    if (expect(el, "Parameters")) {
        juce::XmlElement* child = el->getFirstChildElement();
        while (child != nullptr) {
            success = parseParameter(child);
            child = child->getNextElement();
            if (!success)
              break;
        }
    }
    return success;
}

bool ParameterGenerator::parseParameter(juce::XmlElement* el)
{
    bool success = false;
    
    if (expect(el, "Parameter")) {
        juce::String name = require(el, "name");
        if (name.length() > 0) {
            cout << "Parameter " + name << endl;
            juce::String codeName = formatCodeName(name);
            code.targetHeader();
            code.indent("extern UIParameter* UIParameter");
            code.add(codeName);
            code.add(";\n");
            generateOldCode(el);
        }
        success = true;
    }
    return success;
}

/**
 * Now it starts getting hairy.
 * This is the old style of parameter code.
 */
void ParameterGenerator::generateOldCode(juce::XmlElement* el)
{
    juce::String name = require(el, "name");
    juce::String codeName = formatCodeName(name);
    juce::String qualName = "UIParameter" + codeName;
    juce::String className = qualName + "Class";

    code.targetCode();
    code.add("\n////////////// " + codeName + "\n\n");
    
    // class definition

    code.add("class " + className + " : public UIParameter\n");
    code.add("{\n");
    // indent always does 4 and public: or probably anything
    // with a colon, emacs auto indents with 2
    code.add("  public:\n");
    code.incIndent();
    // constructor declaration
    code.indent(className + "();\n");
    // value accessors
    code.indent("void getValue(void* obj, class ExValue* value) override;\n");
    code.indent("void setValue(void* obj, class ExValue* value) override;\n");
    code.decIndent();
    code.add("};\n");

    // dfeault thesel
    juce::String typeValue = el->getStringAttribute("type");
    if (typeValue.length() == 0)
      typeValue = "int";

    juce::String scopeValue = el->getStringAttribute("scope");
    if (scopeValue.length() == 0)
      scopeValue = "preset";
    
    // constructor
    
    juce::String typeCodeName = formatType(typeValue);
    juce::String scopeClassName = formatScope(scopeValue);
    
    code.add(className + "::" + className + "()\n");
    code.add("{\n");
    code.incIndent();
    // here we might need to substitute our desired name for what old code wants
    code.indent("name = \"" + name + "\";\n");
    code.indent("displayName = \"" + formatDisplayName(name) + "\";\n");
    // note that this isn't the code class name, it's just the upcased xml name
    code.indent("scope = Scope" + formatScopeEnum(scopeValue) + ";\n");
    code.indent("type = Type" + typeCodeName + ";\n");
    addInitializer(el, "low");
    addInitializer(el, "high");
    addInitializer(el, "defaultValue");
    addOption(el, "multi");
    addOption(el, "dynamic");
    addOption(el, "zeroCenter");
    addOption(el, "control");
    addOption(el, "transient");
    addOption(el, "juceValues");

    // todo: coreName
    code.decIndent();
    code.add("}\n");

    // getValue

    code.add("void " + className + "::getValue(void* obj, ExValue* value)\n");
    code.add("{\n");
    code.incIndent();
    code.indent("value->set" + typeCodeName + "(((");
    code.add(scopeClassName + "*)obj)->get" + codeName + "());\n");
    code.decIndent();
    code.add("}\n");
    
    // setValue
    
    code.add("void " + className + "::setValue(void* obj, ExValue* value)\n");
    code.add("{\n");
    code.incIndent();
    code.indent("((" + scopeClassName + "*)obj->set" + codeName +  "(");
    code.add("value->get" + typeCodeName + "());\n");
    code.decIndent();
    code.add("}\n");

    // static object
    juce::String objName = qualName + "Obj";
    code.add(className + " " + objName + ";\n");
    // and finally our pointer
    code.add("UIParameter* " + qualName + " = " + "&" + objName + ";\n");
}

void ParameterGenerator::addInitializer(juce::XmlElement* el, const char* name)
{
    juce::String jname(name); 
    if (el->hasAttribute(jname)) 
      code.add(jname + " = " + el->getStringAttribute(jname) + ";\n");
}

void ParameterGenerator::addOption(juce::XmlElement* el, const char* name)
{
    juce::String opname(name);
    
    if (el->hasAttribute("options")) {
        // this will be a csv but we don't have to split it up
        juce::String options = el->getStringAttribute("options");
        if (options.contains(opname)) {
            code.indent(opname + " = true;\n");
        }

    }
}

juce::String ParameterGenerator::capitalize(juce::String xmlName)
{
    juce::String capName;
    
    if (xmlName.length() > 0) {
        capName += juce::CharacterFunctions::toUpperCase(xmlName[0]);
        capName += xmlName.substring(1);
    }
    return capName;
}    

/**
 * "code" name is the name used when combined with the get/set
 * functions on the target class.  It is usually just
 * an initial capital conversion.
 *
 * Probably going to have some more complex substitutions.
 */
juce::String ParameterGenerator::formatCodeName(juce::String xmlName)
{
    return capitalize(xmlName);
}

/**
 * Display name rules are initial capital followed by space
 * delimited words for each capital in the internal name.
 */
juce::String ParameterGenerator::formatDisplayName(juce::String xmlName)
{
    juce::String displayName;

    for (int i = 0 ; i < xmlName.length() ; i++) {
        juce::juce_wchar ch = xmlName[i];
        
        if (i == 0) {
            ch = juce::CharacterFunctions::toUpperCase(ch);
        }
        else if (juce::CharacterFunctions::isUpperCase(ch)) {
            displayName += " ";
        }
        displayName += ch;
    }

    return displayName;
}

/**
 * Convert the XML scope name to the name of one of the Scope
 * constants.  Just an upcase.
 */
juce::String ParameterGenerator::formatScopeEnum(juce::String xmlName)
{
    return capitalize(xmlName);
}

/**
 * Convert the XML scope name to the name of the target class.
 */
juce::String ParameterGenerator::formatScope(juce::String xmlName)
{
    const char* className = "???";

    if (xmlName == juce::String("global"))
      className = "MobiusConfig";
    
    else if (xmlName == juce::String("preset"))
      className = "Preset";
    
    else if (xmlName == juce::String("setup"))
      className = "Setup";
      
    else if (xmlName == juce::String("track"))
      className = "SetupTrack";
      
    else
      className = "???";
        
    return juce::String(className);
}

/**
 * Format an XML type name into the name used in the ExValue get/set functions
 * enums are going to be more complicated
 */
juce::String ParameterGenerator::formatType(juce::String xmlName)
{
    const char* typeName = "???";

    if (xmlName == juce::String("int"))
      typeName = "Int";

    else if (xmlName == juce::String("bool"))
      typeName = "Boolean";

    else if (xmlName == juce::String("string"))
      typeName = "String";

    else if (xmlName == juce::String("enum")) {
        // this becomes Int with extra casting
        typeName = "Int";
    }

    return juce::String(typeName);
}

      
             

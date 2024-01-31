
#include "../util/Trace.h"
#include "../util/FileUtil.h"

#include "../model/MobiusConfig.h"
#include "../model/Parameter.h"
#include "../model/XmlRenderer.h"

#include "Mobius.h"

// configure this somehow
const char* ConfigFilePath = "c:/dev/jucetest/UI/Source/mobius.xml";
const char* ConfigFilePathTempSave = "c:/dev/jucetest/UI/Source/mobius-save.xml";

Mobius::Mobius()
{
}

Mobius::~Mobius()
{
}

MobiusConfig* Mobius::editConfiguration()
{
    // we're not using one yet so just return
    // what we read
    return readConfiguration();
}

MobiusConfig* Mobius::readConfiguration()
{
    MobiusConfig* config = nullptr;
    

    char* xml = ReadFile(ConfigFilePath);
    if (xml != nullptr) {
        XmlRenderer r;
        config = r.parseMobiusConfig(xml);
    }

    delete xml;

    return config;
}

void Mobius::saveConfiguration(MobiusConfig* config)
{
    XmlRenderer r;
    char* xml = r.render(config);

    WriteFile(ConfigFilePathTempSave, xml);

    delete xml;
    delete config;
}

void Mobius::test()
{
    Trace(1, "Running a test...\n");

    /*
    char* xml = ReadFile("c:/dev/jucetest/UI/Source/mobius.xml");

    MobiusConfig* c = parseMobiusConfig(xml);

    char* xml2 = render(c);
    
    WriteFile("c:/dev/jucetest/UI/Source/mobius2.xml", xml2);

    delete xml2;
    delete xml;
    delete c;
    */

    Parameter::dumpParameters();
}


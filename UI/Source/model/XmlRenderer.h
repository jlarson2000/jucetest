/*
 * An XML generator for configuration objects.
 *
 * MobiusConfig
 *   contains individual global parameters and lists of sub-objects
 *
 * Preset
 *   a collection of operational parameters for functions
 *
 * Setup
 *   a collection of operational parameters for tracks
 *
 * BindingSet
 *   a collection of bindings between external triggers and internal targets
 */

#pragma once

class XmlRenderer {

  public:

    XmlRenderer();
    ~XmlRenderer();

    class MobiusConfig* parseMobiusConfig(const char* xml);
    char* render(class MobiusConfig* c);

    class UIConfig* parseUIConfig(const char* xml);
    char* render(class UIConfig* c);

    class MobiusConfig* clone(class MobiusConfig* src);
    class Preset* clone(class Preset* src);
    class Setup* clone(class Setup* src);

  private:

    // common utilities

    void render(class XmlBuffer* b, class Parameter* p, int value);
    void render(class XmlBuffer* b, class Parameter* p, bool value);
    void render(class XmlBuffer* b, class Parameter* p, const char* value);

    int parse(class XmlElement* e, class Parameter* p);
    const char* parseString(class XmlElement* e, class Parameter* p);

    class StringList* parseStringList(class XmlElement* e);
    void renderList(class XmlBuffer* b, const char* elname, class StringList* list);

    void renderStructure(class XmlBuffer* b, class Structure* s);
    void parseStructure(class XmlElement* e, class Structure* s);

    // main objects

    void render(class XmlBuffer* b, class MobiusConfig* c);
    void parse(class XmlElement* e, class MobiusConfig* c);

    void render(class XmlBuffer* b, class Preset* p);
    void parse(class XmlElement* e, class Preset* p);

    void render(class XmlBuffer* b, class Setup* s);
    void parse(class XmlElement* b, class Setup* s);

    void render(class XmlBuffer* b, class SetupTrack* t);
    void parse(class XmlElement* b, class SetupTrack* t);

    void render(class XmlBuffer* b, class UserVariables* container);
    void parse(class XmlElement* e, class UserVariables* container);

    void parse(class XmlElement* e, class BindingSet* c);
    void render(class XmlBuffer* b, class BindingSet* c);

    void parse(class XmlElement* e, class Binding* c);
    void render(class XmlBuffer* b, class Binding* c);

    void render(class XmlBuffer* b, class ScriptConfig* c);
    void parse(class XmlElement* b, class ScriptConfig* c);

    void render(class XmlBuffer* b, class SampleConfig* c);
    void parse(class XmlElement* b, class SampleConfig* c);

    void render(class XmlBuffer* b, class OscConfig* c);
    void parse(class XmlElement* b, class OscConfig* c);

    void render(class XmlBuffer* b, class OscBindingSet* obs);
    void parse(class XmlElement* e, class OscBindingSet* obs);

    void render(class XmlBuffer* b, class OscWatcher* w);
    void parse(class XmlElement* e, class OscWatcher* w);

    void render(class XmlBuffer* b, class UIConfig* c);
    void parse(class XmlElement* e, class UIConfig* c);
};

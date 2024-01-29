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
 * BindingConfig
 *   a collection of bindings between external triggers and internal targets
 */

#pragma once

class XmlRenderer {

  public:

    XmlRenderer();
    ~XmlRenderer();

    char* render(class Preset* src);

    class Preset* parsePreset(const char* xml);

    void test();

  private:

    void render(class XmlBuffer* b, class Parameter* p, int value);
    void render(class XmlBuffer* b, class Parameter* p, bool value);
    void render(class XmlBuffer* b, class Parameter* p, const char* value);

    int parse(class XmlElement* e, class Parameter* p);
    const char* parseString(class XmlElement* e, class Parameter* p);

    void addBindable(class XmlBuffer* b, class Bindable* bindable);
    void parseBindable(class XmlElement* e, class Bindable* b);

    void render(class XmlBuffer* b, class Preset* p);
    void parse(class XmlElement* e, class Preset* p);

    void render(class XmlBuffer* b, class MobiusConfig* c);
    void parse(class XmlElement* e, class MobiusConfig* c);

    class StringList* parseStringList(class XmlElement* e);
    void renderList(class XmlBuffer* b, const char* elname, class StringList* list);

    void parse(class XmlElement* e, class BindingConfig* c);
    void render(class XmlBuffer* b, class BindingConfig* c);

    void render(class XmlBuffer* b, class Setup* s);
    void parse(class XmlElement* b, class Setup* s);

    void render(class XmlBuffer* b, class ScriptConfig* s);
    void parse(class XmlElement* b, class ScriptConfig* s);

};

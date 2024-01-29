/*
 * Model for configuration scripts to load
 */

#include "ScriptConfig.h"

ScriptConfig::ScriptConfig()
{
    mScripts = NULL;
}

ScriptConfig::~ScriptConfig()
{
	clear();
}

/**
 * Clone for difference detection.
 * All we really need are the original file names.
 */
ScriptConfig* ScriptConfig::clone()
{
    ScriptConfig* clone = new ScriptConfig();
    for (ScriptRef* s = mScripts ; s != NULL ; s = s->getNext()) {
        ScriptRef* s2 = new ScriptRef(s);
        clone->add(s2);
    }
    return clone;
}

void ScriptConfig::clear()
{
    // jebus, why can't refs handle their own chain like everything else
    // will be using std::vector anyway
    ScriptRef* ref = NULL;
    ScriptRef* next = NULL;
    for (ref = mScripts ; ref != NULL ; ref = next) {
        next = ref->getNext();
        delete ref;
    }
	mScripts = NULL;
}

ScriptRef* ScriptConfig::getScripts()
{
    return mScripts;
}

void ScriptConfig::setScripts(ScriptRef* refs) 
{
	clear();
	mScripts = refs;
}

void ScriptConfig::add(ScriptRef* neu) 
{
    ScriptRef* last = NULL;
    for (last = mScripts ; last != NULL && last->getNext() != NULL ; 
         last = last->getNext());

	if (last == NULL)
	  mScripts = neu;
	else
	  last->setNext(neu);
}

void ScriptConfig::add(const char* file) 
{
	add(new ScriptRef(file));
}

/**
 * Utility for difference detection.
 */
bool ScriptConfig::isDifference(ScriptConfig* other)
{
    bool difference = false;

    int myCount = 0;
    for (ScriptRef* s = mScripts ; s != NULL ; s = s->getNext())
      myCount++;

    int otherCount = 0;
    if (other != NULL) {
        for (ScriptRef* s = other->getScripts() ; s != NULL ; s = s->getNext())
          otherCount++;
    }

    if (myCount != otherCount) {
        difference = true;
    }
    else {
        for (ScriptRef* s = mScripts ; s != NULL ; s = s->getNext()) {
            ScriptRef* ref = other->get(s->getFile());
            if (ref == NULL) {
                difference = true;
                break;
            }
        }
    }
    return difference;
}

ScriptRef* ScriptConfig::get(const char* file)
{
    ScriptRef* found = NULL;

    for (ScriptRef* s = mScripts ; s != NULL ; s = s->getNext()) {
        if (StringEqual(s->getFile(), file)) {
            found = s;
            break;
        }
    }
    return found;
}

//////////////////////////////////////////////////////////////////////
//
// ScriptRef
//
//////////////////////////////////////////////////////////////////////

ScriptRef::ScriptRef()
{
    init();
}

ScriptRef::ScriptRef(const char* file)
{
    init();
    setFile(file);
}

ScriptRef::ScriptRef(ScriptRef* src)
{
    init();
    setFile(src->getFile());
}

void ScriptRef::init()
{
    mNext = NULL;
    mFile = NULL;
}

ScriptRef::~ScriptRef()
{
	delete mFile;
}

void ScriptRef::setNext(ScriptRef* ref)
{
    mNext = ref;
}

ScriptRef* ScriptRef::getNext()
{
    return mNext;
}

void ScriptRef::setFile(const char* file)
{
    delete mFile;
    mFile = CopyString(file);
}

const char* ScriptRef::getFile()
{
    return mFile;
}

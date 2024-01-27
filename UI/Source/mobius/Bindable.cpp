
#include "../util/XmlBuffer.h"

#include "Trace.h"
#include "Bindable.h"

/****************************************************************************
 *                                                                          *
 *                              SYSTEM CONSTANT                             *
 *                                                                          *
 ****************************************************************************/

bool TraceCreate = false;

/**
 * This should only be used by constants that need to 
 * initialize themselves in a complex way.  Functions are like this
 * as are some MobiusModes.
 */
SystemConstant::SystemConstant()
{
    init();
}

SystemConstant::SystemConstant(const char* name, const char* displayName)
{
    init();

    if (TraceCreate)
      trace("Creating constant %s\n", name);

    mName = name;
    setDisplayName(displayName);
}

SystemConstant::SystemConstant(const char* name, int key) 
{
    init();

    if (TraceCreate)
      trace("Creating constant %s\n", name);

    mName = name;
    mKey = key;
}

PRIVATE void SystemConstant::init()
{
    mName = nullptr;
    mKey = 0;
    mDisplayName[0] = 0;
    mHelp = nullptr;
}

SystemConstant::~SystemConstant()
{
    if (TraceCreate)
      trace("Deleting constant %s\n", mName);
}

const char* SystemConstant::getName() 
{
    return mName;
}

/**
 * The name is assumed to be static and will not be copied
 * or freed.  This should only be called if you are using
 * the no-arg constructor.
 */
void SystemConstant::setName(const char* name)
{
    if (TraceCreate)
      trace("Creating constant %s\n", name);

    mName = name;
}

int SystemConstant::getKey()
{
    return mKey;
}

void SystemConstant::setKey(int key)
{
    mKey = key;
}

const char* SystemConstant::getDisplayName() 
{
    const char* dname = mDisplayName;

    // if empty fall back to the name
    if (mDisplayName[0] == 0)
      dname = mName;

    return dname;
}

/**
 * The name most likely comes from a message catalog so
 * we keep a private copy.
 */
void SystemConstant::setDisplayName(const char* name)
{
    if (name != nullptr)
      CopyString(name, mDisplayName, sizeof(mDisplayName));
}

/**
 * Look up the display name in a message catalog.
 * Since this is shared by several plugins don't bother
 * localizing if we've done it once.  This does mean that
 * in order to switch languages you will have to bounce 
 * the host.
 */
#if 0
void SystemConstant::localize(MessageCatalog* cat)
{
    if (mKey == 0) {
        // some are allowed to have a static display name
        if (mDisplayName[0] == 0)
          Trace(1, "No catalog key defined for constant %s\n", mName);
    }
    else if (mDisplayName[0] != 0) {
        // already localized, don't do it again
        Trace(2, "Ignoring redundant localization of constant %s\n", mName);
    }
    else {
        const char* msg = cat->get(mKey);
        if (msg != nullptr) {
            setDisplayName(msg);
        }
        else {
            Trace(1, "No localization for constant %s\n", mName);
            setDisplayName(mName);
        }
    }
}
#endif

/**
 * This is currently assumed to be static text so we don't have to 
 * copy it.  This isn't used yet but if we decide to finish it it will
 * need to come from a catalog and be copied here.
 */
void SystemConstant::setHelp(const char* help)
{
    mHelp = help;
}

const char* SystemConstant::getHelp()
{
    return mHelp;
}

/****************************************************************************
 *                                                                          *
 *   							   TARGETS                                  *
 *                                                                          *
 ****************************************************************************/

Target* TargetFunction = new Target("function", "Function");
Target* TargetParameter = new Target("parameter", "Parameter");
Target* TargetSetup = new Target("setup", "Setup");
Target* TargetPreset = new Target("preset", "Preset");
Target* TargetBindings = new Target("bindings", "Bindings");
Target* TargetUIControl = new Target("uiControl", "UI Control");
Target* TargetUIConfig = new Target("uiConfig", "UI Config");
Target* TargetScript = new Target("script", "Script");

Target* Targets[] = {
	TargetFunction,
	TargetParameter,
	TargetSetup,
	TargetPreset,
	TargetBindings,
	TargetUIControl,
	TargetUIConfig,
	TargetScript,
	nullptr
};
 
Target::Target(const char* name, const char* display) :
    SystemConstant(name, display)
{
}

Target* Target::get(const char* name) 
{
	Target* found = nullptr;

    // auto upgrade old bindings
    if (StringEqual(name, "control"))
      name = "parameter";

	if (name != nullptr) {
		for (int i = 0 ; Targets[i] != nullptr ; i++) {
			Target* t = Targets[i];
			if (!strcmp(t->getName(), name)) {
				found = t;
				break;
			}
		}
	}
	return found;
}

/****************************************************************************
 *                                                                          *
 *   							   BINDABLE                                 *
 *                                                                          *
 ****************************************************************************/

#define ATT_NAME "name"
#define ATT_NUMBER "number"

Bindable::Bindable()
{
	mNumber	= 0;
	mName	= nullptr;
}

Bindable::~Bindable()
{
	delete mName;
}

void Bindable::setNumber(int i)
{
	mNumber = i;
}

int Bindable::getNumber()
{
	return mNumber;
}

void Bindable::setName(const char* s)
{
	delete mName;
	mName = CopyString(s);
}

const char* Bindable::getName()
{
	return mName;
}

void Bindable::clone(Bindable* src)
{
	setName(src->mName);
	mNumber = src->mNumber;
}

void Bindable::toXmlCommon(XmlBuffer* b)
{
	// the number is transient on the way to generating a name, 
	// but just in case we don't have a name, serialize it
	if (mName != nullptr)
	  b->addAttribute(ATT_NAME, mName);
	else
	  b->addAttribute(ATT_NUMBER, mNumber);
}

void Bindable::parseXmlCommon(XmlElement* e)
{
	setName(e->getAttribute(ATT_NAME));
	setNumber(e->getIntAttribute(ATT_NUMBER));
}


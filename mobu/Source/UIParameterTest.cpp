/**
 * Base implementation of UIParameter
 * This file is NOT generated
 * Generated subclasses and code are found in UIParameterClasses.cpp
 */

#include <JuceHeader.h>

#include "../util/Util.h"

#include "UIParameter.h"

UIParameter::UIParameter()
{
}

UIParameter::~UIParameter()
{
}

int UIParameter::getOrdinal(void* object)
{
    return 0;
}

void UIParameter::setOrdinal(void* object, int value)
{
}

int UIParameter::getEnumOrdinal(juce::String value)
{
    return 0;
}

int UIParameter::getEnumOrdinal(const char* value)
{
    return 0;
}

const char* UIParameter::getEnumName(int ordinal)
{
    return nullptr;
}

int UIParameter::getEnum(const char *value)
{
    return 0;
}

int UIParameter::getEnumNoWarn(const char *value)
{
    return 0;
}

int UIParameter::getEnum(ExValue *value)
{
    return 0;
}

//////////////////////////////////////////////////////////////////////
//
// Global Parameter Registry
//
//////////////////////////////////////////////////////////////////////

std::vector<UIParameter*> UIParameter::Parameters;

void UIParameter::dumpParameters()
{
    for (int i = 0 ; i < Parameters.size() ; i++) {
        UIParameter* p = Parameters[i];
        // printf("Parameter %s %s %s\n", p->getName(), getEnumLabel(p->type), getEnumLabel(p->scope));
        printf("Parameter %s\n", p->getName());
    }
}

/**
 * Find a Parameter by name
 * This doesn't happen often so we can do a liner search.
 */
UIParameter* UIParameter::find(const char* name)
{
	UIParameter* found = nullptr;
	
	for (int i = 0 ; i < Parameters.size() ; i++) {
		UIParameter* p = Parameters[i];
		if (StringEqualNoCase(p->getName(), name)) {
            found = p;
            break;
        }
	}
	return found;
}

/**
 * Find a parameter by it's display name.
 * I believe this is used only by the Setup editor.
 */
UIParameter* UIParameter::findDisplay(const char* name)
{
	UIParameter* found = nullptr;
	
	for (int i = 0 ; i < Parameters.size() ; i++) {
		UIParameter* p = Parameters[i];
		if (StringEqualNoCase(p->getDisplayName(), name)) {
			found = p;
			break;	
		}
	}
	return found;
}
/*** GENERATED ***/

////////////// Foo

class UIParameterFooClass : public UIParameter
{
  public:
    UIParameterFooClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterFooClass::UIParameterFooClass()
{
    name = "foo";
    displayName = "Foo";
    scope = ScopePreset;
    type = TypeInt;
}
void UIParameterFooClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getFoo());
}
void UIParameterFooClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj->setFoo(value->getInt());
}
UIParameterFooClass UIParameterFooObj;
UIParameter* UIParameterFoo = &UIParameterFooObj;
////////////// Bar

class UIParameterBarClass : public UIParameter
{
  public:
    UIParameterBarClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterBarClass::UIParameterBarClass()
{
    name = "bar";
    displayName = "Bar";
    scope = ScopePreset;
    type = TypeInt;
}
void UIParameterBarClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getBar());
}
void UIParameterBarClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj->setBar(value->getInt());
}
UIParameterBarClass UIParameterBarObj;
UIParameter* UIParameterBar = &UIParameterBarObj;

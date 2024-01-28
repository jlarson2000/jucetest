/*
 * Copyright (c) 2010 Jeffrey S. Larson  <jeff@circularlabs.com>
 * All rights reserved.
 * See the LICENSE file for the full copyright and license declaration.
 * 
 * ---------------------------------------------------------------------
 * 
 * Model for a collection of user defined variables.
 * These are built dynamically in Mobius and Track when Variable
 * statements are evaluated in a script.
 *
 * They may also be serialized in a Project and Setup to store initial
 * values for variables.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../util/Util.h"
//#include "List.h"
#include "../util/XmlModel.h"
#include "../util/XmlBuffer.h"
//#include "XomParser.h"

#include "UserVariable.h"

/****************************************************************************
 *                                                                          *
 *                               XML CONSTANTS                              *
 *                                                                          *
 ****************************************************************************/

#define EL_VARIABLE "Variable"
#define EL_VARIABLES "Variables"
#define ATT_NAME "name"
#define ATT_VALUE "value"

/****************************************************************************
 *                                                                          *
 *   							   VARIABLE                                 *
 *                                                                          *
 ****************************************************************************/

UserVariable::UserVariable()
{
	init();
}

UserVariable::UserVariable(XmlElement* e)
{
	init();
	parseXml(e);
}

void UserVariable::init()
{
	mNext = NULL;
	mName = NULL;
	mValue.setNull();
}

UserVariable::~UserVariable()
{
	UserVariable *v, *next;
	for (v = mNext ; v != NULL ; v = next) {
		next = v->mNext;
		v->mNext = NULL;
		delete v;
	}

	delete mName;
}

void UserVariable::setName(const char* name)
{
	delete mName;
	mName = CopyString(name);
}

const char* UserVariable::getName()
{
	return mName;
}

void UserVariable::setValue(ExValue* value)
{
	mValue.set(value);
}

void UserVariable::getValue(ExValue* value)
{
	value->set(&mValue);
}

void UserVariable::setNext(UserVariable* v)
{
	mNext = v;
}

UserVariable* UserVariable::getNext()
{
	return mNext;
}

void UserVariable::toXml(class XmlBuffer* b)
{
	b->addOpenStartTag(EL_VARIABLE);

	b->addAttribute(ATT_NAME, mName);

	// note that we'll lose the type during serialization

	const char* value = mValue.getString();
	if (value != NULL)
	  b->addAttribute(ATT_VALUE, value);

	b->add("/>\n");
}

void UserVariable::parseXml(XmlElement* e)
{
	setName(e->getAttribute(ATT_NAME));

	// we don't save the type, so a round trip will always stringify
	mValue.setString(e->getAttribute(ATT_VALUE));
}

/****************************************************************************
 *                                                                          *
 *   							  VARIABLES                                 *
 *                                                                          *
 ****************************************************************************/

UserVariables::UserVariables()
{
	mVariables = NULL;
}

UserVariables::UserVariables(XmlElement* e)
{
	mVariables = NULL;
	parseXml(e);
}

UserVariables::~UserVariables()
{
	delete mVariables;
}

UserVariable* UserVariables::getVariable(const char* name)
{
	UserVariable* found = NULL;
	if (name != NULL) {
		for (UserVariable* v = mVariables ; v != NULL ; v = v->getNext()) {
			// case insensitive?
			const char* vname = v->getName();
			if (vname != NULL && !strcmp(name, vname)) {
				found = v;
				break;
			}
		}
	}
	return found;
}

void UserVariables::get(const char* name, ExValue* value)
{
    value->setNull();
	UserVariable* v = getVariable(name);
	if (v != NULL)
	  v->getValue(value);
}

void UserVariables::set(const char* name, ExValue* value)
{
	if (name != NULL) {
		UserVariable* v = getVariable(name);
		if (v != NULL)
		  v->setValue(value);
		else {
			v = new UserVariable();
			v->setName(name);
			v->setValue(value);
			// order these?
			v->setNext(mVariables);
			mVariables = v;
		}
	}
}

/**
 * For now we're going to go with the presence of a UserVariable to 
 * mean that it was bound.  We'll need to change this if we allow the
 * UserVariable list to persist after resets for some reason.
 */
bool UserVariables::isBound(const char* name)
{
	bool bound = false;

	if (name != NULL) {
		UserVariable* v = getVariable(name);
		bound = (v != NULL);
	}

	return bound;
}

/**
 * Clear the bound variables.
 * Assuming that we don't have to keep these but may waant to change that
 * if we need to set up semi-permanent references to them, for example
 * to show in the "visible parameters" component.
 */
void UserVariables::reset()
{
	delete mVariables;
    mVariables = NULL;
}

void UserVariables::parseXml(XmlElement* e)
{
	UserVariable* last = NULL;

	for (XmlElement* child = e->getChildElement() ; e != NULL ; 
		 e = e->getNextElement()) {
		UserVariable* v = new UserVariable(child);
		if (last == NULL)
		  mVariables = v;
		else
		  last->setNext(v);
		last = v;
	}
}

void UserVariables::toXml(XmlBuffer* b)
{
	if (mVariables != NULL) {
		b->addStartTag(EL_VARIABLES);
		b->incIndent();
		for (UserVariable* v = mVariables ; v != NULL ; v = v->getNext())
		  v->toXml(b);
		b->decIndent();
		b->addEndTag(EL_VARIABLES);
	}
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

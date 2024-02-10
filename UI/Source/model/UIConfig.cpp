/*
 * Copyright (c) 2010 Jeffrey S. Larson  <jeff@circularlabs.com>
 * All rights reserved.
 * See the LICENSE file for the full copyright and license declaration.
 * 
 * ---------------------------------------------------------------------
 * 
 * Model for Mobius UI configuration.
 *
 * Handling of Button configuration is weird and was evolving.
 * It started with a list of ButtonConfigs in UIConfig but moved
 * toward representing them as Bindings in MobiusConfig.
 *
 *   <Binding target='function' name='Reset' trigger='ui' value='0'/>
 *
 * This would have created a button for the function named Reset.
 * Though only function targets were used, the model supports adding
 * buttons for selecting configuration objects, controls, parameters, etc.
 *
 * What use is there for parameters?  I guess "set quantize on" when this
 * button is pressed.  Could be useful.
 *
 * Controls might be "set pan center".
 *
 * Since Bindings are compiled, it feels better to keep them in both UIConfig
 * and MobiusConfig and have them merged during startup into the compiled model.
 * Only the UI uses these so it makes sense for these to be adjusted during
 * UI initialization.
 *
 * Could use the Binding model here for buttons, but I like keeping them
 * distinct in case we want to store other button properties like color, or
 * alternate names.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <vector>
#include <memory>

#include "../util/Util.h"

#include "UIConfig.h"

/****************************************************************************
 *                                                                          *
 *   							  UI CONFIG                                 *
 *                                                                          *
 ****************************************************************************/

UIConfig::UIConfig()
{
    mError[0] = 0;
    mName = nullptr;
    mRefreshInterval = DEFAULT_REFRESH_INTERVAL;
    mAlertIntervals = DEFAULT_ALERT_INTERVALS;
    mMessageDuration = DEFAULT_MESSAGE_DURATION;
    width = 0;
    height = 0;

	mParameters = nullptr;
    mFloatingStrip = nullptr;
    mFloatingStrip2 = nullptr;
    mDockedStrip = nullptr;
}

UIConfig::~UIConfig()
{
    delete mName;
	delete mParameters;
    delete mFloatingStrip;
    delete mFloatingStrip2;
    delete mDockedStrip;
    // buttons and locations will auto-delete with std::unique_ptr
}

void UIConfig::setName(const char* s)
{
	delete mName;
	mName = CopyString(s);
}

const char* UIConfig::getName()
{
    return mName;
}

void UIConfig::setRefreshInterval(int i)
{
    // guard against insanely low intervals
    if (i < 10) i = 10;
    mRefreshInterval = i;
}

int UIConfig::getRefreshInterval()
{
    return mRefreshInterval;
}

void UIConfig::setAlertIntervals(int i)
{
    mAlertIntervals = i;
}

int UIConfig::getAlertIntervals()
{
    return mAlertIntervals;
}

void UIConfig::setMessageDuration(int i)
{
    // looks funny in the UI for this to be zero, bootstrap it if we
    // have an old config
    if (i == 0) i = DEFAULT_MESSAGE_DURATION;
    mMessageDuration = i;
}

int UIConfig::getMessageDuration()
{
    return mMessageDuration;
}

std::vector<std::unique_ptr<UILocation>>* UIConfig::getLocations()
{
    return &locations;
}

// ownership is not taken, think about this and do the
// same for the other lists
void UIConfig::addLocation(UILocation* loc)
{
    // the difference between push_back and emplace_back is extremely
    // subtle, god I hate c++
    locations.emplace_back(loc);
}

void UIConfig::clearLocations()
{
    locations.clear();
}

std::vector<std::unique_ptr<UIButton>>* UIConfig::getButtons()
{
    return &buttons;
}

void UIConfig::addButton(UIButton* butt)
{
    buttons.emplace_back(butt);
}

void UIConfig::clearButtons()
{
    buttons.clear();
}

StringList* UIConfig::getParameters()
{
    return mParameters;
}

void UIConfig::setParameters(StringList* l)
{
    delete mParameters;
    mParameters = l;
}

StringList* UIConfig::getFloatingStrip()
{
    return mFloatingStrip;
}

void UIConfig::setFloatingStrip(StringList* l)
{
    delete mFloatingStrip;
    mFloatingStrip = l;
}

StringList* UIConfig::getFloatingStrip2()
{
    return mFloatingStrip2;
}

void UIConfig::setFloatingStrip2(StringList* l)
{
    delete mFloatingStrip2;
    mFloatingStrip2 = l;
}

StringList* UIConfig::getDockedStrip()
{
    return mDockedStrip;
}

void UIConfig::setDockedStrip(StringList* l)
{
    delete mDockedStrip;
    mDockedStrip = l;
}

/**
 * Return the parse error message if it is set.
 */
const char* UIConfig::getError()
{
    return (mError[0] != 0) ? mError : nullptr;
}

/**
 * Cleanup after parsing.
 * For each display component, add a Location for any new ones, and
 * remove obsolete Locations.
 *
 * Needs work, port over UITypes for SpaceElements
 */
#if 0
void UIConfig::checkDisplayComponents()
{
    int i;

    // add missing components
    for (i = 0 ; SpaceElements[i] != nullptr ; i++) {
        DisplayElement* el = SpaceElements[i];
        Location* loc = getLocation(el->getName());
        if (loc == nullptr)
          loc = getLocation(el->alias);

        if (loc == nullptr) {
            loc = new Location(el->getName());
            // these start off disabled
            loc->setDisabled(true);
            addLocation(loc);
        }
        else if (StringEqual(loc->getName(), el->alias)) {
            loc->setName(el->getName());
        }
    }

    // remove obsolete components
    if (mLocations != nullptr) {
        for (i = 0 ; i < mLocations->size() ; i++) {
            Location* l = (Location*)mLocations->get(i);
            DisplayElement* el = DisplayElement::get(l->getName());
            if (el == nullptr)
              mLocations->remove(l);
        }
    }
}

Location* UIConfig::getLocation(const char* name)
{
	Location* found = nullptr;

	if (name != nullptr && mLocations != nullptr) {
		for (int i = 0 ; i < mLocations->size() ; i++) {
			Location* l = (Location*)mLocations->get(i);
			const char* lname = l->getName();
			if (lname != nullptr && !strcmp(lname, name)) {
				found = l;
				break;
			}
		}
	}
	return found;
}

void UIConfig::addLocation(Location* l)
{
	const char* name = l->getName();
	if (name != nullptr) {
		Location* existing = getLocation(name);
		if (existing == nullptr) {
			if (mLocations == nullptr)
			  mLocations = new ObjectList();
			mLocations->add(l);
		}
		else {
			existing->setX(l->getX());
			existing->setY(l->getY());
            existing->setDisabled(l->isDisabled());
			delete l;
		}
	}
	else {
		// malformed, ignore
		delete l;
	}
}

void UIConfig::updateLocation(const char* name, int x, int y)
{
	if (name != nullptr) {
		Location* loc = getLocation(name);
		if (loc == nullptr) {
            loc = new Location(name);
			if (mLocations == nullptr)
			  mLocations = new ObjectList();
			mLocations->add(loc);
		}
        loc->setX(x);
        loc->setY(y);
	}
}

#endif

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

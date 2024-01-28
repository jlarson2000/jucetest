/*
 * Copyright (c) 2024 Jeffrey S. Larson  <jeff@circularlabs.com>
 * All rights reserved.
 * See the LICENSE file for the full copyright and license declaration.
 * 
 * ---------------------------------------------------------------------
 *
 * Common superclass for various constant objects that are allocated
 * during static initialization.  
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "../util/Util.h"
#include "../util/Trace.h"
#include "../util/MessageCatalog.h"

#include "SystemConstant.h"

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

void SystemConstant::init()
{
    mName = NULL;
    mKey = 0;
    mDisplayName[0] = 0;
    mHelp = NULL;
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
    if (name != NULL)
      CopyString(name, mDisplayName, sizeof(mDisplayName));
}

/**
 * Look up the display name in a message catalog.
 * Since this is shared by several plugins don't bother
 * localizing if we've done it once.  This does mean that
 * in order to switch languages you will have to bounce 
 * the host.
 */
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
        if (msg != NULL) {
            setDisplayName(msg);
        }
        else {
            Trace(1, "No localization for constant %s\n", mName);
            setDisplayName(mName);
        }
    }
}

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

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

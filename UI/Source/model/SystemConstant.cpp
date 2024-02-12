/*
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

/**
 * This should only be used by constants that need to 
 * initialize themselves in a complex way.  Functions are like this
 * as are some MobiusModes.
 *
 * Add back if necessary, those were probably using the setters too.
 */
SystemConstant::SystemConstant()
{
    init();
}

/**
 * The normal constructor.
 */
SystemConstant::SystemConstant(const char* name, const char* displayName)
{
    init();
    mName = name;
    mDisplayName = displayName;
}

/**
 * Temporary constructor used by older objects that still use
 * message catalog keys.  The key is ignored.
 */
SystemConstant::SystemConstant(const char* name, int key) 
{
    init();
    mName = name;
}

void SystemConstant::init()
{
    mName = nullptr;
    mDisplayName = nullptr;
    mHelp = nullptr;
}

SystemConstant::~SystemConstant()
{
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
    mName = name;
}

const char* SystemConstant::getDisplayName() 
{
    return mDisplayName;
}

const char* SystemConstant::getDisplayableName() 
{
    return ((mDisplayName != nullptr) ? mDisplayName : mName);
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

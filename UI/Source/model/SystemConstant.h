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
 * System constants all have a name and an optional display name.  
 * Some will have a catalog key with deferred localization.
 *
 * There are assumptions about static initialization in here which is
 * fine for now but eventually may want these to be extensible.
 *
 * The stuff related to localization and message catalogs comes from
 * an old Mobius goal that I don't think is relevant any more and was
 * a pain in the ass.  Keep it around for awhile since it's already there
 * but I'd like to move away from it or at least find a less intrusive way
 * to do it.
 */

#pragma once

#define MAX_CONSTANT_DISPLAY_NAME 32

class SystemConstant {
  public:

	SystemConstant();
	SystemConstant(const char* name, int key);
	SystemConstant(const char* name, const char* displayName);

    virtual ~SystemConstant();

    const char* getName();
    void setName(const char* name);

    const char* getDisplayName();
    void setDisplayName(const char* name);

    int getKey();
    void setKey(int key);

    const char* getHelp();
    void setHelp(const char* name);

    void localize(class MessageCatalog* cat);

  private:

    void init();

    /**
     * This name is assumed to be a static string constant and will
     * not be copied or freed.
     */
    const char* mName;

    /**
     * This may come from a static or a message catalog so keep
     * a private copy.
     */
    char mDisplayName[MAX_CONSTANT_DISPLAY_NAME];

    /**
     * Non-zero if we initialize display name from a message catalog.
     */
    int mKey;

    /**
     * Used by functions, nothing else.
     * This is assumed to be static text.  If we're going to do
     * localization right, then this needs to be localized too.
     * I don't want to mess with that right now, but I don't want
     * to lose the old static help.
     */
    const char* mHelp;

};


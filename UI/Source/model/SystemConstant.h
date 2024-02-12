/*
 * Common superclass for various constant objects that are allocated
 * during static initialization.
 * 
 * System constants all have a name and an optional display name.  
 * A member is defined for help text but is not used.
 *
 * The message catalog key is a temporary holder from old Mobius
 * until I can upgrade all of the object definition to just
 * store display names directly in the definitions.
 */

#pragma once

#define MAX_CONSTANT_DISPLAY_NAME 32

class SystemConstant {
  public:

	SystemConstant();
	SystemConstant(const char* name, int key);
	SystemConstant(const char* name, const char* displayName);

    virtual ~SystemConstant();

    // why would we ever need to set these?
    // comments say used in rare cases like MobiusMode
    // that need to construct in a complex way with
    // the noarg constructor

    const char* getName();
    void setName(const char* name);

    const char* getDisplayName();
    void setDisplayName(const char* name);
    const char* getDisplayableName();
    
    const char* getHelp();
    void setHelp(const char* name);

  private:

    void init();

    /**
     * This name is assumed to be a static string constant and will
     * not be copied or freed.
     */
    const char* mName;

    /**
     * Formerly a buffer so it could hold a catalog entry.
     * Can be a constant now.
     */
    const char* mDisplayName;

    /**
     * Used by functions, nothing else.
     * This is assumed to be static text.  If we're going to do
     * localization right, then this needs to be localized too.
     * I don't want to mess with that right now, but I don't want
     * to lose the old static help.
     */
    const char* mHelp;

};


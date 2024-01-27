// moved from Binding.h into it's own file to reduce dependencies

#pragma once

/****************************************************************************
 *                                                                          *
 *                              SYSTEM CONSTANT                             *
 *                                                                          *
 ****************************************************************************/

#define MAX_CONSTANT_DISPLAY_NAME 32

/**
 * System constants all have a name and an optional display name.  
 * Some will have a catalog key with deferred localization.
 */
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

/****************************************************************************
 *                                                                          *
 *   							   TARGETS                                  *
 *                                                                          *
 ****************************************************************************/

/**
 * A Target represents the various things within Mobius that can 
 * be bound to a Trigger or used in an Export.
 *
 * UIConfig is not currrently used.
 *
 * You can also set setups, presets, and bindings through Parameters
 * named "setup", "preset", and "bindings".  The only reason these are
 * exposed as individual targets is to make it easier to bind 
 * MIDI events directly to them, the alternative would be to bind go 
 * Parameter:setup and use an argument with the setup name.
 */
class Target : public SystemConstant {
  public:

	static Target* get(const char* name);

	Target(const char* name, const char* display);

  private:

};

extern Target* TargetFunction;
extern Target* TargetParameter;
extern Target* TargetSetup;
extern Target* TargetPreset;
extern Target* TargetBindings;
extern Target* TargetUIControl;
extern Target* TargetUIConfig;

// internal targets, can't be used in bindings
extern Target* TargetScript;

extern Target* Targets[];


/**
 * Common base class for configuration objects that can be selected
 * with Triggers.
 *
 * Currently this is Setup, Preset, and BindingConfig.
 *
 * Like UIControl, this isn't part of the Binding model so it doesn't
 * really belong here, but I don't have a better place for it.
 */
class Bindable {

  public:

	Bindable();
	~Bindable();
	void clone(Bindable* src);

	void setNumber(int i);
	int getNumber();

	void setName(const char* name);
	const char* getName();

    virtual Bindable* getNextBindable() = 0;
	virtual class Target* getTarget() = 0;

	void toXmlCommon(class XmlBuffer* b);
	void parseXmlCommon(class XmlElement* e);

  protected:

	/**
	 * A non-persistent internal number.
	 * Used to uniquely identity presets that may not have names
	 * or have ambiguous names.
	 */
	int mNumber;

	/**
	 * Let configurations be named.
	 */
	char* mName;

};


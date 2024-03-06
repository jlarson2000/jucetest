//
// Temporary backward compatibility for things that want the old binding model
//

#include "../../model/SystemConstant.h"

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

/**
 * Defines a control managed by the UI that may be a binding target.
 * These are given to Mobius during initialization, the core code
 * does not have any predefined knowledge of what these are.
 *
 * These are functionally the same as a Function or Parameter objects,
 * so they don't really belong with the Binding definition classes
 * but I don't have a better place for it.  UITypes.h shouldn't be used
 * because that has things in it specific to the UI which core Mobius
 * shouldn't know about.
 * 
 * There are two types of controls: instant and continuous.
 * Instant controls are like Mobius functions, they are one-shot
 * actions that do not have a range of values.
 *
 * Continuous controls are like Mobius controls, they have a range
 * of values.
 *
 * NOTE: Continuous controls have never been used, the current
 * controls are: nextParameter, prevParameter, incParameter, decParameter,
 * spaceDrag (aka Move Display Components).
 *
 * We don't have a way to define min/max ranges even if we did have
 * continuous controls and we don't have a way to define sustain behavior.
 * Basically we'd need things from Function and Parameter combined, 
 * this isn't such a bad thing but it may be better to have the UI
 * give us Function and Parameter objects instead so we have
 * a consistent way of dealing with both internal and UI functions.
 *
 */
class UIControl : public SystemConstant {

  public:

	UIControl();
	UIControl(const char* name, int key);

  private:

    void init();

};

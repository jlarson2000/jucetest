
#pragma once

class Parameter {

  public:

    typedef enum {
        TypeInt,
        TypeBoolean,
        TypeEnum,
        TypeString
    } Type;

    typedef enum {
        ScopeNone,
        ScopePreset,
        ScopeTrack,
        ScopeSetup,
        ScopeGlobal
    } Scope;

    Parameter();
    Parameter(const char* argName, const char* argDisplayName);
	virtual ~Parameter();

    const char* getName() {
        return name;
    }

    const char* getDisplayName() {
        return (displayName != nullptr) ? displayName : name;
    }
    
    Type getType() { return type; }
    Scope getScope() { return scope; }
    int getLow() { return low; }
    int getHigh() { return high; }
    int getValue() { return value; }
    
    static std::vector<Parameter*> Parameters;

//  protected:

    const char* name;
    const char* displayName;
    Type type;
    Scope scope;
    
    int low;
    int high;
    int value;
    
    // mobius has this, why?
	// const char* aliases[MAX_PARAMETER_ALIAS];

    bool bindable;      // true if this bindable 
	bool dynamic;		// true if labels and max ordinal can change
    bool deprecated;    // true if this is a backward compatible parameter
    bool transient;     // memory only, not stored in config objects
    bool resettable;    // true for Setup parameters that may be reset
    bool scheduled;     // true if setting the value schedules an event
    bool takesAction;   // true if ownership of the Action may be taken
    bool control;       // true if this is displayed as control in the binding UI

    /**
     * When this is set, it is a hint to the UI to display the value
     * of this parameter as a positive and negative range with zero
     * at the center.  This has no effect on the value of the parameter
     * only the way it is displayed.
     */
    bool zeroCenter;

    /**
     * Control parameters  have a default value, usually either the 
     * upper end of the range or the center.
     */
    int defaultValue;

};

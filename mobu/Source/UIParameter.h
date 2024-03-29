/*
 * Mobius parameter definitions.
 *
 * This file is NOT generated
 * Generated subclasses and code are found in UIParameterClasses.cpp/h
 *
 * The authoritative source for this file is in the main Mobius source tree.
 * I copied it over to mobu just to make compilation and testing easier.
 * This may become stale, but it matters less because we don't try to
 * actually use UIParameter objects over here, we just need to verify that
 * the classes compile.  But still, with a little more work we could configure
 * Projucer to get UIParameter.h and UIParameter.obj from the main tree.
 */

#pragma once

// for juce::var only
#include <JuceHeader.h>

// do not necessarily need this with generated files
#include "SystemConstant.h"

//////////////////////////////////////////////////////////////////////
// Enumerations
//////////////////////////////////////////////////////////////////////

/**
 * The type of the parameter.
 * The canonical type names found in the generator source files are:
 *
 *   int, bool, enum, string
 *
 * Type names must follow those names with capitalization.
 */
typedef enum {

	TypeInt,
	TypeBool,
	TypeEnum,
	TypeString,
    TypeStructure

} UIParameterType;

/**
 * Parameter scope.
 * Canonical names: global, preset, setup, track
 *
 * The corresponding model classes are:
 *
 * MobiusConfig, Preset, Setup, SetupTrack
 *
 * MobiusConfig is the only one whose name differs from the canonical name.
 * Should we have a model for mapping these?
 */
typedef enum {

    ScopeNone,
    ScopeGlobal,
    ScopePreset,
    ScopeSetup,
    ScopeTrack

} UIParameterScope;

//////////////////////////////////////////////////////////////////////
// Class
//////////////////////////////////////////////////////////////////////

/**
 * Continue using const char* for strings so we don't have to
 * rip up the model editors yet, but move to juce::String eventually
 * Especially for the arrays.
 *
 * SystemConstant defines name and displayName
 */
class UIParameter : public SystemConstant
{
  public:

	UIParameter();
    // don't think we'll be needing subclasses yet
	virtual ~UIParameter();

    UIParameterScope scope = ScopeGlobal;
	UIParameterType type = TypeInt;
    
    /**
     * True if it supports multiple values.
     */
    bool multi = false;

    /**
     * For TypeEnum, the  set of allowed values.
     */
    const char** values = nullptr;

    /**
     * For TypeEnum, the set of alternate display names.
     */
	const char** valueLabels = nullptr;

    /**
     * For TypeInt, the lowest allowed value.
     */
    int low = 0;

    /**
     * For TypeInt, the highest allowed value.
     * If the dynamic flag is set, the high value must be calculated
     * at runtime.
     */
    int high = 0;

    /**
     * For TypeInt, a few parameters may have a default value other
     * than zer.  Typically this will be the upper end of a range
     * or the center.
     */
    int defaultValue = 0;

    /**
     * Indiciates the high value must be calculated at runtime.
     */
    bool dynamic = false;
    
    /**
     * Indicates that the value should be displayed as a positive
     * or negative integer with zero at the center of the low/high range.
     */
    bool zeroCenter = false;

    /**
     * Indicates that this can be highlighted in the UI as a sweepable control.
     * These are the most common parameters used in bindings and can be
     * separated from other parameters to make them easier to find.
     * Explore other presentation categories like "advanced".
     */
    bool control = false;

    /**
     * Indicates that this parameter exists only at runtime and will not
     * be saved in a configuration file.  It can still be used in bindings
     * but will be omitted from configuration file generators.
     */
    bool transient = false;

    /**
     * Indiciates that this parameter may use juce::var for value access.
     * Since we're redesigning this model just for the UI, this can
     * eventually be the default and we can remove ExValue
     */
    bool juceValues = false;

    /**
     * Indicates that this parameter cannot be bound to MIDI or host parameters
     * so keep it out of the operation selection UI.
     */
    bool noBinding;

    /**
     * Indiciates that the value of current value of this parameter is to be retained
     * after a track is Reset.
     */
    bool resetRetain;

    /**
     * Old option I'm not sure we need to carry forward.
     */
    bool scheduled;

    /**
     * Indiciates that this parameter is visible for bindings, but not found
     * in configuration files so no get/set code will be generated.
     * They can only be used at runtime, and are set with UIActions
     * and read with MobiusState or Query.
     * Used for various control parameters related to speed and pitch.
     * Needs more thought.
     */
    bool noConfig;
    
    /**
     * In a few cases the names were changed to be more consistent
     * or obvious or because I liked them better.  In order to correlate
     * the new parameter definitions with the old ones, this would
     * be the name of the old Parameter.
     */
    const char* coreName = nullptr;
    
    //
    // Others from the old model to consider
    //

    // bindable: true if this is may be used in a Binding
    // resettable: used at runtime to indiciate that the parameter
    //  value is changed after the Reset function, don't think this
    //  is interesting for the UI

    //////////////////////////////////////////////////////////////////////
    //
    // Value Access Functions
    //
    // At least one set of ExValue or juce::var functions must be implemented
    // in the subclass. Starting with ExValue being required and juce::var
    // being optional, but need to switch the UI to use juce::var
    //
    // The values are always represented as ordinal integers except for
    // type=string and type=structure.
    //
    // type=structure string values are efffectively an enumeration whose
    // set of allowed values is known and must be queried.
    // 
    //////////////////////////////////////////////////////////////////////

    virtual void getValue(void* object, class ExValue* value) = 0;
    virtual void setValue(void* object, class ExValue* value) = 0;

    // shouldn't this just return a juce::var by value?
    // implementations are stubbed and only used if juceValues is set
    virtual void getValue(void* object, juce::var& value) {
    }
    
    virtual void setValue(void* object, juce::var& value) {
    }

    //////////////////////////////////////////////////////////////////////
    //
    // Enumeration Utilities
    //
    //////////////////////////////////////////////////////////////////////
    
    /**
     * For type=enum, convert a symbolic value into an ordinal value by
     * locating the given value within the allowedValues set.
     *
     * This currently only supports internal names, not display names.
     * Do we have a reason to support display names?
     *
     * Original model getEnun would trace a warning if the value was
     * not in the set of allowed values and returned zero.  This now
     * does not trace and returns -1.
     */
    int getEnumOrdinal(const char* value);

    /**
     * For type=enum, convert an ordinal value into the symbolic value
     * defined by the allowedValues set.
     */
    const char* getEnumName(int ordinal);

    //////////////////////////////////////////////////////////////////////
    //
    // Model Queries
    //
    // For parameters with type='structure' or options='dynamic' some
    // characteristics of the parameter cannot be statically defined and
    // must be calculated at runtime.  These functions provide a way to do that
    // until the Query model is fleshed out.
    //
    // Currently it is assumed that the calculation can be satisfied using
    // only the model contained within MobiusConfig.  Eventually some of
    // these will shift to UIConfig or another object at which point Query
    // must be used in a larger context.
    //
    //////////////////////////////////////////////////////////////////////
    
    /**
     * Calculate the maximum ordinal value of a dynamic or structure parameter.
     */
    int getDynamicHigh(class MobiusConfig* container);

    /**
     * For type=structure, calculate the set of structure names.
     * This is in effect the symbolic values of a dynamic enumeration.
     * If this becomes necessary for types other than type=structure
     * change the name.
     * 
     * The result is returned as a StringList and must be deleted.
     */
    class StringList* getStructureNames(class MobiusConfig* container);

    /**
     * For type=structure convert a symbolic structure name into an ordinal
     * within the value as that would be returned by getDynamicValues
     */
    int getStructureOrdinal(class MobiusConfig* container, const char* name);

    /**
     * For type=structure convert an ordinal into a symbolic name.
     * Note that the string returned will be found within the container
     * model and will become invalid if the container is deleted.
     * It should be considered temporary and copied if it needs to live
     * for an indefinite time.
     */
    const char* getStructureName(class MobiusConfig* container, int ordinal);

    //////////////////////////////////////////////////////////////////////
    //
    // Global Parameter Registry
    //
    //////////////////////////////////////////////////////////////////////

    static std::vector<UIParameter*> Instances;
    static void trace();
	static UIParameter* find(const char* name);
	static UIParameter* findDisplay(const char* name);

};
/*** GENERATED ***/

extern UIParameter* UIParameterLogStatus;
extern UIParameter* UIParameterActiveSetup;
extern UIParameter* UIParameterActiveOverlay;
extern UIParameter* UIParameterFadeFrames;
extern UIParameter* UIParameterMaxSyncDrift;
extern UIParameter* UIParameterDriftCheckPoint;
extern UIParameter* UIParameterPluginPorts;
extern UIParameter* UIParameterLongPress;
extern UIParameter* UIParameterSpreadRange;
extern UIParameter* UIParameterTraceLevel;
extern UIParameter* UIParameterAutoFeedbackReduction;
extern UIParameter* UIParameterIsolateOverdubs;
extern UIParameter* UIParameterMonitorAudio;
extern UIParameter* UIParameterSaveLayers;
extern UIParameter* UIParameterQuickSave;
extern UIParameter* UIParameterIntegerWaveFile;
extern UIParameter* UIParameterGroupFocusLock;
extern UIParameter* UIParameterTrackCount;
extern UIParameter* UIParameterGroupCount;
extern UIParameter* UIParameterMaxLoops;
extern UIParameter* UIParameterInputLatency;
extern UIParameter* UIParameterOutputLatency;
extern UIParameter* UIParameterMidiInput;
extern UIParameter* UIParameterMidiOutput;
extern UIParameter* UIParameterMidiThrough;
extern UIParameter* UIParameterPluginMidiInput;
extern UIParameter* UIParameterPluginMidiOutput;
extern UIParameter* UIParameterPluginMidiThrough;
extern UIParameter* UIParameterAudioInput;
extern UIParameter* UIParameterAudioOutput;
extern UIParameter* UIParameterNoiseFloor;
extern UIParameter* UIParameterMidiRecordMode;
extern UIParameter* UIParameterSubcycles;
extern UIParameter* UIParameterMultiplyMode;
extern UIParameter* UIParameterShuffleMode;
extern UIParameter* UIParameterAltFeedbackEnable;
extern UIParameter* UIParameterEmptyLoopAction;
extern UIParameter* UIParameterEmptyTrackAction;
extern UIParameter* UIParameterTrackLeaveAction;
extern UIParameter* UIParameterLoopCount;
extern UIParameter* UIParameterMuteMode;
extern UIParameter* UIParameterMuteCancel;
extern UIParameter* UIParameterOverdubQuantized;
extern UIParameter* UIParameterQuantize;
extern UIParameter* UIParameterBounceQuantize;
extern UIParameter* UIParameterRecordResetsFeedback;
extern UIParameter* UIParameterSpeedRecord;
extern UIParameter* UIParameterRoundingOverdub;
extern UIParameter* UIParameterSwitchLocation;
extern UIParameter* UIParameterReturnLocation;
extern UIParameter* UIParameterSwitchDuration;
extern UIParameter* UIParameterSwitchQuantize;
extern UIParameter* UIParameterTimeCopyMode;
extern UIParameter* UIParameterSoundCopyMode;
extern UIParameter* UIParameterRecordThreshold;
extern UIParameter* UIParameterSwitchVelocity;
extern UIParameter* UIParameterMaxUndo;
extern UIParameter* UIParameterMaxRedo;
extern UIParameter* UIParameterNoFeedbackUndo;
extern UIParameter* UIParameterNoLayerFlattening;
extern UIParameter* UIParameterSpeedShiftRestart;
extern UIParameter* UIParameterPitchShiftRestart;
extern UIParameter* UIParameterSpeedStepRange;
extern UIParameter* UIParameterSpeedBendRange;
extern UIParameter* UIParameterPitchStepRange;
extern UIParameter* UIParameterPitchBendRange;
extern UIParameter* UIParameterTimeStretchRange;
extern UIParameter* UIParameterSlipMode;
extern UIParameter* UIParameterSlipTime;
extern UIParameter* UIParameterAutoRecordTempo;
extern UIParameter* UIParameterAutoRecordBars;
extern UIParameter* UIParameterRecordTransfer;
extern UIParameter* UIParameterOverdubTransfer;
extern UIParameter* UIParameterReverseTransfer;
extern UIParameter* UIParameterSpeedTransfer;
extern UIParameter* UIParameterPitchTransfer;
extern UIParameter* UIParameterWindowSlideUnit;
extern UIParameter* UIParameterWindowEdgeUnit;
extern UIParameter* UIParameterWindowSlideAmount;
extern UIParameter* UIParameterWindowEdgeAmount;
extern UIParameter* UIParameterDefaultSyncSource;
extern UIParameter* UIParameterDefaultTrackSyncUnit;
extern UIParameter* UIParameterSlaveSyncUnit;
extern UIParameter* UIParameterManualStart;
extern UIParameter* UIParameterMinTempo;
extern UIParameter* UIParameterMaxTempo;
extern UIParameter* UIParameterBeatsPerBar;
extern UIParameter* UIParameterMuteSyncMode;
extern UIParameter* UIParameterResizeSyncAdjust;
extern UIParameter* UIParameterSpeedSyncAdjust;
extern UIParameter* UIParameterRealignTime;
extern UIParameter* UIParameterOutRealign;
extern UIParameter* UIParameterActiveTrack;
extern UIParameter* UIParameterTrackName;
extern UIParameter* UIParameterPreset;
extern UIParameter* UIParameterFocus;
extern UIParameter* UIParameterGroup;
extern UIParameter* UIParameterMono;
extern UIParameter* UIParameterFeedback;
extern UIParameter* UIParameterAltFeedback;
extern UIParameter* UIParameterInput;
extern UIParameter* UIParameterOutput;
extern UIParameter* UIParameterPan;
extern UIParameter* UIParameterSyncSource;
extern UIParameter* UIParameterTrackSyncUnit;
extern UIParameter* UIParameterAudioInputPort;
extern UIParameter* UIParameterAudioOutputPort;
extern UIParameter* UIParameterPluginInputPort;
extern UIParameter* UIParameterPluginOutputPort;
extern UIParameter* UIParameterSpeedOctave;
extern UIParameter* UIParameterSpeedStep;
extern UIParameter* UIParameterSpeedBend;
extern UIParameter* UIParameterPitchOctave;
extern UIParameter* UIParameterPitchStep;
extern UIParameter* UIParameterPitchBend;
extern UIParameter* UIParameterTimeStretch;

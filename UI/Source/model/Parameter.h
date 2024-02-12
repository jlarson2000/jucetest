/*
 * Static object definitions for Mobius parameters.
 *
 * Parameters are named values that may have a range
 * of values and are used to control operational aspects of the engine.
 * Most parameters are displayed in the UI for editing and stored
 * in the MobiusConfig object.
 *
 * The Parameter class defines the characteristics of the parameter
 * including an internal name, an optional display name, the value type,
 * value range, and scope (global, preset, setup, track).
 *
 * DESIGN NOTES
 *
 * I've tried to keep Juce out of this so that the model and engine
 * can be independent of UI technology, but juce::var crept in because
 * it is more convenient than ExValue for a few things.  
 *
 */

#pragma once

#include <vector>

// for juce::var only
#include <JuceHeader.h>

#include "SystemConstant.h"

/****************************************************************************
 *                                                                          *
 *                                 CONSTANTS                                *
 *                                                                          *
 ****************************************************************************/

#define MAX_PARAMETER_ALIAS 4

typedef enum {

	TYPE_INT,
	TYPE_BOOLEAN,
	TYPE_ENUM,
	TYPE_STRING

} ParameterType;

typedef enum {

    // it is really important that these initialze properly
    // don't defualt and assume it's Preset
    PARAM_SCOPE_NONE,
	PARAM_SCOPE_PRESET,
	PARAM_SCOPE_TRACK,
	PARAM_SCOPE_SETUP,
	PARAM_SCOPE_GLOBAL

} ParameterScope;

/****************************************************************************
 *                                                                          *
 *                                 PARAMETER                                *
 *                                                                          *
 ****************************************************************************/

class Parameter : public SystemConstant {

  public:

	Parameter(const char* name, const char* displayName);
	Parameter(const char* name, int key);
	virtual ~Parameter();

    /**
     * Defines the value type, int, string, bool, enum, etc.
     */
	ParameterType type;

    /**
     * True if it supports multiple values.
     */
    bool multi;

    /**
     * The parameter scope, global, preset, track, etc.
     */
	ParameterScope scope;

    /**
     * For enumeration parameters, a set of allowed values.
     */
    const char** values;

    /**
     * For enumeration parameters, a set of alternate display
     * names for the allowed values.
     */
	const char** valueLabels;

    /**
     * For integer parameters, the lowest allowed value.
     */
    int low;

    /**
     * For integer parameters, the highest allowed value.
     * For a small number of parameters, this value may be changed
     * at runtime to adapt to other configuration changes.
     */
    int high;

    /**
     * A few parameters have a default value, usually either
     * the upper end of a range or the center.
     * Used when initialing new configuration objects containing
     * parameters.
     */
    int defaultValue;
    
    //
    // Flags that can be used as hints for the UI for editing
    // These do not effect the use at runtime
    //

    bool bindable;      // true if this bindable 
    bool control;       // true if this is displayed as control in the binding UI
    bool juceValues = false; // true if this may be accessed using juce::var

    /**
     * When this is set, it is a hint to the UI to display the value
     * of this parameter as a positive and negative range with zero
     * at the center.  This has no effect on the value of the parameter
     * only the way it is displayed.
     */
    bool zeroCenter;

    //
    // Flags that are in the process of being phased out
    // Do not write new code that depends on these
    //
    
    // behavioral flags
	bool dynamic;		// true if labels and max ordinal can change
    bool deprecated;    // true if this is a backward compatible parameter
    bool transient;     // memory only, not stored in config objects
    bool resettable;    // true for Setup parameters that may be reset
    bool scheduled;     // true if setting the value schedules an event
    bool takesAction;   // true if ownership of the Action may be taken

    /**
     * Methods implemented by subclasses to get and set the value
     * of a parameter from the configuration objects.
     */

    virtual void getConfigValue(void* object, class ExValue* value) = 0;
    virtual void setConfigValue(void* object, class ExValue* value) = 0;

    /**
     * New interface just for the Juce UI with complicated values.
     */
    virtual void getJuceValue(void* object, juce::var& value);
    virtual void setJuceValue(void* object, juce::var& value);
    
    // 
    // Coercion helpers
    //
    
	/**
	 * Convert a string value to an enumeration ordinal value.
     * If the value is not in the enum, an error is traced and zero is returned.
	 */
	int getEnum(const char *value);

	/**
	 * Convert a string value to an enumeration ordinal value, returning
     * -1 if the value isn't in the enum.
	 */
	int getEnumNoWarn(const char *value);

	/**
	 * Convert an ExValue with an string or a number into an ordinal.
	 */
	int getEnum(ExValue *value);

    /**
     * Convert an enumeration ordinal into the corresponding
     * internal name.
     */
    const char* getEnumName(int value);

    //
    // Global parameter registry
    //
    
    static std::vector<Parameter*> Parameters;
    static void dumpParameters();
	static Parameter* getParameter(const char* name);
	static Parameter* getParameterWithDisplayName(const char* name);
    
    //
    // Temporary backward compatibility for old definitions
    //

    void addAlias(const char* alias) {};


  private:

    static const char* getEnumLabel(ParameterType type);
    static const char* getEnumLabel(ParameterScope scope);

    void init();

};

/****************************************************************************
 *                                                                          *
 *                            PARAMETER CONSTANTS                           *
 *                                                                          *
 ****************************************************************************/

// Preset Parameters

extern Parameter* AltFeedbackEnableParameter;
extern Parameter* AutoRecordBarsParameter;
extern Parameter* AutoRecordTempoParameter;
extern Parameter* BounceQuantizeParameter;
extern Parameter* EmptyLoopActionParameter;
extern Parameter* EmptyTrackActionParameter;
extern Parameter* LoopCountParameter;
extern Parameter* MaxRedoParameter;
extern Parameter* MaxUndoParameter;
extern Parameter* MultiplyModeParameter;
extern Parameter* MuteCancelParameter;
extern Parameter* MuteModeParameter;
extern Parameter* NoFeedbackUndoParameter;
extern Parameter* NoLayerFlatteningParameter;
extern Parameter* OverdubQuantizedParameter;
extern Parameter* OverdubTransferParameter;
extern Parameter* PitchBendRangeParameter;
extern Parameter* PitchSequenceParameter;
extern Parameter* PitchShiftRestartParameter;
extern Parameter* PitchStepRangeParameter;
extern Parameter* PitchTransferParameter;
extern Parameter* QuantizeParameter;
extern Parameter* SpeedBendRangeParameter;
extern Parameter* SpeedRecordParameter;
extern Parameter* SpeedSequenceParameter;
extern Parameter* SpeedShiftRestartParameter;
extern Parameter* SpeedStepRangeParameter;
extern Parameter* SpeedTransferParameter;
extern Parameter* TimeStretchRangeParameter;
extern Parameter* RecordResetsFeedbackParameter;
extern Parameter* RecordThresholdParameter;
extern Parameter* RecordTransferParameter;
extern Parameter* ReturnLocationParameter;
extern Parameter* ReverseTransferParameter;
extern Parameter* RoundingOverdubParameter;
extern Parameter* ShuffleModeParameter;
extern Parameter* SlipModeParameter;
extern Parameter* SlipTimeParameter;
extern Parameter* SoundCopyParameter;
extern Parameter* SubCycleParameter;
extern Parameter* SustainFunctionsParameter;
extern Parameter* SwitchDurationParameter;
extern Parameter* SwitchLocationParameter;
extern Parameter* SwitchQuantizeParameter;
extern Parameter* SwitchVelocityParameter;
extern Parameter* TimeCopyParameter;
extern Parameter* TrackLeaveActionParameter;
extern Parameter* WindowEdgeAmountParameter;
extern Parameter* WindowEdgeUnitParameter;
extern Parameter* WindowSlideAmountParameter;
extern Parameter* WindowSlideUnitParameter;

// Deprecated Parameters

extern Parameter* AutoRecordParameter;
extern Parameter* InsertModeParameter;
extern Parameter* InterfaceModeParameter;
extern Parameter* LoopCopyParameter;
extern Parameter* OverdubModeParameter;
extern Parameter* RecordModeParameter;
extern Parameter* SamplerStyleParameter;
extern Parameter* TrackCopyParameter;

// Track Parameters

extern Parameter* AltFeedbackLevelParameter;
extern Parameter* AudioInputPortParameter;
extern Parameter* AudioOutputPortParameter;
extern Parameter* FeedbackLevelParameter;
extern Parameter* FocusParameter;
extern Parameter* GroupParameter;
extern Parameter* InputLevelParameter;
extern Parameter* InputPortParameter;
extern Parameter* MonoParameter;
extern Parameter* OutputLevelParameter;
extern Parameter* OutputPortParameter;
extern Parameter* PanParameter;
extern Parameter* PluginInputPortParameter;
extern Parameter* PluginOutputPortParameter;
extern Parameter* SpeedBendParameter;
extern Parameter* SpeedOctaveParameter;
extern Parameter* SpeedStepParameter;
extern Parameter* TrackNameParameter;
extern Parameter* PitchBendParameter;
extern Parameter* PitchOctaveParameter;
extern Parameter* PitchStepParameter;
extern Parameter* TimeStretchParameter;
extern Parameter* TrackPresetParameter;
extern Parameter* TrackPresetNumberParameter;
extern Parameter* TrackSyncUnitParameter;
extern Parameter* SyncSourceParameter;

// Setup Parameters

extern Parameter* BeatsPerBarParameter;
extern Parameter* DefaultSyncSourceParameter;
extern Parameter* DefaultTrackSyncUnitParameter;
extern Parameter* ManualStartParameter;
extern Parameter* MaxTempoParameter;
extern Parameter* MinTempoParameter;
extern Parameter* MuteSyncModeParameter;
extern Parameter* OutRealignModeParameter;
extern Parameter* RealignTimeParameter;
extern Parameter* ResizeSyncAdjustParameter;
extern Parameter* SlaveSyncUnitParameter;
extern Parameter* SpeedSyncAdjustParameter;
extern Parameter* InitialTrackParameter;
extern Parameter* ResetablesParameter;

// Global Parameters

extern Parameter* AltFeedbackDisableParameter;
extern Parameter* AudioInputParameter;
extern Parameter* AudioOutputParameter;
extern Parameter* AutoFeedbackReductionParameter;
extern Parameter* BindingsParameter;
extern Parameter* ConfirmationFunctionsParameter;
extern Parameter* CustomMessageFileParameter;
extern Parameter* CustomModeParameter;
extern Parameter* DriftCheckPointParameter;
extern Parameter* DualPluginWindowParameter;
extern Parameter* FadeFramesParameter;
extern Parameter* FocusLockFunctionsParameter;
extern Parameter* GroupFocusLockParameter;
extern Parameter* HostMidiExportParameter;
extern Parameter* InputLatencyParameter;
extern Parameter* IntegerWaveFileParameter;
extern Parameter* IsolateOverdubsParameter;
extern Parameter* LogStatusParameter;
extern Parameter* LongPressParameter;
extern Parameter* MaxLoopsParameter;
extern Parameter* MaxSyncDriftParameter;
extern Parameter* MidiExportParameter;
extern Parameter* MidiInputParameter;
extern Parameter* MidiOutputParameter;
extern Parameter* MidiRecordModeParameter;
extern Parameter* MidiThroughParameter;
extern Parameter* MonitorAudioParameter;
extern Parameter* MuteCancelFunctionsParameter;
extern Parameter* NoiseFloorParameter;
extern Parameter* OutputLatencyParameter;
extern Parameter* OscInputPortParameter;
extern Parameter* OscOutputPortParameter;
extern Parameter* OscOutputHostParameter;
extern Parameter* OscTraceParameter;
extern Parameter* OscEnableParameter;
extern Parameter* PluginMidiInputParameter;
extern Parameter* PluginMidiOutputParameter;
extern Parameter* PluginMidiThroughParameter;
extern Parameter* PluginPortsParameter;
extern Parameter* QuickSaveParameter;
extern Parameter* SampleRateParameter;
extern Parameter* SaveLayersParameter;
extern Parameter* SetupNameParameter;
extern Parameter* SetupNumberParameter;
extern Parameter* SpreadRangeParameter;
extern Parameter* TraceDebugLevelParameter;
extern Parameter* TracePrintLevelParameter;
extern Parameter* TrackGroupsParameter;
extern Parameter* TrackParameter;
extern Parameter* TracksParameter;
extern Parameter* UnitTestsParameter;
extern Parameter* TrackInputPortParameter;
extern Parameter* TrackOutputPortParameter;

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

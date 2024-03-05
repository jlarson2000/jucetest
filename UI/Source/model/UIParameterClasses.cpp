/**
 * File containing the generated UIParameter subclasses.
 * Unlike UIParameter.h which we'll extend to contain the static object pointers
 * we collect all the class definitions in a separate file to keep
 * UIParameter.cpp clean.
 */

#include <JuceHeader.h>

#include "../util/Util.h"
#include "../model/ExValue.h"
#include "../model/MobiusConfig.h"
#include "../model/Preset.h"
#include "../model/Setup.h"

#include "UIParameter.h"

/*** GENERATED ***/


//******************** global


////////////// LogStatus

class UIParameterLogStatusClass : public UIParameter
{
  public:
    UIParameterLogStatusClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterLogStatusClass::UIParameterLogStatusClass()
{
    name = "logStatus";
    displayName = "Log Status";
    scope = ScopeGlobal;
    type = TypeBool;
}
void UIParameterLogStatusClass::getValue(void* obj, ExValue* value)
{
    value->setBool(((MobiusConfig*)obj)->isLogStatus());
}
void UIParameterLogStatusClass::setValue(void* obj, ExValue* value)
{
    ((MobiusConfig*)obj)->setLogStatus(value->getBool());
}
UIParameterLogStatusClass UIParameterLogStatusObj;
UIParameter* UIParameterLogStatus = &UIParameterLogStatusObj;

////////////// ActiveSetup

class UIParameterActiveSetupClass : public UIParameter
{
  public:
    UIParameterActiveSetupClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterActiveSetupClass::UIParameterActiveSetupClass()
{
    name = "activeSetup";
    displayName = "Active Setup";
    coreName = "setup";
    scope = ScopeGlobal;
    type = TypeString;
}
void UIParameterActiveSetupClass::getValue(void* obj, ExValue* value)
{
    value->setString(((MobiusConfig*)obj)->getActiveSetup());
}
void UIParameterActiveSetupClass::setValue(void* obj, ExValue* value)
{
    ((MobiusConfig*)obj)->setActiveSetup(value->getString());
}
UIParameterActiveSetupClass UIParameterActiveSetupObj;
UIParameter* UIParameterActiveSetup = &UIParameterActiveSetupObj;

////////////// ActiveOverlay

class UIParameterActiveOverlayClass : public UIParameter
{
  public:
    UIParameterActiveOverlayClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterActiveOverlayClass::UIParameterActiveOverlayClass()
{
    name = "activeOverlay";
    displayName = "Active Overlay";
    coreName = "bindings";
    scope = ScopeGlobal;
    type = TypeString;
}
void UIParameterActiveOverlayClass::getValue(void* obj, ExValue* value)
{
    value->setString(((MobiusConfig*)obj)->getOverlayBindings());
}
void UIParameterActiveOverlayClass::setValue(void* obj, ExValue* value)
{
    ((MobiusConfig*)obj)->setOverlayBindings(value->getString());
}
UIParameterActiveOverlayClass UIParameterActiveOverlayObj;
UIParameter* UIParameterActiveOverlay = &UIParameterActiveOverlayObj;

////////////// FadeFrames

class UIParameterFadeFramesClass : public UIParameter
{
  public:
    UIParameterFadeFramesClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterFadeFramesClass::UIParameterFadeFramesClass()
{
    name = "fadeFrames";
    displayName = "Fade Frames";
    scope = ScopeGlobal;
    type = TypeInt;
    high = 1024;
}
void UIParameterFadeFramesClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((MobiusConfig*)obj)->getFadeFrames());
}
void UIParameterFadeFramesClass::setValue(void* obj, ExValue* value)
{
    ((MobiusConfig*)obj)->setFadeFrames(value->getInt());
}
UIParameterFadeFramesClass UIParameterFadeFramesObj;
UIParameter* UIParameterFadeFrames = &UIParameterFadeFramesObj;

////////////// MaxSyncDrift

class UIParameterMaxSyncDriftClass : public UIParameter
{
  public:
    UIParameterMaxSyncDriftClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterMaxSyncDriftClass::UIParameterMaxSyncDriftClass()
{
    name = "maxSyncDrift";
    displayName = "Max Sync Drift";
    scope = ScopeGlobal;
    type = TypeInt;
    high = 10000;
}
void UIParameterMaxSyncDriftClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((MobiusConfig*)obj)->getMaxSyncDrift());
}
void UIParameterMaxSyncDriftClass::setValue(void* obj, ExValue* value)
{
    ((MobiusConfig*)obj)->setMaxSyncDrift(value->getInt());
}
UIParameterMaxSyncDriftClass UIParameterMaxSyncDriftObj;
UIParameter* UIParameterMaxSyncDrift = &UIParameterMaxSyncDriftObj;

////////////// DriftCheckPoint

class UIParameterDriftCheckPointClass : public UIParameter
{
  public:
    UIParameterDriftCheckPointClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterDriftCheckPointClass::UIParameterDriftCheckPointClass()
{
    name = "driftCheckPoint";
    displayName = "Drift Check Point";
    scope = ScopeGlobal;
    type = TypeInt;
}
void UIParameterDriftCheckPointClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((MobiusConfig*)obj)->getDriftCheckPoint());
}
void UIParameterDriftCheckPointClass::setValue(void* obj, ExValue* value)
{
    ((MobiusConfig*)obj)->setDriftCheckPoint((DriftCheckPoint)value->getInt());
}
UIParameterDriftCheckPointClass UIParameterDriftCheckPointObj;
UIParameter* UIParameterDriftCheckPoint = &UIParameterDriftCheckPointObj;

////////////// PluginPorts

class UIParameterPluginPortsClass : public UIParameter
{
  public:
    UIParameterPluginPortsClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterPluginPortsClass::UIParameterPluginPortsClass()
{
    name = "pluginPorts";
    displayName = "Plugin Ports";
    scope = ScopeGlobal;
    type = TypeInt;
    low = 1;
    high = 8;
}
void UIParameterPluginPortsClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((MobiusConfig*)obj)->getPluginPorts());
}
void UIParameterPluginPortsClass::setValue(void* obj, ExValue* value)
{
    ((MobiusConfig*)obj)->setPluginPorts(value->getInt());
}
UIParameterPluginPortsClass UIParameterPluginPortsObj;
UIParameter* UIParameterPluginPorts = &UIParameterPluginPortsObj;

////////////// LongPress

class UIParameterLongPressClass : public UIParameter
{
  public:
    UIParameterLongPressClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterLongPressClass::UIParameterLongPressClass()
{
    name = "longPress";
    displayName = "Long Press";
    scope = ScopeGlobal;
    type = TypeInt;
    low = 250;
    high = 10000;
}
void UIParameterLongPressClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((MobiusConfig*)obj)->getLongPress());
}
void UIParameterLongPressClass::setValue(void* obj, ExValue* value)
{
    ((MobiusConfig*)obj)->setLongPress(value->getInt());
}
UIParameterLongPressClass UIParameterLongPressObj;
UIParameter* UIParameterLongPress = &UIParameterLongPressObj;

////////////// SpreadRange

class UIParameterSpreadRangeClass : public UIParameter
{
  public:
    UIParameterSpreadRangeClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterSpreadRangeClass::UIParameterSpreadRangeClass()
{
    name = "spreadRange";
    displayName = "Spread Range";
    scope = ScopeGlobal;
    type = TypeInt;
    low = 1;
    high = 128;
}
void UIParameterSpreadRangeClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((MobiusConfig*)obj)->getSpreadRange());
}
void UIParameterSpreadRangeClass::setValue(void* obj, ExValue* value)
{
    ((MobiusConfig*)obj)->setSpreadRange(value->getInt());
}
UIParameterSpreadRangeClass UIParameterSpreadRangeObj;
UIParameter* UIParameterSpreadRange = &UIParameterSpreadRangeObj;

////////////// TraceLevel

class UIParameterTraceLevelClass : public UIParameter
{
  public:
    UIParameterTraceLevelClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterTraceLevelClass::UIParameterTraceLevelClass()
{
    name = "traceLevel";
    displayName = "Trace Level";
    coreName = "traceDebugLevel";
    scope = ScopeGlobal;
    type = TypeInt;
}
void UIParameterTraceLevelClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((MobiusConfig*)obj)->getTraceDebugLevel());
}
void UIParameterTraceLevelClass::setValue(void* obj, ExValue* value)
{
    ((MobiusConfig*)obj)->setTraceDebugLevel(value->getInt());
}
UIParameterTraceLevelClass UIParameterTraceLevelObj;
UIParameter* UIParameterTraceLevel = &UIParameterTraceLevelObj;

////////////// AutoFeedbackReduction

class UIParameterAutoFeedbackReductionClass : public UIParameter
{
  public:
    UIParameterAutoFeedbackReductionClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterAutoFeedbackReductionClass::UIParameterAutoFeedbackReductionClass()
{
    name = "autoFeedbackReduction";
    displayName = "Auto Feedback Reduction";
    scope = ScopeGlobal;
    type = TypeBool;
}
void UIParameterAutoFeedbackReductionClass::getValue(void* obj, ExValue* value)
{
    value->setBool(((MobiusConfig*)obj)->isAutoFeedbackReduction());
}
void UIParameterAutoFeedbackReductionClass::setValue(void* obj, ExValue* value)
{
    ((MobiusConfig*)obj)->setAutoFeedbackReduction(value->getBool());
}
UIParameterAutoFeedbackReductionClass UIParameterAutoFeedbackReductionObj;
UIParameter* UIParameterAutoFeedbackReduction = &UIParameterAutoFeedbackReductionObj;

////////////// IsolateOverdubs

class UIParameterIsolateOverdubsClass : public UIParameter
{
  public:
    UIParameterIsolateOverdubsClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterIsolateOverdubsClass::UIParameterIsolateOverdubsClass()
{
    name = "isolateOverdubs";
    displayName = "Isolate Overdubs";
    scope = ScopeGlobal;
    type = TypeBool;
}
void UIParameterIsolateOverdubsClass::getValue(void* obj, ExValue* value)
{
    value->setBool(((MobiusConfig*)obj)->isIsolateOverdubs());
}
void UIParameterIsolateOverdubsClass::setValue(void* obj, ExValue* value)
{
    ((MobiusConfig*)obj)->setIsolateOverdubs(value->getBool());
}
UIParameterIsolateOverdubsClass UIParameterIsolateOverdubsObj;
UIParameter* UIParameterIsolateOverdubs = &UIParameterIsolateOverdubsObj;

////////////// MonitorAudio

class UIParameterMonitorAudioClass : public UIParameter
{
  public:
    UIParameterMonitorAudioClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterMonitorAudioClass::UIParameterMonitorAudioClass()
{
    name = "monitorAudio";
    displayName = "Monitor Audio";
    scope = ScopeGlobal;
    type = TypeBool;
}
void UIParameterMonitorAudioClass::getValue(void* obj, ExValue* value)
{
    value->setBool(((MobiusConfig*)obj)->isMonitorAudio());
}
void UIParameterMonitorAudioClass::setValue(void* obj, ExValue* value)
{
    ((MobiusConfig*)obj)->setMonitorAudio(value->getBool());
}
UIParameterMonitorAudioClass UIParameterMonitorAudioObj;
UIParameter* UIParameterMonitorAudio = &UIParameterMonitorAudioObj;

////////////// SaveLayers

class UIParameterSaveLayersClass : public UIParameter
{
  public:
    UIParameterSaveLayersClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterSaveLayersClass::UIParameterSaveLayersClass()
{
    name = "saveLayers";
    displayName = "Save Layers";
    scope = ScopeGlobal;
    type = TypeBool;
}
void UIParameterSaveLayersClass::getValue(void* obj, ExValue* value)
{
    value->setBool(((MobiusConfig*)obj)->isSaveLayers());
}
void UIParameterSaveLayersClass::setValue(void* obj, ExValue* value)
{
    ((MobiusConfig*)obj)->setSaveLayers(value->getBool());
}
UIParameterSaveLayersClass UIParameterSaveLayersObj;
UIParameter* UIParameterSaveLayers = &UIParameterSaveLayersObj;

////////////// QuickSave

class UIParameterQuickSaveClass : public UIParameter
{
  public:
    UIParameterQuickSaveClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterQuickSaveClass::UIParameterQuickSaveClass()
{
    name = "quickSave";
    displayName = "Quick Save";
    scope = ScopeGlobal;
    type = TypeString;
}
void UIParameterQuickSaveClass::getValue(void* obj, ExValue* value)
{
    value->setString(((MobiusConfig*)obj)->getQuickSave());
}
void UIParameterQuickSaveClass::setValue(void* obj, ExValue* value)
{
    ((MobiusConfig*)obj)->setQuickSave(value->getString());
}
UIParameterQuickSaveClass UIParameterQuickSaveObj;
UIParameter* UIParameterQuickSave = &UIParameterQuickSaveObj;

////////////// IntegerWaveFile

class UIParameterIntegerWaveFileClass : public UIParameter
{
  public:
    UIParameterIntegerWaveFileClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterIntegerWaveFileClass::UIParameterIntegerWaveFileClass()
{
    name = "integerWaveFile";
    displayName = "Integer Wave File";
    coreName = "16BitWaveFile";
    scope = ScopeGlobal;
    type = TypeBool;
}
void UIParameterIntegerWaveFileClass::getValue(void* obj, ExValue* value)
{
    value->setBool(((MobiusConfig*)obj)->isIntegerWaveFile());
}
void UIParameterIntegerWaveFileClass::setValue(void* obj, ExValue* value)
{
    ((MobiusConfig*)obj)->setIntegerWaveFile(value->getBool());
}
UIParameterIntegerWaveFileClass UIParameterIntegerWaveFileObj;
UIParameter* UIParameterIntegerWaveFile = &UIParameterIntegerWaveFileObj;

////////////// GroupFocusLock

class UIParameterGroupFocusLockClass : public UIParameter
{
  public:
    UIParameterGroupFocusLockClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterGroupFocusLockClass::UIParameterGroupFocusLockClass()
{
    name = "groupFocusLock";
    displayName = "Group Focus Lock";
    scope = ScopeGlobal;
    type = TypeBool;
}
void UIParameterGroupFocusLockClass::getValue(void* obj, ExValue* value)
{
    value->setBool(((MobiusConfig*)obj)->isGroupFocusLock());
}
void UIParameterGroupFocusLockClass::setValue(void* obj, ExValue* value)
{
    ((MobiusConfig*)obj)->setGroupFocusLock(value->getBool());
}
UIParameterGroupFocusLockClass UIParameterGroupFocusLockObj;
UIParameter* UIParameterGroupFocusLock = &UIParameterGroupFocusLockObj;

////////////// TrackCount

class UIParameterTrackCountClass : public UIParameter
{
  public:
    UIParameterTrackCountClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterTrackCountClass::UIParameterTrackCountClass()
{
    name = "trackCount";
    displayName = "Track Count";
    coreName = "tracks";
    scope = ScopeGlobal;
    type = TypeInt;
}
void UIParameterTrackCountClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((MobiusConfig*)obj)->getTracks());
}
void UIParameterTrackCountClass::setValue(void* obj, ExValue* value)
{
    ((MobiusConfig*)obj)->setTracks(value->getInt());
}
UIParameterTrackCountClass UIParameterTrackCountObj;
UIParameter* UIParameterTrackCount = &UIParameterTrackCountObj;

////////////// GroupCount

class UIParameterGroupCountClass : public UIParameter
{
  public:
    UIParameterGroupCountClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterGroupCountClass::UIParameterGroupCountClass()
{
    name = "groupCount";
    displayName = "Group Count";
    coreName = "trackGroups";
    scope = ScopeGlobal;
    type = TypeInt;
}
void UIParameterGroupCountClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((MobiusConfig*)obj)->getTrackGroups());
}
void UIParameterGroupCountClass::setValue(void* obj, ExValue* value)
{
    ((MobiusConfig*)obj)->setTrackGroups(value->getInt());
}
UIParameterGroupCountClass UIParameterGroupCountObj;
UIParameter* UIParameterGroupCount = &UIParameterGroupCountObj;

////////////// MaxLoops

class UIParameterMaxLoopsClass : public UIParameter
{
  public:
    UIParameterMaxLoopsClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterMaxLoopsClass::UIParameterMaxLoopsClass()
{
    name = "maxLoops";
    displayName = "Max Loops";
    scope = ScopeGlobal;
    type = TypeInt;
}
void UIParameterMaxLoopsClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((MobiusConfig*)obj)->getMaxLoops());
}
void UIParameterMaxLoopsClass::setValue(void* obj, ExValue* value)
{
    ((MobiusConfig*)obj)->setMaxLoops(value->getInt());
}
UIParameterMaxLoopsClass UIParameterMaxLoopsObj;
UIParameter* UIParameterMaxLoops = &UIParameterMaxLoopsObj;

////////////// InputLatency

class UIParameterInputLatencyClass : public UIParameter
{
  public:
    UIParameterInputLatencyClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterInputLatencyClass::UIParameterInputLatencyClass()
{
    name = "inputLatency";
    displayName = "Input Latency";
    scope = ScopeGlobal;
    type = TypeInt;
}
void UIParameterInputLatencyClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((MobiusConfig*)obj)->getInputLatency());
}
void UIParameterInputLatencyClass::setValue(void* obj, ExValue* value)
{
    ((MobiusConfig*)obj)->setInputLatency(value->getInt());
}
UIParameterInputLatencyClass UIParameterInputLatencyObj;
UIParameter* UIParameterInputLatency = &UIParameterInputLatencyObj;

////////////// OutputLatency

class UIParameterOutputLatencyClass : public UIParameter
{
  public:
    UIParameterOutputLatencyClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterOutputLatencyClass::UIParameterOutputLatencyClass()
{
    name = "outputLatency";
    displayName = "Output Latency";
    scope = ScopeGlobal;
    type = TypeInt;
}
void UIParameterOutputLatencyClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((MobiusConfig*)obj)->getOutputLatency());
}
void UIParameterOutputLatencyClass::setValue(void* obj, ExValue* value)
{
    ((MobiusConfig*)obj)->setOutputLatency(value->getInt());
}
UIParameterOutputLatencyClass UIParameterOutputLatencyObj;
UIParameter* UIParameterOutputLatency = &UIParameterOutputLatencyObj;

////////////// MidiInput

class UIParameterMidiInputClass : public UIParameter
{
  public:
    UIParameterMidiInputClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterMidiInputClass::UIParameterMidiInputClass()
{
    name = "midiInput";
    displayName = "Midi Input";
    scope = ScopeGlobal;
    type = TypeString;
}
void UIParameterMidiInputClass::getValue(void* obj, ExValue* value)
{
    value->setString(((MobiusConfig*)obj)->getMidiInput());
}
void UIParameterMidiInputClass::setValue(void* obj, ExValue* value)
{
    ((MobiusConfig*)obj)->setMidiInput(value->getString());
}
UIParameterMidiInputClass UIParameterMidiInputObj;
UIParameter* UIParameterMidiInput = &UIParameterMidiInputObj;

////////////// MidiOutput

class UIParameterMidiOutputClass : public UIParameter
{
  public:
    UIParameterMidiOutputClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterMidiOutputClass::UIParameterMidiOutputClass()
{
    name = "midiOutput";
    displayName = "Midi Output";
    scope = ScopeGlobal;
    type = TypeString;
}
void UIParameterMidiOutputClass::getValue(void* obj, ExValue* value)
{
    value->setString(((MobiusConfig*)obj)->getMidiOutput());
}
void UIParameterMidiOutputClass::setValue(void* obj, ExValue* value)
{
    ((MobiusConfig*)obj)->setMidiOutput(value->getString());
}
UIParameterMidiOutputClass UIParameterMidiOutputObj;
UIParameter* UIParameterMidiOutput = &UIParameterMidiOutputObj;

////////////// MidiThrough

class UIParameterMidiThroughClass : public UIParameter
{
  public:
    UIParameterMidiThroughClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterMidiThroughClass::UIParameterMidiThroughClass()
{
    name = "midiThrough";
    displayName = "Midi Through";
    scope = ScopeGlobal;
    type = TypeString;
}
void UIParameterMidiThroughClass::getValue(void* obj, ExValue* value)
{
    value->setString(((MobiusConfig*)obj)->getMidiThrough());
}
void UIParameterMidiThroughClass::setValue(void* obj, ExValue* value)
{
    ((MobiusConfig*)obj)->setMidiThrough(value->getString());
}
UIParameterMidiThroughClass UIParameterMidiThroughObj;
UIParameter* UIParameterMidiThrough = &UIParameterMidiThroughObj;

////////////// PluginMidiInput

class UIParameterPluginMidiInputClass : public UIParameter
{
  public:
    UIParameterPluginMidiInputClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterPluginMidiInputClass::UIParameterPluginMidiInputClass()
{
    name = "pluginMidiInput";
    displayName = "Plugin Midi Input";
    scope = ScopeGlobal;
    type = TypeString;
}
void UIParameterPluginMidiInputClass::getValue(void* obj, ExValue* value)
{
    value->setString(((MobiusConfig*)obj)->getPluginMidiInput());
}
void UIParameterPluginMidiInputClass::setValue(void* obj, ExValue* value)
{
    ((MobiusConfig*)obj)->setPluginMidiInput(value->getString());
}
UIParameterPluginMidiInputClass UIParameterPluginMidiInputObj;
UIParameter* UIParameterPluginMidiInput = &UIParameterPluginMidiInputObj;

////////////// PluginMidiOutput

class UIParameterPluginMidiOutputClass : public UIParameter
{
  public:
    UIParameterPluginMidiOutputClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterPluginMidiOutputClass::UIParameterPluginMidiOutputClass()
{
    name = "pluginMidiOutput";
    displayName = "Plugin Midi Output";
    scope = ScopeGlobal;
    type = TypeString;
}
void UIParameterPluginMidiOutputClass::getValue(void* obj, ExValue* value)
{
    value->setString(((MobiusConfig*)obj)->getPluginMidiOutput());
}
void UIParameterPluginMidiOutputClass::setValue(void* obj, ExValue* value)
{
    ((MobiusConfig*)obj)->setPluginMidiOutput(value->getString());
}
UIParameterPluginMidiOutputClass UIParameterPluginMidiOutputObj;
UIParameter* UIParameterPluginMidiOutput = &UIParameterPluginMidiOutputObj;

////////////// PluginMidiThrough

class UIParameterPluginMidiThroughClass : public UIParameter
{
  public:
    UIParameterPluginMidiThroughClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterPluginMidiThroughClass::UIParameterPluginMidiThroughClass()
{
    name = "pluginMidiThrough";
    displayName = "Plugin Midi Through";
    scope = ScopeGlobal;
    type = TypeString;
}
void UIParameterPluginMidiThroughClass::getValue(void* obj, ExValue* value)
{
    value->setString(((MobiusConfig*)obj)->getPluginMidiThrough());
}
void UIParameterPluginMidiThroughClass::setValue(void* obj, ExValue* value)
{
    ((MobiusConfig*)obj)->setPluginMidiThrough(value->getString());
}
UIParameterPluginMidiThroughClass UIParameterPluginMidiThroughObj;
UIParameter* UIParameterPluginMidiThrough = &UIParameterPluginMidiThroughObj;

////////////// AudioInput

class UIParameterAudioInputClass : public UIParameter
{
  public:
    UIParameterAudioInputClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterAudioInputClass::UIParameterAudioInputClass()
{
    name = "audioInput";
    displayName = "Audio Input";
    scope = ScopeGlobal;
    type = TypeString;
}
void UIParameterAudioInputClass::getValue(void* obj, ExValue* value)
{
    value->setString(((MobiusConfig*)obj)->getAudioInput());
}
void UIParameterAudioInputClass::setValue(void* obj, ExValue* value)
{
    ((MobiusConfig*)obj)->setAudioInput(value->getString());
}
UIParameterAudioInputClass UIParameterAudioInputObj;
UIParameter* UIParameterAudioInput = &UIParameterAudioInputObj;

//******************** preset


////////////// Subcycles

class UIParameterSubcyclesClass : public UIParameter
{
  public:
    UIParameterSubcyclesClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterSubcyclesClass::UIParameterSubcyclesClass()
{
    name = "subcycles";
    displayName = "Subcycles";
    scope = ScopePreset;
    type = TypeInt;
    low = 1;
    high = 128;
}
void UIParameterSubcyclesClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getSubcycles());
}
void UIParameterSubcyclesClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setSubcycles(value->getInt());
}
UIParameterSubcyclesClass UIParameterSubcyclesObj;
UIParameter* UIParameterSubcycles = &UIParameterSubcyclesObj;

////////////// MultiplyMode

class UIParameterMultiplyModeClass : public UIParameter
{
  public:
    UIParameterMultiplyModeClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterMultiplyModeClass::UIParameterMultiplyModeClass()
{
    name = "multiplyMode";
    displayName = "Multiply Mode";
    scope = ScopePreset;
    type = TypeInt;
}
void UIParameterMultiplyModeClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getMultiplyMode());
}
void UIParameterMultiplyModeClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setMultiplyMode((Preset::MultiplyMode)value->getInt());
}
UIParameterMultiplyModeClass UIParameterMultiplyModeObj;
UIParameter* UIParameterMultiplyMode = &UIParameterMultiplyModeObj;

////////////// ShuffleMode

class UIParameterShuffleModeClass : public UIParameter
{
  public:
    UIParameterShuffleModeClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterShuffleModeClass::UIParameterShuffleModeClass()
{
    name = "shuffleMode";
    displayName = "Shuffle Mode";
    scope = ScopePreset;
    type = TypeInt;
}
void UIParameterShuffleModeClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getShuffleMode());
}
void UIParameterShuffleModeClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setShuffleMode((Preset::ShuffleMode)value->getInt());
}
UIParameterShuffleModeClass UIParameterShuffleModeObj;
UIParameter* UIParameterShuffleMode = &UIParameterShuffleModeObj;

////////////// AltFeedbackEnable

class UIParameterAltFeedbackEnableClass : public UIParameter
{
  public:
    UIParameterAltFeedbackEnableClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterAltFeedbackEnableClass::UIParameterAltFeedbackEnableClass()
{
    name = "altFeedbackEnable";
    displayName = "Alt Feedback Enable";
    scope = ScopePreset;
    type = TypeBool;
}
void UIParameterAltFeedbackEnableClass::getValue(void* obj, ExValue* value)
{
    value->setBool(((Preset*)obj)->isAltFeedbackEnable());
}
void UIParameterAltFeedbackEnableClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setAltFeedbackEnable(value->getBool());
}
UIParameterAltFeedbackEnableClass UIParameterAltFeedbackEnableObj;
UIParameter* UIParameterAltFeedbackEnable = &UIParameterAltFeedbackEnableObj;

////////////// EmptyLoopAction

class UIParameterEmptyLoopActionClass : public UIParameter
{
  public:
    UIParameterEmptyLoopActionClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterEmptyLoopActionClass::UIParameterEmptyLoopActionClass()
{
    name = "emptyLoopAction";
    displayName = "Empty Loop Action";
    scope = ScopePreset;
    type = TypeInt;
}
void UIParameterEmptyLoopActionClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getEmptyLoopAction());
}
void UIParameterEmptyLoopActionClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setEmptyLoopAction((Preset::EmptyLoopAction)value->getInt());
}
UIParameterEmptyLoopActionClass UIParameterEmptyLoopActionObj;
UIParameter* UIParameterEmptyLoopAction = &UIParameterEmptyLoopActionObj;

////////////// EmptyTrackAction

class UIParameterEmptyTrackActionClass : public UIParameter
{
  public:
    UIParameterEmptyTrackActionClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterEmptyTrackActionClass::UIParameterEmptyTrackActionClass()
{
    name = "emptyTrackAction";
    displayName = "Empty Track Action";
    scope = ScopePreset;
    type = TypeInt;
}
void UIParameterEmptyTrackActionClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getEmptyTrackAction());
}
void UIParameterEmptyTrackActionClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setEmptyTrackAction((Preset::EmptyLoopAction)value->getInt());
}
UIParameterEmptyTrackActionClass UIParameterEmptyTrackActionObj;
UIParameter* UIParameterEmptyTrackAction = &UIParameterEmptyTrackActionObj;

////////////// TrackLeaveAction

class UIParameterTrackLeaveActionClass : public UIParameter
{
  public:
    UIParameterTrackLeaveActionClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterTrackLeaveActionClass::UIParameterTrackLeaveActionClass()
{
    name = "trackLeaveAction";
    displayName = "Track Leave Action";
    scope = ScopePreset;
    type = TypeInt;
}
void UIParameterTrackLeaveActionClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getTrackLeaveAction());
}
void UIParameterTrackLeaveActionClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setTrackLeaveAction((Preset::TrackLeaveAction)value->getInt());
}
UIParameterTrackLeaveActionClass UIParameterTrackLeaveActionObj;
UIParameter* UIParameterTrackLeaveAction = &UIParameterTrackLeaveActionObj;

////////////// LoopCount

class UIParameterLoopCountClass : public UIParameter
{
  public:
    UIParameterLoopCountClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterLoopCountClass::UIParameterLoopCountClass()
{
    name = "loopCount";
    displayName = "Loop Count";
    scope = ScopePreset;
    type = TypeInt;
    low = 1;
    high = 32;
    dynamic = true;
    noBinding = true;
}
void UIParameterLoopCountClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getLoops());
}
void UIParameterLoopCountClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setLoops(value->getInt());
}
UIParameterLoopCountClass UIParameterLoopCountObj;
UIParameter* UIParameterLoopCount = &UIParameterLoopCountObj;

////////////// MuteMode

class UIParameterMuteModeClass : public UIParameter
{
  public:
    UIParameterMuteModeClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterMuteModeClass::UIParameterMuteModeClass()
{
    name = "muteMode";
    displayName = "Mute Mode";
    scope = ScopePreset;
    type = TypeInt;
}
void UIParameterMuteModeClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getMuteMode());
}
void UIParameterMuteModeClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setMuteMode((Preset::MuteMode)value->getInt());
}
UIParameterMuteModeClass UIParameterMuteModeObj;
UIParameter* UIParameterMuteMode = &UIParameterMuteModeObj;

////////////// MuteCancel

class UIParameterMuteCancelClass : public UIParameter
{
  public:
    UIParameterMuteCancelClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterMuteCancelClass::UIParameterMuteCancelClass()
{
    name = "muteCancel";
    displayName = "Mute Cancel";
    scope = ScopePreset;
    type = TypeInt;
}
void UIParameterMuteCancelClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getMuteCancel());
}
void UIParameterMuteCancelClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setMuteCancel((Preset::MuteCancel)value->getInt());
}
UIParameterMuteCancelClass UIParameterMuteCancelObj;
UIParameter* UIParameterMuteCancel = &UIParameterMuteCancelObj;

////////////// OverdubQuantized

class UIParameterOverdubQuantizedClass : public UIParameter
{
  public:
    UIParameterOverdubQuantizedClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterOverdubQuantizedClass::UIParameterOverdubQuantizedClass()
{
    name = "overdubQuantized";
    displayName = "Overdub Quantized";
    scope = ScopePreset;
    type = TypeBool;
}
void UIParameterOverdubQuantizedClass::getValue(void* obj, ExValue* value)
{
    value->setBool(((Preset*)obj)->isOverdubQuantized());
}
void UIParameterOverdubQuantizedClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setOverdubQuantized(value->getBool());
}
UIParameterOverdubQuantizedClass UIParameterOverdubQuantizedObj;
UIParameter* UIParameterOverdubQuantized = &UIParameterOverdubQuantizedObj;

////////////// Quantize

class UIParameterQuantizeClass : public UIParameter
{
  public:
    UIParameterQuantizeClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterQuantizeClass::UIParameterQuantizeClass()
{
    name = "quantize";
    displayName = "Quantize";
    scope = ScopePreset;
    type = TypeInt;
}
void UIParameterQuantizeClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getQuantize());
}
void UIParameterQuantizeClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setQuantize((Preset::QuantizeMode)value->getInt());
}
UIParameterQuantizeClass UIParameterQuantizeObj;
UIParameter* UIParameterQuantize = &UIParameterQuantizeObj;

////////////// BounceQuantize

class UIParameterBounceQuantizeClass : public UIParameter
{
  public:
    UIParameterBounceQuantizeClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterBounceQuantizeClass::UIParameterBounceQuantizeClass()
{
    name = "bounceQuantize";
    displayName = "Bounce Quantize";
    scope = ScopePreset;
    type = TypeInt;
}
void UIParameterBounceQuantizeClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getBounceQuantize());
}
void UIParameterBounceQuantizeClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setBounceQuantize((Preset::QuantizeMode)value->getInt());
}
UIParameterBounceQuantizeClass UIParameterBounceQuantizeObj;
UIParameter* UIParameterBounceQuantize = &UIParameterBounceQuantizeObj;

////////////// RecordResetsFeedback

class UIParameterRecordResetsFeedbackClass : public UIParameter
{
  public:
    UIParameterRecordResetsFeedbackClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterRecordResetsFeedbackClass::UIParameterRecordResetsFeedbackClass()
{
    name = "recordResetsFeedback";
    displayName = "Record Resets Feedback";
    scope = ScopePreset;
    type = TypeBool;
}
void UIParameterRecordResetsFeedbackClass::getValue(void* obj, ExValue* value)
{
    value->setBool(((Preset*)obj)->isRecordResetsFeedback());
}
void UIParameterRecordResetsFeedbackClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setRecordResetsFeedback(value->getBool());
}
UIParameterRecordResetsFeedbackClass UIParameterRecordResetsFeedbackObj;
UIParameter* UIParameterRecordResetsFeedback = &UIParameterRecordResetsFeedbackObj;

////////////// SpeedRecord

class UIParameterSpeedRecordClass : public UIParameter
{
  public:
    UIParameterSpeedRecordClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterSpeedRecordClass::UIParameterSpeedRecordClass()
{
    name = "speedRecord";
    displayName = "Speed Record";
    scope = ScopePreset;
    type = TypeBool;
}
void UIParameterSpeedRecordClass::getValue(void* obj, ExValue* value)
{
    value->setBool(((Preset*)obj)->isSpeedRecord());
}
void UIParameterSpeedRecordClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setSpeedRecord(value->getBool());
}
UIParameterSpeedRecordClass UIParameterSpeedRecordObj;
UIParameter* UIParameterSpeedRecord = &UIParameterSpeedRecordObj;

////////////// RoundingOverdub

class UIParameterRoundingOverdubClass : public UIParameter
{
  public:
    UIParameterRoundingOverdubClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterRoundingOverdubClass::UIParameterRoundingOverdubClass()
{
    name = "roundingOverdub";
    displayName = "Rounding Overdub";
    scope = ScopePreset;
    type = TypeBool;
}
void UIParameterRoundingOverdubClass::getValue(void* obj, ExValue* value)
{
    value->setBool(((Preset*)obj)->isRoundingOverdub());
}
void UIParameterRoundingOverdubClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setRoundingOverdub(value->getBool());
}
UIParameterRoundingOverdubClass UIParameterRoundingOverdubObj;
UIParameter* UIParameterRoundingOverdub = &UIParameterRoundingOverdubObj;

////////////// SwitchLocation

class UIParameterSwitchLocationClass : public UIParameter
{
  public:
    UIParameterSwitchLocationClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterSwitchLocationClass::UIParameterSwitchLocationClass()
{
    name = "switchLocation";
    displayName = "Switch Location";
    scope = ScopePreset;
    type = TypeInt;
}
void UIParameterSwitchLocationClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getSwitchLocation());
}
void UIParameterSwitchLocationClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setSwitchLocation((Preset::SwitchLocation)value->getInt());
}
UIParameterSwitchLocationClass UIParameterSwitchLocationObj;
UIParameter* UIParameterSwitchLocation = &UIParameterSwitchLocationObj;

////////////// ReturnLocation

class UIParameterReturnLocationClass : public UIParameter
{
  public:
    UIParameterReturnLocationClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterReturnLocationClass::UIParameterReturnLocationClass()
{
    name = "returnLocation";
    displayName = "Return Location";
    scope = ScopePreset;
    type = TypeInt;
}
void UIParameterReturnLocationClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getReturnLocation());
}
void UIParameterReturnLocationClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setReturnLocation((Preset::SwitchLocation)value->getInt());
}
UIParameterReturnLocationClass UIParameterReturnLocationObj;
UIParameter* UIParameterReturnLocation = &UIParameterReturnLocationObj;

////////////// SwitchQuantize

class UIParameterSwitchQuantizeClass : public UIParameter
{
  public:
    UIParameterSwitchQuantizeClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterSwitchQuantizeClass::UIParameterSwitchQuantizeClass()
{
    name = "switchQuantize";
    displayName = "Switch Quantize";
    scope = ScopePreset;
    type = TypeInt;
}
void UIParameterSwitchQuantizeClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getSwitchQuantize());
}
void UIParameterSwitchQuantizeClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setSwitchQuantize((Preset::SwitchQuantize)value->getInt());
}
UIParameterSwitchQuantizeClass UIParameterSwitchQuantizeObj;
UIParameter* UIParameterSwitchQuantize = &UIParameterSwitchQuantizeObj;

////////////// TimeCopyMode

class UIParameterTimeCopyModeClass : public UIParameter
{
  public:
    UIParameterTimeCopyModeClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterTimeCopyModeClass::UIParameterTimeCopyModeClass()
{
    name = "timeCopyMode";
    displayName = "Time Copy Mode";
    scope = ScopePreset;
    type = TypeInt;
}
void UIParameterTimeCopyModeClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getTimeCopyMode());
}
void UIParameterTimeCopyModeClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setTimeCopyMode((Preset::CopyMode)value->getInt());
}
UIParameterTimeCopyModeClass UIParameterTimeCopyModeObj;
UIParameter* UIParameterTimeCopyMode = &UIParameterTimeCopyModeObj;

////////////// SoundCopyMode

class UIParameterSoundCopyModeClass : public UIParameter
{
  public:
    UIParameterSoundCopyModeClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterSoundCopyModeClass::UIParameterSoundCopyModeClass()
{
    name = "soundCopyMode";
    displayName = "Sound Copy Mode";
    scope = ScopePreset;
    type = TypeInt;
}
void UIParameterSoundCopyModeClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getSoundCopyMode());
}
void UIParameterSoundCopyModeClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setSoundCopyMode((Preset::CopyMode)value->getInt());
}
UIParameterSoundCopyModeClass UIParameterSoundCopyModeObj;
UIParameter* UIParameterSoundCopyMode = &UIParameterSoundCopyModeObj;

////////////// RecordThreshold

class UIParameterRecordThresholdClass : public UIParameter
{
  public:
    UIParameterRecordThresholdClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterRecordThresholdClass::UIParameterRecordThresholdClass()
{
    name = "recordThreshold";
    displayName = "Record Threshold";
    scope = ScopePreset;
    type = TypeInt;
    high = 8;
}
void UIParameterRecordThresholdClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getRecordThreshold());
}
void UIParameterRecordThresholdClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setRecordThreshold(value->getInt());
}
UIParameterRecordThresholdClass UIParameterRecordThresholdObj;
UIParameter* UIParameterRecordThreshold = &UIParameterRecordThresholdObj;

////////////// SwitchVelocity

class UIParameterSwitchVelocityClass : public UIParameter
{
  public:
    UIParameterSwitchVelocityClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterSwitchVelocityClass::UIParameterSwitchVelocityClass()
{
    name = "switchVelocity";
    displayName = "Switch Velocity";
    scope = ScopePreset;
    type = TypeBool;
}
void UIParameterSwitchVelocityClass::getValue(void* obj, ExValue* value)
{
    value->setBool(((Preset*)obj)->isSwitchVelocity());
}
void UIParameterSwitchVelocityClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setSwitchVelocity(value->getBool());
}
UIParameterSwitchVelocityClass UIParameterSwitchVelocityObj;
UIParameter* UIParameterSwitchVelocity = &UIParameterSwitchVelocityObj;

////////////// MaxUndo

class UIParameterMaxUndoClass : public UIParameter
{
  public:
    UIParameterMaxUndoClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterMaxUndoClass::UIParameterMaxUndoClass()
{
    name = "maxUndo";
    displayName = "Max Undo";
    scope = ScopePreset;
    type = TypeInt;
    noBinding = true;
}
void UIParameterMaxUndoClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getMaxUndo());
}
void UIParameterMaxUndoClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setMaxUndo(value->getInt());
}
UIParameterMaxUndoClass UIParameterMaxUndoObj;
UIParameter* UIParameterMaxUndo = &UIParameterMaxUndoObj;

////////////// MaxRedo

class UIParameterMaxRedoClass : public UIParameter
{
  public:
    UIParameterMaxRedoClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterMaxRedoClass::UIParameterMaxRedoClass()
{
    name = "maxRedo";
    displayName = "Max Redo";
    scope = ScopePreset;
    type = TypeInt;
    noBinding = true;
}
void UIParameterMaxRedoClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getMaxRedo());
}
void UIParameterMaxRedoClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setMaxRedo(value->getInt());
}
UIParameterMaxRedoClass UIParameterMaxRedoObj;
UIParameter* UIParameterMaxRedo = &UIParameterMaxRedoObj;

////////////// NoFeedbackUndo

class UIParameterNoFeedbackUndoClass : public UIParameter
{
  public:
    UIParameterNoFeedbackUndoClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterNoFeedbackUndoClass::UIParameterNoFeedbackUndoClass()
{
    name = "noFeedbackUndo";
    displayName = "No Feedback Undo";
    scope = ScopePreset;
    type = TypeBool;
    noBinding = true;
}
void UIParameterNoFeedbackUndoClass::getValue(void* obj, ExValue* value)
{
    value->setBool(((Preset*)obj)->isNoFeedbackUndo());
}
void UIParameterNoFeedbackUndoClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setNoFeedbackUndo(value->getBool());
}
UIParameterNoFeedbackUndoClass UIParameterNoFeedbackUndoObj;
UIParameter* UIParameterNoFeedbackUndo = &UIParameterNoFeedbackUndoObj;

////////////// NoLayerFlattening

class UIParameterNoLayerFlatteningClass : public UIParameter
{
  public:
    UIParameterNoLayerFlatteningClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterNoLayerFlatteningClass::UIParameterNoLayerFlatteningClass()
{
    name = "noLayerFlattening";
    displayName = "No Layer Flattening";
    scope = ScopePreset;
    type = TypeBool;
    noBinding = true;
}
void UIParameterNoLayerFlatteningClass::getValue(void* obj, ExValue* value)
{
    value->setBool(((Preset*)obj)->isNoLayerFlattening());
}
void UIParameterNoLayerFlatteningClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setNoLayerFlattening(value->getBool());
}
UIParameterNoLayerFlatteningClass UIParameterNoLayerFlatteningObj;
UIParameter* UIParameterNoLayerFlattening = &UIParameterNoLayerFlatteningObj;

////////////// SpeedShiftRestart

class UIParameterSpeedShiftRestartClass : public UIParameter
{
  public:
    UIParameterSpeedShiftRestartClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterSpeedShiftRestartClass::UIParameterSpeedShiftRestartClass()
{
    name = "speedShiftRestart";
    displayName = "Speed Shift Restart";
    scope = ScopePreset;
    type = TypeBool;
    noBinding = true;
}
void UIParameterSpeedShiftRestartClass::getValue(void* obj, ExValue* value)
{
    value->setBool(((Preset*)obj)->isSpeedShiftRestart());
}
void UIParameterSpeedShiftRestartClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setSpeedShiftRestart(value->getBool());
}
UIParameterSpeedShiftRestartClass UIParameterSpeedShiftRestartObj;
UIParameter* UIParameterSpeedShiftRestart = &UIParameterSpeedShiftRestartObj;

////////////// PitchShiftRestart

class UIParameterPitchShiftRestartClass : public UIParameter
{
  public:
    UIParameterPitchShiftRestartClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterPitchShiftRestartClass::UIParameterPitchShiftRestartClass()
{
    name = "pitchShiftRestart";
    displayName = "Pitch Shift Restart";
    scope = ScopePreset;
    type = TypeBool;
    noBinding = true;
}
void UIParameterPitchShiftRestartClass::getValue(void* obj, ExValue* value)
{
    value->setBool(((Preset*)obj)->isPitchShiftRestart());
}
void UIParameterPitchShiftRestartClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setPitchShiftRestart(value->getBool());
}
UIParameterPitchShiftRestartClass UIParameterPitchShiftRestartObj;
UIParameter* UIParameterPitchShiftRestart = &UIParameterPitchShiftRestartObj;

////////////// SpeedStepRange

class UIParameterSpeedStepRangeClass : public UIParameter
{
  public:
    UIParameterSpeedStepRangeClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterSpeedStepRangeClass::UIParameterSpeedStepRangeClass()
{
    name = "speedStepRange";
    displayName = "Speed Step Range";
    scope = ScopePreset;
    type = TypeInt;
    low = 1;
    high = 48;
    noBinding = true;
}
void UIParameterSpeedStepRangeClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getSpeedStepRange());
}
void UIParameterSpeedStepRangeClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setSpeedStepRange(value->getInt());
}
UIParameterSpeedStepRangeClass UIParameterSpeedStepRangeObj;
UIParameter* UIParameterSpeedStepRange = &UIParameterSpeedStepRangeObj;

////////////// SpeedBendRange

class UIParameterSpeedBendRangeClass : public UIParameter
{
  public:
    UIParameterSpeedBendRangeClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterSpeedBendRangeClass::UIParameterSpeedBendRangeClass()
{
    name = "speedBendRange";
    displayName = "Speed Bend Range";
    scope = ScopePreset;
    type = TypeInt;
    low = 1;
    high = 12;
    noBinding = true;
}
void UIParameterSpeedBendRangeClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getSpeedBendRange());
}
void UIParameterSpeedBendRangeClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setSpeedBendRange(value->getInt());
}
UIParameterSpeedBendRangeClass UIParameterSpeedBendRangeObj;
UIParameter* UIParameterSpeedBendRange = &UIParameterSpeedBendRangeObj;

////////////// PitchStepRange

class UIParameterPitchStepRangeClass : public UIParameter
{
  public:
    UIParameterPitchStepRangeClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterPitchStepRangeClass::UIParameterPitchStepRangeClass()
{
    name = "pitchStepRange";
    displayName = "Pitch Step Range";
    scope = ScopePreset;
    type = TypeInt;
    low = 1;
    high = 48;
    noBinding = true;
}
void UIParameterPitchStepRangeClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getPitchStepRange());
}
void UIParameterPitchStepRangeClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setPitchStepRange(value->getInt());
}
UIParameterPitchStepRangeClass UIParameterPitchStepRangeObj;
UIParameter* UIParameterPitchStepRange = &UIParameterPitchStepRangeObj;

////////////// PitchBendRange

class UIParameterPitchBendRangeClass : public UIParameter
{
  public:
    UIParameterPitchBendRangeClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterPitchBendRangeClass::UIParameterPitchBendRangeClass()
{
    name = "pitchBendRange";
    displayName = "Pitch Bend Range";
    scope = ScopePreset;
    type = TypeInt;
    low = 1;
    high = 12;
    noBinding = true;
}
void UIParameterPitchBendRangeClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getPitchBendRange());
}
void UIParameterPitchBendRangeClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setPitchBendRange(value->getInt());
}
UIParameterPitchBendRangeClass UIParameterPitchBendRangeObj;
UIParameter* UIParameterPitchBendRange = &UIParameterPitchBendRangeObj;

////////////// TimeStretchRange

class UIParameterTimeStretchRangeClass : public UIParameter
{
  public:
    UIParameterTimeStretchRangeClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterTimeStretchRangeClass::UIParameterTimeStretchRangeClass()
{
    name = "timeStretchRange";
    displayName = "Time Stretch Range";
    scope = ScopePreset;
    type = TypeInt;
    low = 1;
    high = 12;
    noBinding = true;
}
void UIParameterTimeStretchRangeClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getTimeStretchRange());
}
void UIParameterTimeStretchRangeClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setTimeStretchRange(value->getInt());
}
UIParameterTimeStretchRangeClass UIParameterTimeStretchRangeObj;
UIParameter* UIParameterTimeStretchRange = &UIParameterTimeStretchRangeObj;

////////////// SlipMode

class UIParameterSlipModeClass : public UIParameter
{
  public:
    UIParameterSlipModeClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterSlipModeClass::UIParameterSlipModeClass()
{
    name = "slipMode";
    displayName = "Slip Mode";
    scope = ScopePreset;
    type = TypeInt;
}
void UIParameterSlipModeClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getSlipMode());
}
void UIParameterSlipModeClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setSlipMode((Preset::SlipMode)value->getInt());
}
UIParameterSlipModeClass UIParameterSlipModeObj;
UIParameter* UIParameterSlipMode = &UIParameterSlipModeObj;

////////////// SlipTime

class UIParameterSlipTimeClass : public UIParameter
{
  public:
    UIParameterSlipTimeClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterSlipTimeClass::UIParameterSlipTimeClass()
{
    name = "slipTime";
    displayName = "Slip Time";
    scope = ScopePreset;
    type = TypeInt;
    high = 128;
}
void UIParameterSlipTimeClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getSlipTime());
}
void UIParameterSlipTimeClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setSlipTime(value->getInt());
}
UIParameterSlipTimeClass UIParameterSlipTimeObj;
UIParameter* UIParameterSlipTime = &UIParameterSlipTimeObj;

////////////// AutoRecordTempo

class UIParameterAutoRecordTempoClass : public UIParameter
{
  public:
    UIParameterAutoRecordTempoClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterAutoRecordTempoClass::UIParameterAutoRecordTempoClass()
{
    name = "autoRecordTempo";
    displayName = "Auto Record Tempo";
    scope = ScopePreset;
    type = TypeInt;
    high = 500;
}
void UIParameterAutoRecordTempoClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getAutoRecordTempo());
}
void UIParameterAutoRecordTempoClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setAutoRecordTempo(value->getInt());
}
UIParameterAutoRecordTempoClass UIParameterAutoRecordTempoObj;
UIParameter* UIParameterAutoRecordTempo = &UIParameterAutoRecordTempoObj;

////////////// AutoRecordBars

class UIParameterAutoRecordBarsClass : public UIParameter
{
  public:
    UIParameterAutoRecordBarsClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterAutoRecordBarsClass::UIParameterAutoRecordBarsClass()
{
    name = "autoRecordBars";
    displayName = "Auto Record Bars";
    scope = ScopePreset;
    type = TypeInt;
    low = 1;
    high = 64;
}
void UIParameterAutoRecordBarsClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getAutoRecordBars());
}
void UIParameterAutoRecordBarsClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setAutoRecordBars(value->getInt());
}
UIParameterAutoRecordBarsClass UIParameterAutoRecordBarsObj;
UIParameter* UIParameterAutoRecordBars = &UIParameterAutoRecordBarsObj;

////////////// RecordTransfer

class UIParameterRecordTransferClass : public UIParameter
{
  public:
    UIParameterRecordTransferClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterRecordTransferClass::UIParameterRecordTransferClass()
{
    name = "recordTransfer";
    displayName = "Record Transfer";
    scope = ScopePreset;
    type = TypeInt;
}
void UIParameterRecordTransferClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getRecordTransfer());
}
void UIParameterRecordTransferClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setRecordTransfer((Preset::TransferMode)value->getInt());
}
UIParameterRecordTransferClass UIParameterRecordTransferObj;
UIParameter* UIParameterRecordTransfer = &UIParameterRecordTransferObj;

////////////// OverdubTransfer

class UIParameterOverdubTransferClass : public UIParameter
{
  public:
    UIParameterOverdubTransferClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterOverdubTransferClass::UIParameterOverdubTransferClass()
{
    name = "overdubTransfer";
    displayName = "Overdub Transfer";
    scope = ScopePreset;
    type = TypeInt;
}
void UIParameterOverdubTransferClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getOverdubTransfer());
}
void UIParameterOverdubTransferClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setOverdubTransfer((Preset::TransferMode)value->getInt());
}
UIParameterOverdubTransferClass UIParameterOverdubTransferObj;
UIParameter* UIParameterOverdubTransfer = &UIParameterOverdubTransferObj;

////////////// ReverseTransfer

class UIParameterReverseTransferClass : public UIParameter
{
  public:
    UIParameterReverseTransferClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterReverseTransferClass::UIParameterReverseTransferClass()
{
    name = "reverseTransfer";
    displayName = "Reverse Transfer";
    scope = ScopePreset;
    type = TypeInt;
}
void UIParameterReverseTransferClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getReverseTransfer());
}
void UIParameterReverseTransferClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setReverseTransfer((Preset::TransferMode)value->getInt());
}
UIParameterReverseTransferClass UIParameterReverseTransferObj;
UIParameter* UIParameterReverseTransfer = &UIParameterReverseTransferObj;

////////////// SpeedTransfer

class UIParameterSpeedTransferClass : public UIParameter
{
  public:
    UIParameterSpeedTransferClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterSpeedTransferClass::UIParameterSpeedTransferClass()
{
    name = "speedTransfer";
    displayName = "Speed Transfer";
    scope = ScopePreset;
    type = TypeInt;
}
void UIParameterSpeedTransferClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getSpeedTransfer());
}
void UIParameterSpeedTransferClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setSpeedTransfer((Preset::TransferMode)value->getInt());
}
UIParameterSpeedTransferClass UIParameterSpeedTransferObj;
UIParameter* UIParameterSpeedTransfer = &UIParameterSpeedTransferObj;

////////////// PitchTransfer

class UIParameterPitchTransferClass : public UIParameter
{
  public:
    UIParameterPitchTransferClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterPitchTransferClass::UIParameterPitchTransferClass()
{
    name = "pitchTransfer";
    displayName = "Pitch Transfer";
    scope = ScopePreset;
    type = TypeInt;
}
void UIParameterPitchTransferClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getPitchTransfer());
}
void UIParameterPitchTransferClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setPitchTransfer((Preset::TransferMode)value->getInt());
}
UIParameterPitchTransferClass UIParameterPitchTransferObj;
UIParameter* UIParameterPitchTransfer = &UIParameterPitchTransferObj;

////////////// WindowSlideUnit

class UIParameterWindowSlideUnitClass : public UIParameter
{
  public:
    UIParameterWindowSlideUnitClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterWindowSlideUnitClass::UIParameterWindowSlideUnitClass()
{
    name = "windowSlideUnit";
    displayName = "Window Slide Unit";
    scope = ScopePreset;
    type = TypeInt;
}
void UIParameterWindowSlideUnitClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getWindowSlideUnit());
}
void UIParameterWindowSlideUnitClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setWindowSlideUnit((Preset::WindowUnit)value->getInt());
}
UIParameterWindowSlideUnitClass UIParameterWindowSlideUnitObj;
UIParameter* UIParameterWindowSlideUnit = &UIParameterWindowSlideUnitObj;

////////////// WindowEdgeUnit

class UIParameterWindowEdgeUnitClass : public UIParameter
{
  public:
    UIParameterWindowEdgeUnitClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterWindowEdgeUnitClass::UIParameterWindowEdgeUnitClass()
{
    name = "windowEdgeUnit";
    displayName = "Window Edge Unit";
    scope = ScopePreset;
    type = TypeInt;
}
void UIParameterWindowEdgeUnitClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getWindowEdgeUnit());
}
void UIParameterWindowEdgeUnitClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setWindowEdgeUnit((Preset::WindowUnit)value->getInt());
}
UIParameterWindowEdgeUnitClass UIParameterWindowEdgeUnitObj;
UIParameter* UIParameterWindowEdgeUnit = &UIParameterWindowEdgeUnitObj;

////////////// WindowSlideAmount

class UIParameterWindowSlideAmountClass : public UIParameter
{
  public:
    UIParameterWindowSlideAmountClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterWindowSlideAmountClass::UIParameterWindowSlideAmountClass()
{
    name = "windowSlideAmount";
    displayName = "Window Slide Amount";
    scope = ScopePreset;
    type = TypeInt;
    low = 1;
    high = 128;
}
void UIParameterWindowSlideAmountClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getWindowSlideAmount());
}
void UIParameterWindowSlideAmountClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setWindowSlideAmount(value->getInt());
}
UIParameterWindowSlideAmountClass UIParameterWindowSlideAmountObj;
UIParameter* UIParameterWindowSlideAmount = &UIParameterWindowSlideAmountObj;

////////////// WindowEdgeAmount

class UIParameterWindowEdgeAmountClass : public UIParameter
{
  public:
    UIParameterWindowEdgeAmountClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterWindowEdgeAmountClass::UIParameterWindowEdgeAmountClass()
{
    name = "windowEdgeAmount";
    displayName = "Window Edge Amount";
    scope = ScopePreset;
    type = TypeInt;
    low = 1;
    high = 128;
}
void UIParameterWindowEdgeAmountClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Preset*)obj)->getWindowEdgeAmount());
}
void UIParameterWindowEdgeAmountClass::setValue(void* obj, ExValue* value)
{
    ((Preset*)obj)->setWindowEdgeAmount(value->getInt());
}
UIParameterWindowEdgeAmountClass UIParameterWindowEdgeAmountObj;
UIParameter* UIParameterWindowEdgeAmount = &UIParameterWindowEdgeAmountObj;

//******************** setup


////////////// DefaultSyncSource

class UIParameterDefaultSyncSourceClass : public UIParameter
{
  public:
    UIParameterDefaultSyncSourceClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterDefaultSyncSourceClass::UIParameterDefaultSyncSourceClass()
{
    name = "defaultSyncSource";
    displayName = "Default Sync Source";
    coreName = "syncSource";
    scope = ScopeSetup;
    type = TypeInt;
}
void UIParameterDefaultSyncSourceClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Setup*)obj)->getSyncSource());
}
void UIParameterDefaultSyncSourceClass::setValue(void* obj, ExValue* value)
{
    ((Setup*)obj)->setSyncSource((SyncSource)value->getInt());
}
UIParameterDefaultSyncSourceClass UIParameterDefaultSyncSourceObj;
UIParameter* UIParameterDefaultSyncSource = &UIParameterDefaultSyncSourceObj;

////////////// DefaultTrackSyncUnit

class UIParameterDefaultTrackSyncUnitClass : public UIParameter
{
  public:
    UIParameterDefaultTrackSyncUnitClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterDefaultTrackSyncUnitClass::UIParameterDefaultTrackSyncUnitClass()
{
    name = "defaultTrackSyncUnit";
    displayName = "Default Track Sync Unit";
    scope = ScopeSetup;
    type = TypeInt;
}
void UIParameterDefaultTrackSyncUnitClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Setup*)obj)->getSyncTrackUnit());
}
void UIParameterDefaultTrackSyncUnitClass::setValue(void* obj, ExValue* value)
{
    ((Setup*)obj)->setSyncTrackUnit((SyncTrackUnit)value->getInt());
}
UIParameterDefaultTrackSyncUnitClass UIParameterDefaultTrackSyncUnitObj;
UIParameter* UIParameterDefaultTrackSyncUnit = &UIParameterDefaultTrackSyncUnitObj;

////////////// SlaveSyncUnit

class UIParameterSlaveSyncUnitClass : public UIParameter
{
  public:
    UIParameterSlaveSyncUnitClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterSlaveSyncUnitClass::UIParameterSlaveSyncUnitClass()
{
    name = "slaveSyncUnit";
    displayName = "Slave Sync Unit";
    scope = ScopeSetup;
    type = TypeInt;
}
void UIParameterSlaveSyncUnitClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Setup*)obj)->getSyncUnit());
}
void UIParameterSlaveSyncUnitClass::setValue(void* obj, ExValue* value)
{
    ((Setup*)obj)->setSyncUnit((SyncUnit)value->getInt());
}
UIParameterSlaveSyncUnitClass UIParameterSlaveSyncUnitObj;
UIParameter* UIParameterSlaveSyncUnit = &UIParameterSlaveSyncUnitObj;

////////////// ManualStart

class UIParameterManualStartClass : public UIParameter
{
  public:
    UIParameterManualStartClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterManualStartClass::UIParameterManualStartClass()
{
    name = "manualStart";
    displayName = "Manual Start";
    scope = ScopeSetup;
    type = TypeBool;
}
void UIParameterManualStartClass::getValue(void* obj, ExValue* value)
{
    value->setBool(((Setup*)obj)->isManualStart());
}
void UIParameterManualStartClass::setValue(void* obj, ExValue* value)
{
    ((Setup*)obj)->setManualStart(value->getBool());
}
UIParameterManualStartClass UIParameterManualStartObj;
UIParameter* UIParameterManualStart = &UIParameterManualStartObj;

////////////// MinTempo

class UIParameterMinTempoClass : public UIParameter
{
  public:
    UIParameterMinTempoClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterMinTempoClass::UIParameterMinTempoClass()
{
    name = "minTempo";
    displayName = "Min Tempo";
    scope = ScopeSetup;
    type = TypeInt;
    high = 500;
}
void UIParameterMinTempoClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Setup*)obj)->getMinTempo());
}
void UIParameterMinTempoClass::setValue(void* obj, ExValue* value)
{
    ((Setup*)obj)->setMinTempo(value->getInt());
}
UIParameterMinTempoClass UIParameterMinTempoObj;
UIParameter* UIParameterMinTempo = &UIParameterMinTempoObj;

////////////// MaxTempo

class UIParameterMaxTempoClass : public UIParameter
{
  public:
    UIParameterMaxTempoClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterMaxTempoClass::UIParameterMaxTempoClass()
{
    name = "maxTempo";
    displayName = "Max Tempo";
    scope = ScopeSetup;
    type = TypeInt;
    high = 500;
}
void UIParameterMaxTempoClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Setup*)obj)->getMaxTempo());
}
void UIParameterMaxTempoClass::setValue(void* obj, ExValue* value)
{
    ((Setup*)obj)->setMaxTempo(value->getInt());
}
UIParameterMaxTempoClass UIParameterMaxTempoObj;
UIParameter* UIParameterMaxTempo = &UIParameterMaxTempoObj;

////////////// BeatsPerBar

class UIParameterBeatsPerBarClass : public UIParameter
{
  public:
    UIParameterBeatsPerBarClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterBeatsPerBarClass::UIParameterBeatsPerBarClass()
{
    name = "beatsPerBar";
    displayName = "Beats Per Bar";
    scope = ScopeSetup;
    type = TypeInt;
    high = 64;
}
void UIParameterBeatsPerBarClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Setup*)obj)->getBeatsPerBar());
}
void UIParameterBeatsPerBarClass::setValue(void* obj, ExValue* value)
{
    ((Setup*)obj)->setBeatsPerBar(value->getInt());
}
UIParameterBeatsPerBarClass UIParameterBeatsPerBarObj;
UIParameter* UIParameterBeatsPerBar = &UIParameterBeatsPerBarObj;

////////////// MuteSyncMode

class UIParameterMuteSyncModeClass : public UIParameter
{
  public:
    UIParameterMuteSyncModeClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterMuteSyncModeClass::UIParameterMuteSyncModeClass()
{
    name = "muteSyncMode";
    displayName = "Mute Sync Mode";
    scope = ScopeSetup;
    type = TypeInt;
}
void UIParameterMuteSyncModeClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Setup*)obj)->getMuteSyncMode());
}
void UIParameterMuteSyncModeClass::setValue(void* obj, ExValue* value)
{
    ((Setup*)obj)->setMuteSyncMode((MuteSyncMode)value->getInt());
}
UIParameterMuteSyncModeClass UIParameterMuteSyncModeObj;
UIParameter* UIParameterMuteSyncMode = &UIParameterMuteSyncModeObj;

////////////// ResizeSyncAdjust

class UIParameterResizeSyncAdjustClass : public UIParameter
{
  public:
    UIParameterResizeSyncAdjustClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterResizeSyncAdjustClass::UIParameterResizeSyncAdjustClass()
{
    name = "resizeSyncAdjust";
    displayName = "Resize Sync Adjust";
    scope = ScopeSetup;
    type = TypeInt;
}
void UIParameterResizeSyncAdjustClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Setup*)obj)->getResizeSyncAdjust());
}
void UIParameterResizeSyncAdjustClass::setValue(void* obj, ExValue* value)
{
    ((Setup*)obj)->setResizeSyncAdjust((SyncAdjust)value->getInt());
}
UIParameterResizeSyncAdjustClass UIParameterResizeSyncAdjustObj;
UIParameter* UIParameterResizeSyncAdjust = &UIParameterResizeSyncAdjustObj;

////////////// SpeedSyncAdjust

class UIParameterSpeedSyncAdjustClass : public UIParameter
{
  public:
    UIParameterSpeedSyncAdjustClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterSpeedSyncAdjustClass::UIParameterSpeedSyncAdjustClass()
{
    name = "speedSyncAdjust";
    displayName = "Speed Sync Adjust";
    scope = ScopeSetup;
    type = TypeInt;
}
void UIParameterSpeedSyncAdjustClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Setup*)obj)->getSpeedSyncAdjust());
}
void UIParameterSpeedSyncAdjustClass::setValue(void* obj, ExValue* value)
{
    ((Setup*)obj)->setSpeedSyncAdjust((SyncAdjust)value->getInt());
}
UIParameterSpeedSyncAdjustClass UIParameterSpeedSyncAdjustObj;
UIParameter* UIParameterSpeedSyncAdjust = &UIParameterSpeedSyncAdjustObj;

////////////// RealignTime

class UIParameterRealignTimeClass : public UIParameter
{
  public:
    UIParameterRealignTimeClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterRealignTimeClass::UIParameterRealignTimeClass()
{
    name = "realignTime";
    displayName = "Realign Time";
    scope = ScopeSetup;
    type = TypeInt;
}
void UIParameterRealignTimeClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Setup*)obj)->getRealignTime());
}
void UIParameterRealignTimeClass::setValue(void* obj, ExValue* value)
{
    ((Setup*)obj)->setRealignTime((RealignTime)value->getInt());
}
UIParameterRealignTimeClass UIParameterRealignTimeObj;
UIParameter* UIParameterRealignTime = &UIParameterRealignTimeObj;

////////////// OutRealign

class UIParameterOutRealignClass : public UIParameter
{
  public:
    UIParameterOutRealignClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterOutRealignClass::UIParameterOutRealignClass()
{
    name = "outRealign";
    displayName = "Out Realign";
    scope = ScopeSetup;
    type = TypeInt;
}
void UIParameterOutRealignClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Setup*)obj)->getOutRealignMode());
}
void UIParameterOutRealignClass::setValue(void* obj, ExValue* value)
{
    ((Setup*)obj)->setOutRealignMode((OutRealignMode)value->getInt());
}
UIParameterOutRealignClass UIParameterOutRealignObj;
UIParameter* UIParameterOutRealign = &UIParameterOutRealignObj;

////////////// ActiveTrack

class UIParameterActiveTrackClass : public UIParameter
{
  public:
    UIParameterActiveTrackClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterActiveTrackClass::UIParameterActiveTrackClass()
{
    name = "activeTrack";
    displayName = "Active Track";
    scope = ScopeSetup;
    type = TypeInt;
    high = 8;
    dynamic = true;
    noBinding = true;
}
void UIParameterActiveTrackClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((Setup*)obj)->getActiveTrack());
}
void UIParameterActiveTrackClass::setValue(void* obj, ExValue* value)
{
    ((Setup*)obj)->setActiveTrack(value->getInt());
}
UIParameterActiveTrackClass UIParameterActiveTrackObj;
UIParameter* UIParameterActiveTrack = &UIParameterActiveTrackObj;

//******************** track


////////////// TrackName

class UIParameterTrackNameClass : public UIParameter
{
  public:
    UIParameterTrackNameClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterTrackNameClass::UIParameterTrackNameClass()
{
    name = "trackName";
    displayName = "Track Name";
    scope = ScopeTrack;
    type = TypeString;
    noBinding = true;
}
void UIParameterTrackNameClass::getValue(void* obj, ExValue* value)
{
    value->setString(((SetupTrack*)obj)->getName());
}
void UIParameterTrackNameClass::setValue(void* obj, ExValue* value)
{
    ((SetupTrack*)obj)->setName(value->getString());
}
UIParameterTrackNameClass UIParameterTrackNameObj;
UIParameter* UIParameterTrackName = &UIParameterTrackNameObj;

////////////// Preset

class UIParameterPresetClass : public UIParameter
{
  public:
    UIParameterPresetClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterPresetClass::UIParameterPresetClass()
{
    name = "preset";
    displayName = "Preset";
    scope = ScopeTrack;
    type = TypeString;
    resettable = true;
}
void UIParameterPresetClass::getValue(void* obj, ExValue* value)
{
    value->setString(((SetupTrack*)obj)->getPreset());
}
void UIParameterPresetClass::setValue(void* obj, ExValue* value)
{
    ((SetupTrack*)obj)->setPreset(value->getString());
}
UIParameterPresetClass UIParameterPresetObj;
UIParameter* UIParameterPreset = &UIParameterPresetObj;

////////////// Focus

class UIParameterFocusClass : public UIParameter
{
  public:
    UIParameterFocusClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterFocusClass::UIParameterFocusClass()
{
    name = "focus";
    displayName = "Focus";
    scope = ScopeTrack;
    type = TypeBool;
    noBinding = true;
    resettable = true;
}
void UIParameterFocusClass::getValue(void* obj, ExValue* value)
{
    value->setBool(((SetupTrack*)obj)->isFocusLock());
}
void UIParameterFocusClass::setValue(void* obj, ExValue* value)
{
    ((SetupTrack*)obj)->setFocusLock(value->getBool());
}
UIParameterFocusClass UIParameterFocusObj;
UIParameter* UIParameterFocus = &UIParameterFocusObj;

////////////// Group

class UIParameterGroupClass : public UIParameter
{
  public:
    UIParameterGroupClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterGroupClass::UIParameterGroupClass()
{
    name = "group";
    displayName = "Group";
    scope = ScopeTrack;
    type = TypeInt;
    dynamic = true;
    resettable = true;
}
void UIParameterGroupClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((SetupTrack*)obj)->getGroup());
}
void UIParameterGroupClass::setValue(void* obj, ExValue* value)
{
    ((SetupTrack*)obj)->setGroup(value->getInt());
}
UIParameterGroupClass UIParameterGroupObj;
UIParameter* UIParameterGroup = &UIParameterGroupObj;

////////////// Mono

class UIParameterMonoClass : public UIParameter
{
  public:
    UIParameterMonoClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterMonoClass::UIParameterMonoClass()
{
    name = "mono";
    displayName = "Mono";
    scope = ScopeTrack;
    type = TypeBool;
    noBinding = true;
}
void UIParameterMonoClass::getValue(void* obj, ExValue* value)
{
    value->setBool(((SetupTrack*)obj)->isMono());
}
void UIParameterMonoClass::setValue(void* obj, ExValue* value)
{
    ((SetupTrack*)obj)->setMono(value->getBool());
}
UIParameterMonoClass UIParameterMonoObj;
UIParameter* UIParameterMono = &UIParameterMonoObj;

////////////// Feedback

class UIParameterFeedbackClass : public UIParameter
{
  public:
    UIParameterFeedbackClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterFeedbackClass::UIParameterFeedbackClass()
{
    name = "feedback";
    displayName = "Feedback";
    scope = ScopeTrack;
    type = TypeInt;
    high = 127;
    control = true;
    resettable = true;
}
void UIParameterFeedbackClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((SetupTrack*)obj)->getFeedback());
}
void UIParameterFeedbackClass::setValue(void* obj, ExValue* value)
{
    ((SetupTrack*)obj)->setFeedback(value->getInt());
}
UIParameterFeedbackClass UIParameterFeedbackObj;
UIParameter* UIParameterFeedback = &UIParameterFeedbackObj;

////////////// AltFeedback

class UIParameterAltFeedbackClass : public UIParameter
{
  public:
    UIParameterAltFeedbackClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterAltFeedbackClass::UIParameterAltFeedbackClass()
{
    name = "altFeedback";
    displayName = "Alt Feedback";
    scope = ScopeTrack;
    type = TypeInt;
    high = 127;
    control = true;
    resettable = true;
}
void UIParameterAltFeedbackClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((SetupTrack*)obj)->getAltFeedback());
}
void UIParameterAltFeedbackClass::setValue(void* obj, ExValue* value)
{
    ((SetupTrack*)obj)->setAltFeedback(value->getInt());
}
UIParameterAltFeedbackClass UIParameterAltFeedbackObj;
UIParameter* UIParameterAltFeedback = &UIParameterAltFeedbackObj;

////////////// Input

class UIParameterInputClass : public UIParameter
{
  public:
    UIParameterInputClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterInputClass::UIParameterInputClass()
{
    name = "input";
    displayName = "Input";
    scope = ScopeTrack;
    type = TypeInt;
    high = 127;
    control = true;
    resettable = true;
}
void UIParameterInputClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((SetupTrack*)obj)->getInputLevel());
}
void UIParameterInputClass::setValue(void* obj, ExValue* value)
{
    ((SetupTrack*)obj)->setInputLevel(value->getInt());
}
UIParameterInputClass UIParameterInputObj;
UIParameter* UIParameterInput = &UIParameterInputObj;

////////////// Output

class UIParameterOutputClass : public UIParameter
{
  public:
    UIParameterOutputClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterOutputClass::UIParameterOutputClass()
{
    name = "output";
    displayName = "Output";
    scope = ScopeTrack;
    type = TypeInt;
    high = 127;
    control = true;
    resettable = true;
}
void UIParameterOutputClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((SetupTrack*)obj)->getOutputLevel());
}
void UIParameterOutputClass::setValue(void* obj, ExValue* value)
{
    ((SetupTrack*)obj)->setOutputLevel(value->getInt());
}
UIParameterOutputClass UIParameterOutputObj;
UIParameter* UIParameterOutput = &UIParameterOutputObj;

////////////// Pan

class UIParameterPanClass : public UIParameter
{
  public:
    UIParameterPanClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterPanClass::UIParameterPanClass()
{
    name = "pan";
    displayName = "Pan";
    scope = ScopeTrack;
    type = TypeInt;
    high = 127;
    control = true;
    resettable = true;
}
void UIParameterPanClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((SetupTrack*)obj)->getPan());
}
void UIParameterPanClass::setValue(void* obj, ExValue* value)
{
    ((SetupTrack*)obj)->setPan(value->getInt());
}
UIParameterPanClass UIParameterPanObj;
UIParameter* UIParameterPan = &UIParameterPanObj;

////////////// SyncSource

class UIParameterSyncSourceClass : public UIParameter
{
  public:
    UIParameterSyncSourceClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterSyncSourceClass::UIParameterSyncSourceClass()
{
    name = "syncSource";
    displayName = "Sync Source";
    scope = ScopeTrack;
    type = TypeInt;
}
void UIParameterSyncSourceClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((SetupTrack*)obj)->getSyncSource());
}
void UIParameterSyncSourceClass::setValue(void* obj, ExValue* value)
{
    ((SetupTrack*)obj)->setSyncSource((SyncSource)value->getInt());
}
UIParameterSyncSourceClass UIParameterSyncSourceObj;
UIParameter* UIParameterSyncSource = &UIParameterSyncSourceObj;

////////////// TrackSyncUnit

class UIParameterTrackSyncUnitClass : public UIParameter
{
  public:
    UIParameterTrackSyncUnitClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterTrackSyncUnitClass::UIParameterTrackSyncUnitClass()
{
    name = "trackSyncUnit";
    displayName = "Track Sync Unit";
    scope = ScopeTrack;
    type = TypeInt;
}
void UIParameterTrackSyncUnitClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((SetupTrack*)obj)->getSyncTrackUnit());
}
void UIParameterTrackSyncUnitClass::setValue(void* obj, ExValue* value)
{
    ((SetupTrack*)obj)->setSyncTrackUnit((SyncTrackUnit)value->getInt());
}
UIParameterTrackSyncUnitClass UIParameterTrackSyncUnitObj;
UIParameter* UIParameterTrackSyncUnit = &UIParameterTrackSyncUnitObj;

////////////// AudioInputPort

class UIParameterAudioInputPortClass : public UIParameter
{
  public:
    UIParameterAudioInputPortClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterAudioInputPortClass::UIParameterAudioInputPortClass()
{
    name = "audioInputPort";
    displayName = "Audio Input Port";
    scope = ScopeTrack;
    type = TypeInt;
    low = 1;
    high = 64;
    noBinding = true;
}
void UIParameterAudioInputPortClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((SetupTrack*)obj)->getAudioInputPort());
}
void UIParameterAudioInputPortClass::setValue(void* obj, ExValue* value)
{
    ((SetupTrack*)obj)->setAudioInputPort(value->getInt());
}
UIParameterAudioInputPortClass UIParameterAudioInputPortObj;
UIParameter* UIParameterAudioInputPort = &UIParameterAudioInputPortObj;

////////////// AudioOutputPort

class UIParameterAudioOutputPortClass : public UIParameter
{
  public:
    UIParameterAudioOutputPortClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterAudioOutputPortClass::UIParameterAudioOutputPortClass()
{
    name = "audioOutputPort";
    displayName = "Audio Output Port";
    scope = ScopeTrack;
    type = TypeInt;
    low = 1;
    high = 64;
    noBinding = true;
}
void UIParameterAudioOutputPortClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((SetupTrack*)obj)->getAudioOutputPort());
}
void UIParameterAudioOutputPortClass::setValue(void* obj, ExValue* value)
{
    ((SetupTrack*)obj)->setAudioOutputPort(value->getInt());
}
UIParameterAudioOutputPortClass UIParameterAudioOutputPortObj;
UIParameter* UIParameterAudioOutputPort = &UIParameterAudioOutputPortObj;

////////////// PluginInputPort

class UIParameterPluginInputPortClass : public UIParameter
{
  public:
    UIParameterPluginInputPortClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterPluginInputPortClass::UIParameterPluginInputPortClass()
{
    name = "pluginInputPort";
    displayName = "Plugin Input Port";
    scope = ScopeTrack;
    type = TypeInt;
    low = 1;
    high = 64;
    noBinding = true;
}
void UIParameterPluginInputPortClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((SetupTrack*)obj)->getPluginInputPort());
}
void UIParameterPluginInputPortClass::setValue(void* obj, ExValue* value)
{
    ((SetupTrack*)obj)->setPluginInputPort(value->getInt());
}
UIParameterPluginInputPortClass UIParameterPluginInputPortObj;
UIParameter* UIParameterPluginInputPort = &UIParameterPluginInputPortObj;

////////////// PluginOutputPort

class UIParameterPluginOutputPortClass : public UIParameter
{
  public:
    UIParameterPluginOutputPortClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterPluginOutputPortClass::UIParameterPluginOutputPortClass()
{
    name = "pluginOutputPort";
    displayName = "Plugin Output Port";
    scope = ScopeTrack;
    type = TypeInt;
    low = 1;
    high = 64;
    noBinding = true;
}
void UIParameterPluginOutputPortClass::getValue(void* obj, ExValue* value)
{
    value->setInt(((SetupTrack*)obj)->getPluginOutputPort());
}
void UIParameterPluginOutputPortClass::setValue(void* obj, ExValue* value)
{
    ((SetupTrack*)obj)->setPluginOutputPort(value->getInt());
}
UIParameterPluginOutputPortClass UIParameterPluginOutputPortObj;
UIParameter* UIParameterPluginOutputPort = &UIParameterPluginOutputPortObj;

////////////// SpeedOctave

class UIParameterSpeedOctaveClass : public UIParameter
{
  public:
    UIParameterSpeedOctaveClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterSpeedOctaveClass::UIParameterSpeedOctaveClass()
{
    name = "speedOctave";
    displayName = "Speed Octave";
    scope = ScopeTrack;
    type = TypeInt;
    high = 4;
    zeroCenter = true;
    control = true;
    resettable = true;
    scheduled = true;
    noConfig = true;
}
void UIParameterSpeedOctaveClass::getValue(void* obj, ExValue* value)
{
    value->setNull();
}
void UIParameterSpeedOctaveClass::setValue(void* obj, ExValue* value)
{
}
UIParameterSpeedOctaveClass UIParameterSpeedOctaveObj;
UIParameter* UIParameterSpeedOctave = &UIParameterSpeedOctaveObj;

////////////// SpeedStep

class UIParameterSpeedStepClass : public UIParameter
{
  public:
    UIParameterSpeedStepClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterSpeedStepClass::UIParameterSpeedStepClass()
{
    name = "speedStep";
    displayName = "Speed Step";
    scope = ScopeTrack;
    type = TypeInt;
    high = 48;
    zeroCenter = true;
    control = true;
    resettable = true;
    scheduled = true;
    noConfig = true;
}
void UIParameterSpeedStepClass::getValue(void* obj, ExValue* value)
{
    value->setNull();
}
void UIParameterSpeedStepClass::setValue(void* obj, ExValue* value)
{
}
UIParameterSpeedStepClass UIParameterSpeedStepObj;
UIParameter* UIParameterSpeedStep = &UIParameterSpeedStepObj;

////////////// SpeedBend

class UIParameterSpeedBendClass : public UIParameter
{
  public:
    UIParameterSpeedBendClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterSpeedBendClass::UIParameterSpeedBendClass()
{
    name = "speedBend";
    displayName = "Speed Bend";
    scope = ScopeTrack;
    type = TypeInt;
    high = 8191;
    zeroCenter = true;
    control = true;
    resettable = true;
    scheduled = true;
    noConfig = true;
}
void UIParameterSpeedBendClass::getValue(void* obj, ExValue* value)
{
    value->setNull();
}
void UIParameterSpeedBendClass::setValue(void* obj, ExValue* value)
{
}
UIParameterSpeedBendClass UIParameterSpeedBendObj;
UIParameter* UIParameterSpeedBend = &UIParameterSpeedBendObj;

////////////// PitchOctave

class UIParameterPitchOctaveClass : public UIParameter
{
  public:
    UIParameterPitchOctaveClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterPitchOctaveClass::UIParameterPitchOctaveClass()
{
    name = "pitchOctave";
    displayName = "Pitch Octave";
    scope = ScopeTrack;
    type = TypeInt;
    high = 4;
    zeroCenter = true;
    control = true;
    resettable = true;
    scheduled = true;
    noConfig = true;
}
void UIParameterPitchOctaveClass::getValue(void* obj, ExValue* value)
{
    value->setNull();
}
void UIParameterPitchOctaveClass::setValue(void* obj, ExValue* value)
{
}
UIParameterPitchOctaveClass UIParameterPitchOctaveObj;
UIParameter* UIParameterPitchOctave = &UIParameterPitchOctaveObj;

////////////// PitchStep

class UIParameterPitchStepClass : public UIParameter
{
  public:
    UIParameterPitchStepClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterPitchStepClass::UIParameterPitchStepClass()
{
    name = "pitchStep";
    displayName = "Pitch Step";
    scope = ScopeTrack;
    type = TypeInt;
    high = 48;
    zeroCenter = true;
    control = true;
    resettable = true;
    scheduled = true;
    noConfig = true;
}
void UIParameterPitchStepClass::getValue(void* obj, ExValue* value)
{
    value->setNull();
}
void UIParameterPitchStepClass::setValue(void* obj, ExValue* value)
{
}
UIParameterPitchStepClass UIParameterPitchStepObj;
UIParameter* UIParameterPitchStep = &UIParameterPitchStepObj;

////////////// PitchBend

class UIParameterPitchBendClass : public UIParameter
{
  public:
    UIParameterPitchBendClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterPitchBendClass::UIParameterPitchBendClass()
{
    name = "pitchBend";
    displayName = "Pitch Bend";
    scope = ScopeTrack;
    type = TypeInt;
    high = 8191;
    zeroCenter = true;
    control = true;
    resettable = true;
    scheduled = true;
    noConfig = true;
}
void UIParameterPitchBendClass::getValue(void* obj, ExValue* value)
{
    value->setNull();
}
void UIParameterPitchBendClass::setValue(void* obj, ExValue* value)
{
}
UIParameterPitchBendClass UIParameterPitchBendObj;
UIParameter* UIParameterPitchBend = &UIParameterPitchBendObj;

////////////// TimeStretch

class UIParameterTimeStretchClass : public UIParameter
{
  public:
    UIParameterTimeStretchClass();
    void getValue(void* obj, class ExValue* value) override;
    void setValue(void* obj, class ExValue* value) override;
};
UIParameterTimeStretchClass::UIParameterTimeStretchClass()
{
    name = "timeStretch";
    displayName = "Time Stretch";
    scope = ScopeTrack;
    type = TypeInt;
    high = 8191;
    zeroCenter = true;
    control = true;
    resettable = true;
    scheduled = true;
    noConfig = true;
}
void UIParameterTimeStretchClass::getValue(void* obj, ExValue* value)
{
    value->setNull();
}
void UIParameterTimeStretchClass::setValue(void* obj, ExValue* value)
{
}
UIParameterTimeStretchClass UIParameterTimeStretchObj;
UIParameter* UIParameterTimeStretch = &UIParameterTimeStretchObj;

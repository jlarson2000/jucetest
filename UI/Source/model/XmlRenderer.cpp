/*
 * An XML generator for configuration objects.
 * Formerly this was embedded within each configuration class but
 * I like making model transformations more encapsulated to avoid
 * class clutter and making it more obvious how to do other types
 * of transforms, such as DTOs for the editor.
 */

// I see this a lot for the old getValue(ExValue) methods
// can probably do away with all these and if anyone else needs
// it move this to a general getEnumValue on Parameter
//
//  void MultiplyModeParameterType::getValue(Preset* p, ExValue* value)
//  {
//	  value->setString(values[p->getMultiplyMode()]);
//  }

#define HIDE_SAMPLES 1
#define HIDE_UICONFIG 1
#define HIDE_SAMPLES 1

// necessary to get FILE* for XMlParser.h
#include <stdio.h>
// for atoi
#include <stdlib.h>
// for strcmp
#include <string.h>

#include "../util/Trace.h"
#include "../util/List.h"
#include "../util/XmlModel.h"
#include "../util/XmlBuffer.h"
#include "../util/XomParser.h"
#include "../util/FileUtil.h"

#include "../qtrace.h"

#include "MobiusConfig.h"
#include "Preset.h"
#include "Setup.h"
#include "Binding.h"
#include "Parameter.h"

#include "XmlRenderer.h"

//////////////////////////////////////////////////////////////////////
//
// Public
//
//////////////////////////////////////////////////////////////////////

XmlRenderer::XmlRenderer()
{
}

XmlRenderer::~XmlRenderer()
{
}

char* XmlRenderer::render(Preset* p)
{
	char* xml = nullptr;

    // TODO: make this more like others and have a clear() so it can be reused
    // and not dynamically allocated 
    XmlBuffer* b = new XmlBuffer();
    render(b, p);
    xml = b->stealString();
    delete b;
    return xml;
}

Preset* XmlRenderer::parsePreset(const char* xml)
{
    Preset* preset = nullptr;
	XomParser* parser = new XomParser();
	XmlDocument* doc = parser->parse(xml);

    if (doc == NULL) {
        Trace(1, "XmlRender: Parse error %s\n", parser->getError());
    }
    else {
        XmlElement* e = doc->getChildElement();
        if (e == nullptr) {
            Trace(1, "XmlRender: Missing child element\n");
        }
        else if (!e->isName(EL_PRESET)) {
            Trace(1, "XmlRenderer: Document is not a Preset: %s\n", e->getName());
        }
        else {
            preset = new Preset();
			parse(e, preset);
        }
    }

    delete doc;
	delete parser;

    return preset;
}

//////////////////////////////////////////////////////////////////////
//
// Common 
//
//////////////////////////////////////////////////////////////////////

void XmlRenderer::render(XmlBuffer* b, Parameter* p, int value)
{
    if (p->type == ParameterType::TYPE_ENUM) {
        if (p->values == nullptr) {
            Trace(1, "XmlRenderer: Attempt to render enum parameter without value list %s\n",
                  p->getName());
        }
        else {
            // should do some range checking here but we're only ever getting a value
            // from an object member cast as an int
            b->addAttribute(p->getName(), p->values[value]);
        }
    }
    else {
        // option to filter zero?
        b->addAttribute(p->getName(), value);
    }
}

void XmlRenderer::render(XmlBuffer* b, Parameter* p, bool value)
{
    // old way used ExValue.getString which converted false to "false"
    // and wrote that, XmlBuffer suppresses it
    // continue to supress or included it for clarity?
    if (value)
      b->addAttribute(p->getName(), "true");
    else
      b->addAttribute(p->getName(), "false");
}

void XmlRenderer::render(XmlBuffer* b, Parameter* p, const char* value)
{
    // any filtering options?
    b->addAttribute(p->getName(), value);
}

/**
 * Most parameters are boolean, integer, or enumerations.
 * Parse and return an int which can then be cast by the caller.
 */
int XmlRenderer::parse(XmlElement* e, Parameter* p)
{
    int value = 0;

    const char* str = e->getAttribute(p->getName());
    if (str != nullptr) {
        if (p->type == ParameterType::TYPE_STRING) {
            // error: should not have called this method
            Trace(1, "XmlRenderer: Can't parse string parameter %s as int\n", p->getName());
        }
        else if (p->type == ParameterType::TYPE_BOOLEAN) {
            value = !strcmp(str, "true");
        }
        else if (p->type == ParameterType::TYPE_INT) {
            value = atoi(str);
        }
        else if (p->type == ParameterType::TYPE_ENUM) {
            value = p->getEnumValue(str);
            if (value < 0) {
                // invalid enum name, leave zero
                Trace(1, "XmlRenderer: Invalid enumeration value %s for %s\n", str, p->getName());
            }
        }
    }
    else {
        // there was no attribute
        // note that by returning zero here it will initialize the bool/int/enum
        // to that value rather than selecting a default value or just leaving it alone
        // okay for now since the element is expected to have all attributes
    }

    return value;
}

/**
 * Parse a string attribute.
 * Can return the constant element attribute value, caller is expected
 * to copy it.
 */
const char* XmlRenderer::parseString(XmlElement* e, Parameter* p)
{
    const char* value = nullptr;

    if (p->type == ParameterType::TYPE_STRING) {
        value = e->getAttribute(p->getName());
    }
    else {
        Trace(1, "XmlRenderer: Can't parse parameter %s value as a string\n", p->getName());
    }
    return value;
}

//////////////////////////////////////////////////////////////////////
//
// Bindable
//
//////////////////////////////////////////////////////////////////////

#define ATT_NAME "name"
#define ATT_NUMBER "number"

/**
 * For Bindables, add the name or number.
 */
void XmlRenderer::addBindable(XmlBuffer* b, Bindable* bindable)
{
    // old comments, what does this mean?
    // the number is transient on the way to generating a name, 
	// but just in case we don't have a name, serialize it
    const char* name = bindable->getName();
    if (name != nullptr)
	  b->addAttribute(ATT_NAME, name);
	else
	  b->addAttribute(ATT_NUMBER, bindable->getNumber());
}

void XmlRenderer::parseBindable(XmlElement* e, Bindable* b)
{
	b->setName(e->getAttribute(ATT_NAME));
	b->setNumber(e->getIntAttribute(ATT_NUMBER));
}

//////////////////////////////////////////////////////////////////////
//
// Preset
//
//////////////////////////////////////////////////////////////////////

#define EL_PRESET "Preset"

void XmlRenderer::render(XmlBuffer* b, Preset* p)
{
	b->addOpenStartTag(EL_PRESET);

	// name, number
	addBindable(b, p);
	b->setAttributeNewline(true);

    render(b, AltFeedbackEnableParameter, p->isAltFeedbackEnable());
    render(b, AutoRecordBarsParameter, p->getAutoRecordBars());
    render(b, AutoRecordTempoParameter, p->getAutoRecordTempo());
    render(b, BounceQuantizeParameter, p->getBounceQuantize());
    render(b, EmptyLoopActionParameter, p->getEmptyLoopAction());
    render(b, EmptyTrackActionParameter, p->getEmptyTrackAction());
    render(b, LoopCountParameter, p->getLoops());
    render(b, MaxRedoParameter, p->getMaxRedo());
    render(b, MaxUndoParameter, p->getMaxUndo());
    render(b, MultiplyModeParameter, p->getMultiplyMode());
    render(b, MuteCancelParameter, p->getMuteCancel());
    render(b, MuteModeParameter, p->getMuteMode());
    render(b, NoFeedbackUndoParameter, p->isNoFeedbackUndo());
    render(b, NoLayerFlatteningParameter, p->isNoLayerFlattening());
    render(b, OverdubQuantizedParameter, p->isOverdubQuantized());
    render(b, OverdubTransferParameter, p->getOverdubTransfer());
    render(b, PitchBendRangeParameter, p->getPitchBendRange());
    render(b, PitchSequenceParameter, p->getPitchSequence());
    render(b, PitchShiftRestartParameter, p->isPitchShiftRestart());
    render(b, PitchStepRangeParameter, p->getPitchStepRange());
    render(b, PitchTransferParameter, p->getPitchTransfer());
    render(b, QuantizeParameter, p->getQuantize());
    render(b, SpeedBendRangeParameter, p->getSpeedBendRange());
    render(b, SpeedRecordParameter, p->isSpeedRecord());
    render(b, SpeedSequenceParameter, p->getSpeedSequence());
    render(b, SpeedShiftRestartParameter, p->isSpeedShiftRestart());
    render(b, SpeedStepRangeParameter, p->getSpeedStepRange());
    render(b, SpeedTransferParameter, p->getSpeedTransfer());
    render(b, TimeStretchRangeParameter, p->getTimeStretchRange());
    render(b, RecordResetsFeedbackParameter, p->isRecordResetsFeedback());
    render(b, RecordThresholdParameter, p->getRecordThreshold());
    render(b, RecordTransferParameter, p->getRecordTransfer());
    render(b, ReturnLocationParameter, p->getReturnLocation());
    render(b, ReverseTransferParameter, p->getReverseTransfer());
    render(b, RoundingOverdubParameter, p->isRoundingOverdub());
    render(b, ShuffleModeParameter, p->getShuffleMode());
    render(b, SlipModeParameter, p->getSlipMode());
    render(b, SlipTimeParameter, p->getSlipTime());
    render(b, SoundCopyParameter, p->getSoundCopyMode());
    render(b, SubCycleParameter, p->getSubcycles());
    render(b, SustainFunctionsParameter, p->getSustainFunctions());
    render(b, SwitchDurationParameter, p->getSwitchDuration());
    render(b, SwitchLocationParameter, p->getSwitchLocation());
    render(b, SwitchQuantizeParameter, p->getSwitchQuantize());
    render(b, SwitchVelocityParameter, p->isSwitchVelocity());
    render(b, TimeCopyParameter, p->getTimeCopyMode());
    render(b, TrackLeaveActionParameter, p->getTrackLeaveAction());
    render(b, WindowEdgeAmountParameter, p->getWindowEdgeAmount());
    render(b, WindowEdgeUnitParameter, p->getWindowEdgeUnit());
    render(b, WindowSlideAmountParameter, p->getWindowSlideAmount());
    render(b, WindowSlideUnitParameter, p->getWindowSlideUnit());

	b->add("/>\n");
	b->setAttributeNewline(false);
}

void XmlRenderer::parse(XmlElement* e, Preset* p)
{
	parseBindable(e, p);

    p->setAltFeedbackEnable(parse(e, AltFeedbackEnableParameter));
    p->setAutoRecordBars(parse(e, AutoRecordBarsParameter));
    p->setAutoRecordTempo(parse(e, AutoRecordTempoParameter));
    p->setBounceQuantize(parse(e, BounceQuantizeParameter));
    p->setEmptyLoopAction(parse(e, EmptyLoopActionParameter));
    p->setEmptyTrackAction(parse(e, EmptyTrackActionParameter));
    p->setLoops(parse(e, LoopCountParameter));
    p->setMaxRedo(parse(e, MaxRedoParameter));
    p->setMaxUndo(parse(e, MaxUndoParameter));
    p->setMultiplyMode(parse(e, MultiplyModeParameter));
    p->setMuteCancel(parse(e, MuteCancelParameter));
    p->setMuteMode(parse(e, MuteModeParameter));
    p->setNoFeedbackUndo(parse(e, NoFeedbackUndoParameter));
    p->setNoLayerFlattening(parse(e, NoLayerFlatteningParameter));
    p->setOverdubQuantized(parse(e, OverdubQuantizedParameter));
    p->setOverdubTransfer(parse(e, OverdubTransferParameter));
    p->setPitchBendRange(parse(e, PitchBendRangeParameter));
    p->setPitchSequence(parseString(e, PitchSequenceParameter));
    p->setPitchShiftRestart(parse(e, PitchShiftRestartParameter));
    p->setPitchStepRange(parse(e, PitchStepRangeParameter));
    p->setPitchTransfer(parse(e, PitchTransferParameter));
    p->setQuantize(parse(e, QuantizeParameter));
    p->setSpeedBendRange(parse(e, SpeedBendRangeParameter));
    p->setSpeedRecord(parse(e, SpeedRecordParameter));
    p->setSpeedSequence(parseString(e, SpeedSequenceParameter));
    p->setSpeedShiftRestart(parse(e, SpeedShiftRestartParameter));
    p->setSpeedStepRange(parse(e, SpeedStepRangeParameter));
    p->setSpeedTransfer(parse(e, SpeedTransferParameter));
    p->setTimeStretchRange(parse(e, TimeStretchRangeParameter));
    p->setRecordResetsFeedback(parse(e, RecordResetsFeedbackParameter));
    p->setRecordThreshold(parse(e, RecordThresholdParameter));
    p->setRecordTransfer(parse(e, RecordTransferParameter));
    p->setReturnLocation(parse(e, ReturnLocationParameter));
    p->setReverseTransfer(parse(e, ReverseTransferParameter));
    p->setRoundingOverdub(parse(e, RoundingOverdubParameter));
    p->setShuffleMode(parse(e, ShuffleModeParameter));
    p->setSlipMode(parse(e, SlipModeParameter));
    p->setSlipTime(parse(e, SlipTimeParameter));
    p->setSoundCopyMode(parse(e, SoundCopyParameter));
    p->setSubcycles(parse(e, SubCycleParameter));
    p->setSustainFunctions(parseString(e, SustainFunctionsParameter));
    p->setSwitchDuration(parse(e, SwitchDurationParameter));
    p->setSwitchLocation(parse(e, SwitchLocationParameter));
    p->setSwitchQuantize(parse(e, SwitchQuantizeParameter));
    p->setSwitchVelocity(parse(e, SwitchVelocityParameter));
    p->setTimeCopyMode(parse(e, TimeCopyParameter));
    p->setTrackLeaveAction(parse(e, TrackLeaveActionParameter));
    p->setWindowEdgeAmount(parse(e, WindowEdgeAmountParameter));

    // ugh, I seem to have made redundant setters for all these that take an int
    // rather than an enum, but not these, why?  Kind of like not having the duplication
    p->setWindowEdgeUnit((Preset::WindowUnit)parse(e, WindowEdgeUnitParameter));
    p->setWindowSlideAmount(parse(e, WindowSlideAmountParameter));
    p->setWindowSlideUnit((Preset::WindowUnit)parse(e, WindowSlideUnitParameter));
}

//////////////////////////////////////////////////////////////////////
//
// MobiusConfig
//
//////////////////////////////////////////////////////////////////////

#define EL_CONFIG "MobiusConfig"
#define ATT_LANGUAGE "language"
#define ATT_SETUP "setup"
#define ATT_MIDI_CONFIG "midiConfig"
#define ATT_SUGGESTED_LATENCY "suggestedLatencyMsec"
#define ATT_UI_CONFIG  "uiConfig"
#define ATT_PLUGIN_PINS "pluginPins"
#define ATT_PLUGIN_HOST_REWINDS "pluginHostRewinds"

#define ATT_NO_SYNC_BEAT_ROUNDING "noSyncBeatRounding"

#define ATT_OVERLAY_BINDINGS "overlayBindings"

#define EL_FOCUS_LOCK_FUNCTIONS "FocusLockFunctions"
// old name for FocusLockFunctions
#define EL_GROUP_FUNCTIONS "GroupFunctions"
#define EL_MUTE_CANCEL_FUNCTIONS "MuteCancelFunctions"
#define EL_CONFIRMATION_FUNCTIONS "ConfirmationFunctions"
#define EL_ALT_FEEDBACK_DISABLES "AltFeedbackDisables"
#define EL_STRING "String"

#define EL_SCRIPT_CONFIG "ScriptConfig"
#define EL_SCRIPT_REF "ScripRef"
#define ATT_FILE "file"

#define EL_CONTROL_SURFACE "ControlSurface"
#define ATT_NAME "name"

#define EL_OSC_CONFIG "OscConfig"

#define ATT_LOG_STATUS "logStatus"
#define ATT_EDPISMS "edpisms"

void XmlRenderer::render(XmlBuffer* b, MobiusConfig* c)
{
	// !! this really needs to be table driven like Preset parameters

	b->addOpenStartTag(EL_CONFIG);

    b->addAttribute(ATT_LANGUAGE, c->getLanguage());
    render(b, MidiInputParameter, c->getMidiInput());
    render(b, MidiOutputParameter, c->getMidiOutput());
    render(b, MidiThroughParameter, c->getMidiThrough());
    render(b, PluginMidiInputParameter, c->getPluginMidiInput());
    render(b, PluginMidiOutputParameter, c->getPluginMidiOutput());
    render(b, PluginMidiThroughParameter, c->getPluginMidiThrough());
    render(b, AudioInputParameter, c->getAudioInput());
    render(b, AudioOutputParameter, c->getAudioOutput());
	b->addAttribute(ATT_UI_CONFIG, c->getUIConfig());
    render(b, QuickSaveParameter, c->getQuickSave());
    render(b, CustomMessageFileParameter, c->getCustomMessageFile());
    render(b, UnitTestsParameter, c->getUnitTests());

    render(b, NoiseFloorParameter, c->getNoiseFloor());
	b->addAttribute(ATT_SUGGESTED_LATENCY, c->getSuggestedLatencyMsec());
    render(b, InputLatencyParameter, c->getInputLatency());
    render(b, OutputLatencyParameter, c->getOutputLatency());
    // don't bother saving this until it can have a more useful range
	//render(FadeFramesParameter, c->getFadeFrames());
    render(b, MaxSyncDriftParameter, c->getMaxSyncDrift());
    render(b, TracksParameter, c->getTracks());
    render(b, TrackGroupsParameter, c->getTrackGroups());
    render(b, MaxLoopsParameter, c->getMaxLoops());
    render(b, LongPressParameter, c->getLongPress());
    render(b, MonitorAudioParameter, c->isMonitorAudio());
	b->addAttribute(ATT_PLUGIN_HOST_REWINDS, c->isHostRewinds());
	b->addAttribute(ATT_PLUGIN_PINS, c->getPluginPins());
    render(b, AutoFeedbackReductionParameter, c->isAutoFeedbackReduction());
    // don't allow this to be persisted any more, can only be set in scripts
	//render(IsolateOverdubsParameter->getName(), mIsolateOverdubs);
    render(b, IntegerWaveFileParameter, c->isIntegerWaveFile());
    render(b, SpreadRangeParameter, c->getSpreadRange());
    render(b, TracePrintLevelParameter, c->getTracePrintLevel());
    render(b, TraceDebugLevelParameter, c->getTraceDebugLevel());
    render(b, SaveLayersParameter, c->isSaveLayers());
    render(b, DriftCheckPointParameter, c->getDriftCheckPoint());
    render(b, MidiRecordModeParameter, c->getMidiRecordMode());
    render(b, DualPluginWindowParameter, c->isDualPluginWindow());
    render(b, MidiExportParameter, c->isMidiExport());
    render(b, HostMidiExportParameter, c->isHostMidiExport());
    render(b, GroupFocusLockParameter, c->isGroupFocusLock());

    b->addAttribute(ATT_NO_SYNC_BEAT_ROUNDING, c->isNoSyncBeatRounding());
    b->addAttribute(ATT_LOG_STATUS, c->isLogStatus());

    render(b, OscInputPortParameter, c->getOscInputPort());
    render(b, OscOutputPortParameter, c->getOscOutputPort());
    render(b, OscOutputHostParameter, c->getOscOutputHost());
    render(b, OscTraceParameter, c->isOscTrace());
    render(b, OscEnableParameter, c->isOscEnable());

    render(b, SampleRateParameter, c->getSampleRate());

    // The setup is all we store, if the preset has been overridden
    // this is not saved in the config.
    Setup* setup = c->getCurrentSetup();
	if (setup != nullptr)
	  b->addAttribute(ATT_SETUP, setup->getName());

    BindingConfig* overlay = c->getOverlayBindingConfig();
	if (overlay != nullptr)
	  b->addAttribute(ATT_OVERLAY_BINDINGS, overlay->getName());

    // not an official Parameter yet
    if (c->isEdpisms())
      b->addAttribute(ATT_EDPISMS, "true");

	b->add(">\n");
	b->incIndent();

	if (c->getScriptConfig() != nullptr)
      render(b, c->getScriptConfig());

	for (Preset* p = c->getPresets() ; p != NULL ; p = p->getNext())
	  render(b, p);

	for (Setup* s = c->getSetups() ; s != NULL ; s = s->getNext())
	  render(b, s);

	for (BindingConfig* bc = c->getBindingConfigs() ; bc != NULL ; bc = bc->getNext())
	  render(b, bc);

    // should have cleaned these up by now
    if (mMidiConfigs != NULL) {
        Trace(1, "Still have MidiConfigs!!\n");
        //for (MidiConfig* mc = mMidiConfigs ; mc != NULL ; mc = mc->getNext())
        //mc->toXml(b);
    }

    // never really implemented these
	//for (ControlSurfaceConfig* cs = c->getControlSurfaces() ; cs != NULL ; cs = cs->getNext())
    //render(b, cs);

#ifndef HIDE_SAMPLES
	if (mSamples != NULL)
	  mSamples->toXml(b);
#endif
    
    renderList(b, EL_FOCUS_LOCK_FUNCTIONS, c->getFocusLockFunctions());
    renderList(b, EL_MUTE_CANCEL_FUNCTIONS, c->getMuteCancelFunctions());
    renderList(b, EL_CONFIRMATION_FUNCTIONS, c->getConfirmationFunctions());
    renderList(b, EL_ALT_FEEDBACK_DISABLES, c->getAltFeedbackDisables());

	b->decIndent();

	b->addEndTag(EL_CONFIG);
}

void XmlRenderer::renderList(XmlBuffer* b, const char* elname, StringList* list)
{
	if (list != nullptr && list->size() > 0) {
		b->addStartTag(elname, true);
		b->incIndent();
		for (int i = 0 ; i < list->size() ; i++) {
			const char* name = list->getString(i);
			b->addElement(EL_STRING, name);
		}
		b->decIndent();
		b->addEndTag(elname, true);
	}		
}

void XmlRenderer::parse(XmlElement* e, MobiusConfig* c)
{
    const char* setup = e->getAttribute(ATT_SETUP);
	const char* bconfig = e->getAttribute(ATT_OVERLAY_BINDINGS);

    // save this for upgrade
    // this is part of OldBinding, get rid of this?
    c->setSelectedMidiConfig(e->getAttribute(ATT_MIDI_CONFIG));

	c->setLanguage(e->getAttribute(ATT_LANGUAGE));
	c->setMidiInput(e->getAttribute(MidiInputParameter->getName()));
	c->setMidiOutput(e->getAttribute(MidiOutputParameter->getName()));
	c->setMidiThrough(e->getAttribute(MidiThroughParameter->getName()));
	c->setPluginMidiInput(e->getAttribute(PluginMidiInputParameter->getName()));
	c->setPluginMidiOutput(e->getAttribute(PluginMidiOutputParameter->getName()));
	c->setPluginMidiThrough(e->getAttribute(PluginMidiThroughParameter->getName()));
	c->setAudioInput(e->getAttribute(AudioInputParameter->getName()));
	c->setAudioOutput(e->getAttribute(AudioOutputParameter->getName()));
#ifndef HIDE_UICONFIG
	c->setUIConfig(e->getAttribute(ATT_UI_CONFIG));
#endif    
	c->setQuickSave(e->getAttribute(QuickSaveParameter->getName()));
	c->setUnitTests(e->getAttribute(UnitTestsParameter->getName()));
	c->setCustomMessageFile(e->getAttribute(CustomMessageFileParameter->getName()));

	c->setNoiseFloor(e->getIntAttribute(NoiseFloorParameter->getName()));
	c->setSuggestedLatencyMsec(e->getIntAttribute(ATT_SUGGESTED_LATENCY));
	c->setInputLatency(e->getIntAttribute(InputLatencyParameter->getName()));
	c->setOutputLatency(e->getIntAttribute(OutputLatencyParameter->getName()));
	c->setMaxSyncDrift(e->getIntAttribute(MaxSyncDriftParameter->getName()));
	c->setTracks(e->getIntAttribute(TracksParameter->getName()));
	c->setTrackGroups(e->getIntAttribute(TrackGroupsParameter->getName()));
	c->setMaxLoops(e->getIntAttribute(MaxLoopsParameter->getName()));
	c->setLongPress(e->getIntAttribute(LongPressParameter->getName()));

	c->setMonitorAudio(e->getBoolAttribute(MonitorAudioParameter->getName()));
	c->setHostRewinds(e->getBoolAttribute(ATT_PLUGIN_HOST_REWINDS));
	c->setPluginPins(e->getIntAttribute(ATT_PLUGIN_PINS));
	c->setAutoFeedbackReduction(e->getBoolAttribute(AutoFeedbackReductionParameter->getName()));
    // don't allow this to be persisted any more, can only be set in scripts
	//setIsolateOverdubs(e->getBoolAttribute(IsolateOverdubsParameter->getName()));
	c->setIntegerWaveFile(e->getBoolAttribute(IntegerWaveFileParameter->getName()));
	c->setSpreadRange(e->getIntAttribute(SpreadRangeParameter->getName()));
	c->setTracePrintLevel(e->getIntAttribute(TracePrintLevelParameter->getName()));
	c->setTraceDebugLevel(e->getIntAttribute(TraceDebugLevelParameter->getName()));
	c->setSaveLayers(e->getBoolAttribute(SaveLayersParameter->getName()));
	c->setDriftCheckPoint((DriftCheckPoint)XmlGetEnum(e, DriftCheckPointParameter->getName(), DriftCheckPointParameter->values));
	c->setMidiRecordMode((MidiRecordMode)XmlGetEnum(e, MidiRecordModeParameter->getName(), MidiRecordModeParameter->values));
    c->setDualPluginWindow(e->getBoolAttribute(DualPluginWindowParameter->getName()));
    c->setMidiExport(e->getBoolAttribute(MidiExportParameter->getName()));
    c->setHostMidiExport(e->getBoolAttribute(HostMidiExportParameter->getName()));

    c->setOscInputPort(e->getIntAttribute(OscInputPortParameter->getName()));
    c->setOscOutputPort(e->getIntAttribute(OscOutputPortParameter->getName()));
    c->setOscOutputHost(e->getAttribute(OscOutputHostParameter->getName()));
    c->setOscTrace(e->getBoolAttribute(OscTraceParameter->getName()));
    c->setOscEnable(e->getBoolAttribute(OscEnableParameter->getName()));

    // this isn't a parameter yet
    c->setNoSyncBeatRounding(e->getBoolAttribute(ATT_NO_SYNC_BEAT_ROUNDING));
    c->setLogStatus(e->getBoolAttribute(ATT_LOG_STATUS));

    // not an official parameter yet
    c->setEdpisms(e->getBoolAttribute(ATT_EDPISMS));

	c->setSampleRate((AudioSampleRate)XmlGetEnum(e, SampleRateParameter->getName(), SampleRateParameter->values));

    // fade frames can no longer be set high so we don't bother exposing it
	//setFadeFrames(e->getIntAttribute(FadeFramesParameter->getName()));

	for (XmlElement* child = e->getChildElement() ; child != NULL ; 
		 child = child->getNextElement()) {

		if (child->isName(EL_PRESET)) {
			Preset* p = new Preset();
            parse(child, p);
			c->addPreset(p);
		}
		else if (child->isName(EL_SETUP)) {
			Setup* s = new Setup();
            parse(child, s);
			c->addSetup(s);
		}
		else if (child->isName(EL_BINDING_CONFIG)) {
			BindingConfig* bc = new BindingConfig();
            parse(child, bc);
			c->addBindingConfig(bc);
		}
		else if (child->isName(EL_MIDI_CONFIG)) {
			MidiConfig* mc = new MidiConfig();
            parse(child, mc);
			c->addMidiConfig(c);
		}
		else if (child->isName(EL_SCRIPT_CONFIG)) {
			ScriptConfig* sc = new ScriptConfig();
            parse(child, sc);
            c->setScriptConfig(sc);
		}
		else if (child->isName(EL_CONTROL_SURFACE)) {
			ControlSurfaceConfig* cs = new ControlSurfaceConfig();
            parse(child, cs);
			c->addControlSurface(cs);
		}
#ifndef HIDE_OSC        
		else if (child->isName(EL_OSC_CONFIG)) {
			setOscConfig(new OscConfig(child));
		}
#endif        
#ifndef HIDE_SAMPLES
		else if (child->isName(EL_SAMPLES)) {
			mSamples = new Samples(child);
        }
#endif
		else if (child->isName(EL_FOCUS_LOCK_FUNCTIONS) ||
                 child->isName(EL_GROUP_FUNCTIONS)) {
            // changed the name in 1.43
            c->setFocusLockfunctions(parseStringList(child));
		}
		else if (child->isName(EL_MUTE_CANCEL_FUNCTIONS)) {
            c->setMuteCancelFunctions(parseStringList(child));
		}
		else if (child->isName(EL_CONFIRMATION_FUNCTIONS)) {
            c->setConfirmationFunctions(parseStringList(child));
		}
		else if (child->isName(EL_ALT_FEEDBACK_DISABLES)) {
            c->setAltFeedbackDisables(parseStringList(child));
		}
	}

	// have to wait until these are populated
    // WHY!???
	c->setOverlayBindingConfig(bconfig);
    c->setCurrentSetup(setup);
}

/**
 * Parse a list of <String> eleemnts within a given element.
 * Used mostly in MobiusConfig for function name lists.
 */
StringList* XmlRenderer::parseStringList(XmlElement* e)
{
    StringList* names = new StringList();
    for (XmlElement* child = e->getChildElement() ; 
         child != NULL ; 
         child = child->getNextElement()) {
        // assumed to be <String>xxx</String>
        const char* name = child->getContent();
        if (name != NULL) 
          names->add(name);
    }
    return names;
}

//////////////////////////////////////////////////////////////////////
//
// BindingConfig
//
//////////////////////////////////////////////////////////////////////

/**
 * XML Name for BindingConfig.
 * Public so we can parse it in MobiusConfig.
 */
#define EL_BINDING_CONFIG "BindingConfig"


//////////////////////////////////////////////////////////////////////
//
// Setup
//
//////////////////////////////////////////////////////////////////////

void Setup::toXml(XmlBuffer* b, Setup* setup)
{
	b->addOpenStartTag(EL_SETUP);

	renderBindable(b, setup);

    // these haven't been defined as Parameters, now that we're
    // doing that for the sync options could do these...
    b->addAttribute(ATT_BINDINGS, setup->getBindings());
    b->addAttribute(ATT_ACTIVE, setup->getActive());
    
    StringList* resettables = setup->getResetables();
	if (resetables != NULL) {
		char* csv = resetables->toCsv();
		b->addAttribute(ATT_RESETABLES, csv);
		delete csv;
	}

    // new sync options with Parameter interfaces
	for (int i = 0 ; Parameters[i] != NULL ; i++)  {
        Parameter* p = Parameters[i];
        if (p->scope == PARAM_SCOPE_SETUP && !p->transient)
          p->toXml(b, this);
    }

    render(b, BeatsPerBarParameter, setup->getBeatsPerBar());
    render(b, DefaultSyncSourceParameter, setup->getDefaultSyncSource());
    render(b, DefaultTrackSyncUnitParameter, setup->getDefaultTrackSyncUnit());
    render(b, ManualStartParameter, setup->getManualStart());
    render(b, MaxTempoParameter, setup->getMaxTempo());
    render(b, MinTempoParameter, setup->getMinTempo());
    render(b, MuteSyncModeParameter, setup->getMuteSyncMode());
    render(b, OutRealignModeParameter, setup->getOutRealignMode());
    render(b, RealignTimeParameter, setup->getRealignTime());
    render(b, ResizeSyncAdjustParameter, setup->getResizeSyncAdjust());
    render(b, SlaveSyncUnitParameter, setup->getSlaveSyncUnit());
    render(b, SpeedSyncAdjustParameter, setup->getSpeedSyncAdjust());

    b->add(">\n");
	b->incIndent();

    for (SetupTrack* t = setup->getTracks() ; t != NULL ; t = t->getNext())
	  render(b, t);

	b->decIndent();
	b->addEndTag(EL_SETUP, true);
}

void Setup::parseXml(XmlElement* e, Setup* setup)
{
	parseBindable(e, setup);

	setup->setActiveTrack(e->getIntAttribute(ATT_ACTIVE));

	const char* csv = e->getAttribute(ATT_RESETABLES);
	if (csv != NULL)
	  setup->setResetables(new StringList(csv));
    
    setup->setBeatsPerBar(parse(e, BeatsPerBarParameter));
    setup->setDefaultSyncSource(parse(e, DefaultSyncSourceParameter));
    setup->setDefaultTrackSyncUnit(parse(e, DefaultTrackSyncUnitParameter));
    setup->setManualStart(parse(e, ManualStartParameter));
    setup->setMaxTempo(parse(e, MaxTempoParameter));
    setup->setMinTempo(parse(e, MinTempoParameter));
    setup->setMuteSyncMode(parse(e, MuteSyncModeParameter));
    setup->setOutRealignMode(parse(e, OutRealignModeParameter));
    setup->setRealignTime(parse(e, RealignTimeParameter));
    setup->setResizeSyncAdjust(parse(e, ResizeSyncAdjustParameter));
    setup->setSlaveSyncUnit(parse(e, SlaveSyncUnitParameter));
    setup->setSpeedSyncAdjust(parse(e, SpeedSyncAdjustParameter));

    // recognize the old MidiConfig name, the MidiConfigs will
    // have been upgraded to BindingConfigs by now
    // ?? still need this
    const char* bindings = e->getAttribute(ATT_BINDINGS);
    if (bindings == NULL)
      bindings = e->getAttribute(ATT_MIDI_CONFIG);
	setup->setBindings(bindings);

    SetupTrack* tracks = nullptr;
    SetupTrack* last = nullptr;
    
	for (XmlElement* child = e->getChildElement() ; child != NULL ; 
		 child = child->getNextElement()) {
		SetupTrack* t = new SetupTrack();
        parse(child, t);
		if (last == nullptr)
		  tracks = t;
		else
		  last->setNext(t);
		last = t;
	}
    setup->setTracks(tracks);
}

//////////////////////////////////////////////////////////////////////
//
// Test
//
//////////////////////////////////////////////////////////////////////

void XmlRenderer::test()
{
    Preset* p = new Preset();
    char* xml = render(p);
    Trace(1, "%s\n", xml);

    WriteFile("c:/dev/jucetest/UI/Source/test.xml", xml);

    char* xml2 = ReadFile("c:/dev/jucetest/UI/Source/test.xml");
    if (strcmp(xml, xml2)) {
        Trace(1, "Strings differ\n");
        WriteFile("c:/dev/jucetest/UI/Source/test-err.xml", xml2);
    }

    Preset* p2 = parsePreset(xml2);
    if (p2 != nullptr) {
        char* xml3 = render(p2);
        WriteFile("c:/dev/jucetest/UI/Source/test2.xml", xml3);
        delete xml3;
    }

    delete xml2;
    delete xml;
    delete p;
}

/*
 * An XML generator for configuration objects.
 * Formerly this was embedded within each configuration class but
 * I like making model transformations more encapsulated to avoid
 * class clutter and making it more obvious how to do other types
 * of transforms, such as DTOs for the editor.
 *
 * The main object is MobiusConfig which contains several things
 *
 * MobiusConfig
 *   global paraeters
 *   Preset
 *     preset parameters
 *   Setup
 *     setup parameters, wait are these exposed as parameters?
 *     SetupTrack
 *       track parameters
 *       UserVariables
 *   BindingConfig
 *     Binding
 *   ScriptConfig
 *     ScriptRef
 *   SampleConfig
 *     Sample
 *   OscConfig
 *     things related to OSC
 *
 * This also ControlSurface which was an experiment that never went
 * anywhere.
 *
 * Many things in MobiusConfig are defined as UIParameters whih means
 * they can be accessed in scripts and bindings.  The things that aren't
 * can only be changed in the UI.
 *
 * Another is UIConfig which contains things related to the UI
 * 
 */

// necessary to get FILE* for XMlParser.h
#include <stdio.h>
// for atoi
#include <stdlib.h>
// for strcmp
#include <string.h>

#include "../util/Trace.h"
#include "../util/List.h"
#include "../util/XmlBuffer.h"
#include "../util/XomParser.h"
#include "../util/XmlModel.h"
#include "../util/FileUtil.h"

#include "MobiusConfig.h"
#include "Preset.h"
#include "Setup.h"
#include "UserVariable.h"
#include "Binding.h"
#include "ScriptConfig.h"
#include "SampleConfig.h"
#include "OscConfig.h"
#include "UIConfig.h"
#include "UIParameter.h"

#include "XmlRenderer.h"

//////////////////////////////////////////////////////////////////////
//
// Public
//
//////////////////////////////////////////////////////////////////////

#define EL_MOBIUS_CONFIG "MobiusConfig"
#define EL_UI_CONFIG "UIConfig"
#define EL_PRESET "Preset"

XmlRenderer::XmlRenderer()
{
}

XmlRenderer::~XmlRenderer()
{
}

//////////////////////////////////////////////////////////////////////
//
// Object renderers and cloners
//
// Really shouldn't need clone methods if we made all the objects
// copyable.
//
//////////////////////////////////////////////////////////////////////

char* XmlRenderer::render(MobiusConfig* c)
{
	char* xml = nullptr;
    XmlBuffer b;

    render(&b, c);
    xml = b.stealString();
    return xml;
}

MobiusConfig* XmlRenderer::parseMobiusConfig(const char* xml)
{
    MobiusConfig* config = nullptr;
	XomParser* parser = new XomParser();
    XmlDocument* doc = parser->parse(xml);
	
    if (doc == nullptr) {
        Trace(1, "XmlRender: Parse error %s\n", parser->getError());
    }
    else {
        XmlElement* e = doc->getChildElement();
        if (e == nullptr) {
            Trace(1, "XmlRender: Missing child element\n");
        }
        else if (!e->isName(EL_MOBIUS_CONFIG)) {
            Trace(1, "XmlRenderer: Document is not a MobiusConfig: %s\n", e->getName());
        }
        else {
            config = new MobiusConfig();
			parse(e, config);
        }
    }

    delete doc;
	delete parser;

    return config;
}

char* XmlRenderer::render(UIConfig* c)
{
	char* xml = nullptr;
    XmlBuffer b;

    render(&b, c);
    xml = b.stealString();
    return xml;
}

UIConfig* XmlRenderer::parseUIConfig(const char* xml)
{
    UIConfig* config = nullptr;
	XomParser* parser = new XomParser();
    XmlDocument* doc = parser->parse(xml);
	
    if (doc == nullptr) {
        Trace(1, "XmlRender: Parse error %s\n", parser->getError());
    }
    else {
        XmlElement* e = doc->getChildElement();
        if (e == nullptr) {
            Trace(1, "XmlRender: Missing child element\n");
        }
        else if (!e->isName(EL_UI_CONFIG)) {
            Trace(1, "XmlRenderer: Document is not a UIConfig: %s\n", e->getName());
        }
        else {
            config = new UIConfig();
			parse(e, config);
        }
    }

    delete doc;
	delete parser;

    return config;
}

/**
 * Don't need XML based cloners any more but could use this
 * to verify that the class cloners work properly
 */
#if 0
Preset* XmlRenderer::clone(Preset* src)
{
    Preset* copy = nullptr;

    XmlBuffer b;
    render(&b, src);

	XomParser parser;
    // nicer if the parser owns the document so it can be
    // deleted with it, when would we not want that
	XmlDocument* doc = parser.parse(b.getString());
    if (doc != nullptr) {
        XmlElement* e = doc->getChildElement();
        if (e != nullptr) {
            copy = new Preset();
            parse(e, copy);
        }
        delete doc;
    }
    
    return copy;
}

Setup* XmlRenderer::clone(Setup* src)
{
    Setup* copy = nullptr;

    XmlBuffer b;
    render(&b, src);

	XomParser parser;
    // nicer if the parser owns the document so it can be
    // deleted with it, when would we not want that
	XmlDocument* doc = parser.parse(b.getString());
    if (doc != nullptr) {
        XmlElement* e = doc->getChildElement();
        if (e != nullptr) {
            copy = new Setup();
            parse(e, copy);
        }
        delete doc;
    }
    
    return copy;
}
#endif

/**
 * Really hating this repetition, figure out a way to share this
 */
MobiusConfig* XmlRenderer::clone(MobiusConfig* src)
{
    MobiusConfig* copy = nullptr;

    XmlBuffer b;
    render(&b, src);

	XomParser parser;
    // nicer if the parser owns the document so it can be
    // deleted with it, when would we not want that
	XmlDocument* doc = parser.parse(b.getString());
    if (doc != nullptr) {
        XmlElement* e = doc->getChildElement();
        if (e != nullptr) {
            copy = new MobiusConfig();
            parse(e, copy);
        }
        delete doc;
    }
    
    return copy;
}

//////////////////////////////////////////////////////////////////////
//
// Common Utilities
//
//////////////////////////////////////////////////////////////////////

#define EL_STRING "String"

void XmlRenderer::render(XmlBuffer* b, UIParameter* p, int value)
{
    if (p->type == UIParameterType::TypeEnum) {
        if (p->values == nullptr) {
            Trace(1, "XmlRenderer: Attempt to render enum parameter without value list %s\n",
                  p->getName());
        }
        else {
            // should do some range checking here but we're only ever getting a value
            // from an object member cast as an int
            // shoudl be letting the 
            // !! put this in UIParameter::getEnumValue
            b->addAttribute(p->getName(), p->getEnumName(value));
        }
    }
    else {
        // option to filter zero?
        // yes, lots of things are zero/false
        if (value > 0)
          b->addAttribute(p->getName(), value);
    }
}

void XmlRenderer::render(XmlBuffer* b, UIParameter* p, bool value)
{
    // old way used ExValue.getString which converted false to "false"
    // and wrote that, XmlBuffer suppresses it
    // continue to supress or included it for clarity?
    if (value)
      b->addAttribute(p->getName(), "true");
    //else
    //b->addAttribute(p->getName(), "false");
}

void XmlRenderer::render(XmlBuffer* b, UIParameter* p, const char* value)
{
    // any filtering options?
    if (value != nullptr)
      b->addAttribute(p->getName(), value);
}

/**
 * Most parameters are boolean, integer, or enumerations.
 * Parse and return an int which can then be cast by the caller.
 */
int XmlRenderer::parse(XmlElement* e, UIParameter* p)
{
    int value = 0;

    const char* str = e->getAttribute(p->getName());
    if (str != nullptr) {
        if (p->type == UIParameterType::TypeString) {
            // error: should not have called this method
            Trace(1, "XmlRenderer: Can't parse string parameter %s as int\n", p->getName());
        }
        else if (p->type == UIParameterType::TypeBool) {
            value = !strcmp(str, "true");
        }
        else if (p->type == UIParameterType::TypeInt) {
            value = atoi(str);
        }
        else if (p->type == UIParameterType::TypeEnum) {
            value = p->getEnumOrdinal(str);
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
const char* XmlRenderer::parseString(XmlElement* e, UIParameter* p)
{
    const char* value = nullptr;

    if (p->type == UIParameterType::TypeString) {
        value = e->getAttribute(p->getName());
    }
    else {
        Trace(1, "XmlRenderer: Can't parse parameter %s value as a string\n", p->getName());
    }
    return value;
}

/**
 * Parse a list of <String> eleemnts within a given element.
 * Used mostly in MobiusConfig for function name lists.
 * TODO: I'm leaning toward CSVs for these
 */
StringList* XmlRenderer::parseStringList(XmlElement* e)
{
    StringList* names = new StringList();
    for (XmlElement* child = e->getChildElement() ; 
         child != nullptr ; 
         child = child->getNextElement()) {
        // assumed to be <String>xxx</String>
        const char* name = child->getContent();
        if (name != nullptr) 
          names->add(name);
    }
    return names;
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

//////////////////////////////////////////////////////////////////////
//
// Structure (formerly Bindable)
//
//////////////////////////////////////////////////////////////////////

#define ATT_NAME "name"
#define ATT_ORDINAL "ordinal"

/**
 * For Bindables, add the name.
 * The ordinal is runtime only but old comments say to include
 * it if the name is not set.  Can't think of the circumstances
 * where that would be necessary.
 */
void XmlRenderer::renderStructure(XmlBuffer* b, Structure* structure)
{
    const char* name = structure->getName();
    if (name != nullptr)
	  b->addAttribute(ATT_NAME, name);
	else
	  b->addAttribute(ATT_ORDINAL, structure->ordinal);
}

void XmlRenderer::parseStructure(XmlElement* e, Structure* structure)
{
	structure->setName(e->getAttribute(ATT_NAME));
    if (structure->getName() == nullptr)
      structure->ordinal = e->getIntAttribute(ATT_ORDINAL);
}

//////////////////////////////////////////////////////////////////////
//
// MobiusConfig
//
//////////////////////////////////////////////////////////////////////

#define EL_MOBIUS_CONFIG "MobiusConfig"
#define EL_PRESET "Preset"
#define EL_SETUP "Setup"

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

// old name
#define EL_BINDING_CONFIG "BindingConfig"
// new name
#define EL_BINDING_SET "BindingSet"

#define EL_SCRIPT_CONFIG "ScriptConfig"
#define EL_SCRIPT_REF "ScripRef"
#define ATT_FILE "file"

// old name was just "Samples" not bothering with an upgrade
#define EL_SAMPLE_CONFIG "SampleConfig"

#define EL_CONTROL_SURFACE "ControlSurface"
#define ATT_NAME "name"

#define EL_OSC_CONFIG "OscConfig"

#define ATT_LOG_STATUS "logStatus"
#define ATT_EDPISMS "edpisms"

void XmlRenderer::render(XmlBuffer* b, MobiusConfig* c)
{
	b->addOpenStartTag(EL_MOBIUS_CONFIG);
	b->setAttributeNewline(true);

    render(b, UIParameterMidiInput, c->getMidiInput());
    render(b, UIParameterMidiOutput, c->getMidiOutput());
    render(b, UIParameterMidiThrough, c->getMidiThrough());
    render(b, UIParameterPluginMidiInput, c->getPluginMidiInput());
    render(b, UIParameterPluginMidiOutput, c->getPluginMidiOutput());
    render(b, UIParameterPluginMidiThrough, c->getPluginMidiThrough());
    render(b, UIParameterAudioInput, c->getAudioInput());
    render(b, UIParameterAudioOutput, c->getAudioOutput());

    render(b, UIParameterQuickSave, c->getQuickSave());
    //render(b, UIParameterCustomMessageFile, c->getCustomMessageFile());
    //render(b, UIParameterUnitTests, c->getUnitTests());

    render(b, UIParameterNoiseFloor, c->getNoiseFloor());

	b->addAttribute(ATT_SUGGESTED_LATENCY, c->getSuggestedLatencyMsec());
    render(b, UIParameterInputLatency, c->getInputLatency());
    render(b, UIParameterOutputLatency, c->getOutputLatency());
    // don't bother saving this until it can have a more useful range
	//render(UIParameterFadeFrames, c->getFadeFrames());
    render(b, UIParameterMaxSyncDrift, c->getMaxSyncDrift());
    render(b, UIParameterTrackCount, c->getTracks());
    render(b, UIParameterGroupCount, c->getTrackGroups());
    render(b, UIParameterMaxLoops, c->getMaxLoops());
    render(b, UIParameterLongPress, c->getLongPress());
    render(b, UIParameterMonitorAudio, c->isMonitorAudio());
	b->addAttribute(ATT_PLUGIN_HOST_REWINDS, c->isHostRewinds());
	b->addAttribute(ATT_PLUGIN_PINS, c->getPluginPins());
    render(b, UIParameterAutoFeedbackReduction, c->isAutoFeedbackReduction());
    // don't allow this to be persisted any more, can only be set in scripts
	//render(IsolateOverdubsParameter->getName(), mIsolateOverdubs);
    render(b, UIParameterIntegerWaveFile, c->isIntegerWaveFile());
    render(b, UIParameterSpreadRange, c->getSpreadRange());
    render(b, UIParameterTraceLevel, c->getTraceDebugLevel());
    render(b, UIParameterSaveLayers, c->isSaveLayers());
    render(b, UIParameterDriftCheckPoint, c->getDriftCheckPoint());

    render(b, UIParameterMidiRecordMode, c->getMidiRecordMode());
    //render(b, UIParameterDualPluginWindow, c->isDualPluginWindow());
    //render(b, UIParameterMidiExport, c->isMidiExport());
    //render(b, UIParameterHostMidiExport, c->isHostMidiExport());
    render(b, UIParameterGroupFocusLock, c->isGroupFocusLock());

    b->addAttribute(ATT_NO_SYNC_BEAT_ROUNDING, c->isNoSyncBeatRounding());
    b->addAttribute(ATT_LOG_STATUS, c->isLogStatus());

    // why is this here?  move to OscConfig
    // also doesn't need to be a bindable Parameter
    //render(b, UIParameterOscInputPort, c->getOscInputPort());
    //render(b, UIParameterOscOutputPort, c->getOscOutputPort());
    //render(b, UIParameterOscOutputHost, c->getOscOutputHost());
    //render(b, UIParameterOscTrace, c->isOscTrace());
    //render(b, UIParameterOscEnable, c->isOscEnable());

    //render(b, UIParameterSampleRate, c->getSampleRate());

    // active setup name
    // old notes say if the preset has been overridden this is not
    // saved in the config
    b->addAttribute(ATT_SETUP, c->getStartingSetupName());

    // active binding overlay name
    b->addAttribute(ATT_OVERLAY_BINDINGS, c->getOverlayBindings());

    // not an official Parameter yet
    if (c->isEdpisms())
      b->addAttribute(ATT_EDPISMS, "true");

	b->add(">\n");
	b->incIndent();

	for (Preset* p = c->getPresets() ; p != nullptr ; p = (Preset*)(p->getNext()))
	  render(b, p);

	for (Setup* s = c->getSetups() ; s != nullptr ; s = (Setup*)(s->getNext()))
	  render(b, s);

	for (BindingSet* bs = c->getBindingSets() ; bs != nullptr ; bs = (BindingSet*)(bs->getNext()))
	  render(b, bs);

	if (c->getScriptConfig() != nullptr)
      render(b, c->getScriptConfig());

	if (c->getSampleConfig() != nullptr)
      render(b, c->getSampleConfig());

    if (c->getOscConfig() != nullptr)
      render(b, c->getOscConfig());

#if 0
    // never really implemented these
	for (ControlSurfaceConfig* cs = c->getControlSurfaces() ; cs != nullptr ; cs = cs->getNext())
      render(b, cs);
#endif

    // though they are top-level parameters, put these last since
    // they are long and not as interesting as the main child objects
    // TODO: just use csv like SustainFunctions
    renderList(b, EL_FOCUS_LOCK_FUNCTIONS, c->getFocusLockFunctions());
    renderList(b, EL_MUTE_CANCEL_FUNCTIONS, c->getMuteCancelFunctions());
    renderList(b, EL_CONFIRMATION_FUNCTIONS, c->getConfirmationFunctions());
    renderList(b, EL_ALT_FEEDBACK_DISABLES, c->getAltFeedbackDisables());

	b->decIndent();
	b->setAttributeNewline(false);

	b->addEndTag(EL_MOBIUS_CONFIG);
}

void XmlRenderer::parse(XmlElement* e, MobiusConfig* c)
{
    // save this for upgrade
    // this is part of OldBinding, get rid of this?
    // c->setSelectedMidiConfig(e->getAttribute(ATT_MIDI_CONFIG));
    
	c->setMidiInput(parseString(e, UIParameterMidiInput));
	c->setMidiOutput(parseString(e, UIParameterMidiOutput));
	c->setMidiThrough(parseString(e, UIParameterMidiThrough));
	c->setPluginMidiInput(parseString(e, UIParameterPluginMidiInput));
	c->setPluginMidiOutput(parseString(e, UIParameterPluginMidiOutput));
	c->setPluginMidiThrough(parseString(e, UIParameterPluginMidiThrough));
	c->setAudioInput(parseString(e, UIParameterAudioInput));
	c->setAudioOutput(parseString(e, UIParameterAudioOutput));
	c->setUIConfig(e->getAttribute(ATT_UI_CONFIG));
	c->setQuickSave(parseString(e, UIParameterQuickSave));
	//c->setUnitTests(parseString(e, UIParameterUnitTests));
	//c->setCustomMessageFile(parseString(e, UIParameterCustomMessageFile));

	c->setNoiseFloor(parse(e, UIParameterNoiseFloor));
	c->setSuggestedLatencyMsec(e->getIntAttribute(ATT_SUGGESTED_LATENCY));
	c->setInputLatency(parse(e, UIParameterInputLatency));
	c->setOutputLatency(parse(e, UIParameterOutputLatency));
	c->setMaxSyncDrift(parse(e, UIParameterMaxSyncDrift));
	c->setTracks(parse(e, UIParameterTrackCount));
	c->setTrackGroups(parse(e, UIParameterGroupCount));
	c->setMaxLoops(parse(e, UIParameterMaxLoops));
	c->setLongPress(parse(e, UIParameterLongPress));

	c->setMonitorAudio(parse(e, UIParameterMonitorAudio));
	c->setHostRewinds(e->getBoolAttribute(ATT_PLUGIN_HOST_REWINDS));
	c->setPluginPins(e->getIntAttribute(ATT_PLUGIN_PINS));
	c->setAutoFeedbackReduction(parse(e, UIParameterAutoFeedbackReduction));

    // don't allow this to be persisted any more, can only be set in scripts
	//setIsolateOverdubs(e->getBoolAttribute(IsolateOverdubsParameter->getName()));
	c->setIntegerWaveFile(parse(e, UIParameterIntegerWaveFile));
	c->setSpreadRange(parse(e, UIParameterSpreadRange));
	//c->setTracePrintLevel(parse(e, UIParameterTracePrintLevel));
	c->setTraceDebugLevel(parse(e, UIParameterTraceLevel));
	c->setSaveLayers(parse(e, UIParameterSaveLayers));
	c->setDriftCheckPoint((DriftCheckPoint)parse(e, UIParameterDriftCheckPoint));
	c->setMidiRecordMode((MidiRecordMode)parse(e, UIParameterMidiRecordMode));
    //c->setDualPluginWindow(parse(e, UIParameterDualPluginWindow));
    //c->setMidiExport(parse(e, UIParameterMidiExport));
    //c->setHostMidiExport(parse(e, UIParameterHostMidiExport));

    //c->setOscInputPort(parse(e, UIParameterOscInputPort));
    //c->setOscOutputPort(parse(e, UIParameterOscOutputPort));
    //c->setOscOutputHost(parseString(e, UIParameterOscOutputHost));
    //c->setOscTrace(parse(e, UIParameterOscTrace));
    //c->setOscEnable(parse(e, UIParameterOscEnable));

    // this isn't a parameter yet
    c->setNoSyncBeatRounding(e->getBoolAttribute(ATT_NO_SYNC_BEAT_ROUNDING));
    c->setLogStatus(e->getBoolAttribute(ATT_LOG_STATUS));

    // not an official parameter yet
    c->setEdpisms(e->getBoolAttribute(ATT_EDPISMS));

	//c->setSampleRate((AudioSampleRate)parse(e, UIParameterSampleRate));

    // fade frames can no longer be set high so we don't bother exposing it
	//setFadeFrames(e->getIntAttribute(FadeFramesParameter->getName()));

	for (XmlElement* child = e->getChildElement() ; child != nullptr ; 
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
		else if (child->isName(EL_BINDING_CONFIG) ||
                 child->isName(EL_BINDING_SET)) {
			BindingSet* bs = new BindingSet();
            parse(child, bs);
			c->addBindingSet(bs);
		}
		else if (child->isName(EL_SCRIPT_CONFIG)) {
			ScriptConfig* sc = new ScriptConfig();
            parse(child, sc);
            c->setScriptConfig(sc);
		}
		else if (child->isName(EL_SAMPLE_CONFIG)) {
			SampleConfig* sc = new SampleConfig();
            parse(child, sc);
            c->setSampleConfig(sc);
		}

        // never did fully support this 
		//else if (child->isName(EL_CONTROL_SURFACE)) {
        //ControlSurfaceConfig* cs = new ControlSurfaceConfig();
        //parse(child, cs);
        //c->addControlSurface(cs);
        //}

		else if (child->isName(EL_OSC_CONFIG)) {
			OscConfig* oc = new OscConfig();
            parse(child, oc);
			c->setOscConfig(oc);
		}

		else if (child->isName(EL_FOCUS_LOCK_FUNCTIONS) ||
                 child->isName(EL_GROUP_FUNCTIONS)) {
            // changed the name in 1.43
            c->setFocusLockFunctions(parseStringList(child));
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
        else {
            trace("Unknnown element %s\n", child->getName());
        }
	}

    // formerly had to do these last after the object lists
    // were built, now they're just names
    c->setStartingSetupName(e->getAttribute(ATT_SETUP));
    c->setOverlayBindings(e->getAttribute(ATT_OVERLAY_BINDINGS));
}

//////////////////////////////////////////////////////////////////////
//
// Preset
//
//////////////////////////////////////////////////////////////////////

void XmlRenderer::render(XmlBuffer* b, Preset* p)
{
	b->addOpenStartTag(EL_PRESET);
	b->setAttributeNewline(true);

	// name, number
	renderStructure(b, p);

    render(b, UIParameterAltFeedbackEnable, p->isAltFeedbackEnable());
    render(b, UIParameterAutoRecordBars, p->getAutoRecordBars());
    render(b, UIParameterAutoRecordTempo, p->getAutoRecordTempo());
    render(b, UIParameterBounceQuantize, p->getBounceQuantize());
    render(b, UIParameterEmptyLoopAction, p->getEmptyLoopAction());
    render(b, UIParameterEmptyTrackAction, p->getEmptyTrackAction());
    render(b, UIParameterLoopCount, p->getLoops());
    render(b, UIParameterMaxRedo, p->getMaxRedo());
    render(b, UIParameterMaxUndo, p->getMaxUndo());
    render(b, UIParameterMultiplyMode, p->getMultiplyMode());
    render(b, UIParameterMuteCancel, p->getMuteCancel());
    render(b, UIParameterMuteMode, p->getMuteMode());
    render(b, UIParameterNoFeedbackUndo, p->isNoFeedbackUndo());
    render(b, UIParameterNoLayerFlattening, p->isNoLayerFlattening());
    render(b, UIParameterOverdubQuantized, p->isOverdubQuantized());
    render(b, UIParameterOverdubTransfer, p->getOverdubTransfer());
    render(b, UIParameterPitchBendRange, p->getPitchBendRange());
    //render(b, UIParameterPitchSequence, p->getPitchSequence());
    render(b, UIParameterPitchShiftRestart, p->isPitchShiftRestart());
    render(b, UIParameterPitchStepRange, p->getPitchStepRange());
    render(b, UIParameterPitchTransfer, p->getPitchTransfer());
    render(b, UIParameterQuantize, p->getQuantize());
    render(b, UIParameterSpeedBendRange, p->getSpeedBendRange());
    render(b, UIParameterSpeedRecord, p->isSpeedRecord());
    //render(b, UIParameterSpeedSequence, p->getSpeedSequence());
    render(b, UIParameterSpeedShiftRestart, p->isSpeedShiftRestart());
    render(b, UIParameterSpeedStepRange, p->getSpeedStepRange());
    render(b, UIParameterSpeedTransfer, p->getSpeedTransfer());
    render(b, UIParameterTimeStretchRange, p->getTimeStretchRange());
    render(b, UIParameterRecordResetsFeedback, p->isRecordResetsFeedback());
    render(b, UIParameterRecordThreshold, p->getRecordThreshold());
    render(b, UIParameterRecordTransfer, p->getRecordTransfer());
    render(b, UIParameterReturnLocation, p->getReturnLocation());
    render(b, UIParameterReverseTransfer, p->getReverseTransfer());
    render(b, UIParameterRoundingOverdub, p->isRoundingOverdub());
    render(b, UIParameterShuffleMode, p->getShuffleMode());
    render(b, UIParameterSlipMode, p->getSlipMode());
    render(b, UIParameterSlipTime, p->getSlipTime());
    render(b, UIParameterSoundCopyMode, p->getSoundCopyMode());
    render(b, UIParameterSubcycles, p->getSubcycles());
    //render(b, UIParameterSustainFunctions, p->getSustainFunctions());
    render(b, UIParameterSwitchDuration, p->getSwitchDuration());
    render(b, UIParameterSwitchLocation, p->getSwitchLocation());
    render(b, UIParameterSwitchQuantize, p->getSwitchQuantize());
    render(b, UIParameterSwitchVelocity, p->isSwitchVelocity());
    render(b, UIParameterTimeCopyMode, p->getTimeCopyMode());
    render(b, UIParameterTrackLeaveAction, p->getTrackLeaveAction());
    render(b, UIParameterWindowEdgeAmount, p->getWindowEdgeAmount());
    render(b, UIParameterWindowEdgeUnit, p->getWindowEdgeUnit());
    render(b, UIParameterWindowSlideAmount, p->getWindowSlideAmount());
    render(b, UIParameterWindowSlideUnit, p->getWindowSlideUnit());

	b->add("/>\n");
	b->setAttributeNewline(false);
}

void XmlRenderer::parse(XmlElement* e, Preset* p)
{
	parseStructure(e, p);

    p->setAltFeedbackEnable(parse(e, UIParameterAltFeedbackEnable));
    p->setAutoRecordBars(parse(e, UIParameterAutoRecordBars));
    p->setAutoRecordTempo(parse(e, UIParameterAutoRecordTempo));
    p->setBounceQuantize(parse(e, UIParameterBounceQuantize));
    p->setEmptyLoopAction(parse(e, UIParameterEmptyLoopAction));
    p->setEmptyTrackAction(parse(e, UIParameterEmptyTrackAction));
    p->setLoops(parse(e, UIParameterLoopCount));
    p->setMaxRedo(parse(e, UIParameterMaxRedo));
    p->setMaxUndo(parse(e, UIParameterMaxUndo));
    p->setMultiplyMode(parse(e, UIParameterMultiplyMode));
    p->setMuteCancel(parse(e, UIParameterMuteCancel));
    p->setMuteMode(parse(e, UIParameterMuteMode));
    p->setNoFeedbackUndo(parse(e, UIParameterNoFeedbackUndo));
    p->setNoLayerFlattening(parse(e, UIParameterNoLayerFlattening));
    p->setOverdubQuantized(parse(e, UIParameterOverdubQuantized));
    p->setOverdubTransfer(parse(e, UIParameterOverdubTransfer));
    p->setPitchBendRange(parse(e, UIParameterPitchBendRange));
    //p->setPitchSequence(parseString(e, UIParameterPitchSequence));
    p->setPitchShiftRestart(parse(e, UIParameterPitchShiftRestart));
    p->setPitchStepRange(parse(e, UIParameterPitchStepRange));
    p->setPitchTransfer(parse(e, UIParameterPitchTransfer));
    p->setQuantize(parse(e, UIParameterQuantize));
    p->setSpeedBendRange(parse(e, UIParameterSpeedBendRange));
    p->setSpeedRecord(parse(e, UIParameterSpeedRecord));
    //p->setSpeedSequence(parseString(e, UIParameterSpeedSequence));
    p->setSpeedShiftRestart(parse(e, UIParameterSpeedShiftRestart));
    p->setSpeedStepRange(parse(e, UIParameterSpeedStepRange));
    p->setSpeedTransfer(parse(e, UIParameterSpeedTransfer));
    p->setTimeStretchRange(parse(e, UIParameterTimeStretchRange));
    p->setRecordResetsFeedback(parse(e, UIParameterRecordResetsFeedback));
    p->setRecordThreshold(parse(e, UIParameterRecordThreshold));
    p->setRecordTransfer(parse(e, UIParameterRecordTransfer));
    p->setReturnLocation(parse(e, UIParameterReturnLocation));
    p->setReverseTransfer(parse(e, UIParameterReverseTransfer));
    p->setRoundingOverdub(parse(e, UIParameterRoundingOverdub));
    p->setShuffleMode(parse(e, UIParameterShuffleMode));
    p->setSlipMode(parse(e, UIParameterSlipMode));
    p->setSlipTime(parse(e, UIParameterSlipTime));
    p->setSoundCopyMode(parse(e, UIParameterSoundCopyMode));
    p->setSubcycles(parse(e, UIParameterSubcycles));
    //p->setSustainFunctions(parseString(e, UIParameterSustainFunctions));
    p->setSwitchDuration(parse(e, UIParameterSwitchDuration));
    p->setSwitchLocation(parse(e, UIParameterSwitchLocation));
    p->setSwitchQuantize(parse(e, UIParameterSwitchQuantize));
    p->setSwitchVelocity(parse(e, UIParameterSwitchVelocity));
    p->setTimeCopyMode(parse(e, UIParameterTimeCopyMode));
    p->setTrackLeaveAction(parse(e, UIParameterTrackLeaveAction));
    p->setWindowEdgeAmount(parse(e, UIParameterWindowEdgeAmount));

    // ugh, I seem to have made redundant setters for all these that take an int
    // rather than an enum, but not these, why?  Kind of like not having the duplication
    p->setWindowEdgeUnit((Preset::WindowUnit)parse(e, UIParameterWindowEdgeUnit));
    p->setWindowSlideAmount(parse(e, UIParameterWindowSlideAmount));
    p->setWindowSlideUnit((Preset::WindowUnit)parse(e, UIParameterWindowSlideUnit));
}

//////////////////////////////////////////////////////////////////////
//
// Setup
//
//////////////////////////////////////////////////////////////////////

#define EL_SETUP "Setup"

#define ATT_BINDINGS "bindings"
#define ATT_MIDI_CONFIG "midiConfig"

#define ATT_NAME "name"
#define ATT_ACTIVE "active"
#define ATT_TRACK_GROUPS "trackGroups"
#define ATT_RESET_RETAINS "resetRetains"
#define ATT_ACTIVE "active"

#define EL_SETUP_TRACK "SetupTrack"
#define EL_VARIABLES "Variables"

void XmlRenderer::render(XmlBuffer* b, Setup* setup)
{
	b->addOpenStartTag(EL_SETUP);
	b->setAttributeNewline(true);

	renderStructure(b, setup);

    // these haven't been defined as Parameters, now that we're
    // doing that for the sync options could do these...
    b->addAttribute(ATT_ACTIVE, setup->getActiveTrack());
    b->addAttribute(ATT_BINDINGS, setup->getBindings());
    
    // these are a csv while the function lists in MobiusConfig
    // are String lists, should be consistent, I'm liking csv for brevity
    b->addAttribute(ATT_RESET_RETAINS, setup->getResetRetains());

    render(b, UIParameterBeatsPerBar, setup->getBeatsPerBar());
    // why is the name pattern not followed here?
    render(b, UIParameterDefaultSyncSource, setup->getSyncSource());
    render(b, UIParameterDefaultTrackSyncUnit, setup->getSyncTrackUnit());
    render(b, UIParameterManualStart, setup->isManualStart());
    render(b, UIParameterMaxTempo, setup->getMaxTempo());
    render(b, UIParameterMinTempo, setup->getMinTempo());
    render(b, UIParameterMuteSyncMode, setup->getMuteSyncMode());
    render(b, UIParameterOutRealign, setup->getOutRealignMode());
    render(b, UIParameterRealignTime, setup->getRealignTime());
    render(b, UIParameterResizeSyncAdjust, setup->getResizeSyncAdjust());
    render(b, UIParameterSlaveSyncUnit, setup->getSyncUnit());
    render(b, UIParameterSpeedSyncAdjust, setup->getSpeedSyncAdjust());

    b->add(">\n");
	b->incIndent();

    for (SetupTrack* t = setup->getTracks() ; t != nullptr ; t = t->getNext())
	  render(b, t);

	b->decIndent();
	b->setAttributeNewline(false);
	b->addEndTag(EL_SETUP, true);
}

void XmlRenderer::parse(XmlElement* e, Setup* setup)
{
	parseStructure(e, setup);

	setup->setActiveTrack(e->getIntAttribute(ATT_ACTIVE));
	setup->setBindings(e->getAttribute(ATT_BINDINGS));
    setup->setResetRetains(e->getAttribute(ATT_RESET_RETAINS));

    setup->setBeatsPerBar(parse(e, UIParameterBeatsPerBar));
    setup->setSyncSource((SyncSource)parse(e, UIParameterDefaultSyncSource));
    setup->setSyncTrackUnit((SyncTrackUnit)parse(e, UIParameterDefaultTrackSyncUnit));
    setup->setManualStart(parse(e, UIParameterManualStart));
    setup->setMaxTempo(parse(e, UIParameterMaxTempo));
    setup->setMinTempo(parse(e, UIParameterMinTempo));
    setup->setMuteSyncMode(parse(e, UIParameterMuteSyncMode));
    setup->setOutRealignMode(parse(e, UIParameterOutRealign));
    setup->setRealignTime(parse(e, UIParameterRealignTime));
    setup->setResizeSyncAdjust(parse(e, UIParameterResizeSyncAdjust));
    setup->setSyncUnit((SyncUnit)parse(e, UIParameterSlaveSyncUnit));
    setup->setSpeedSyncAdjust(parse(e, UIParameterSpeedSyncAdjust));

    SetupTrack* tracks = nullptr;
    SetupTrack* last = nullptr;
	for (XmlElement* child = e->getChildElement() ; child != nullptr ; 
		 child = child->getNextElement()) {
        // todo: should verify the element name
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

void XmlRenderer::render(XmlBuffer* b, SetupTrack* t)
{
	b->addOpenStartTag(EL_SETUP_TRACK);

    const char* name = t->getName();
    if (name != nullptr)
	  b->addAttribute(ATT_NAME, name);

    // in the old model, this was driven from Parameters
    // in TRACK scope that did not have the transient flag set
    // this was only InputPort, OutputPort, and PresetNumber
    // actually there are a lot missing and not just ones with transient

    render(b, UIParameterStartingPreset, t->getStartingPresetName());
    render(b, UIParameterFocus, t->isFocusLock());
    render(b, UIParameterMono, t->isMono());
    render(b, UIParameterGroup, t->getGroup());
    render(b, UIParameterInput, t->getInputLevel());
    render(b, UIParameterOutput, t->getOutputLevel());
    render(b, UIParameterFeedback, t->getFeedback());
    render(b, UIParameterAltFeedback, t->getAltFeedback());
    render(b, UIParameterPan, t->getPan());

    render(b, UIParameterAudioInputPort, t->getAudioInputPort());
    render(b, UIParameterAudioOutputPort, t->getAudioOutputPort());
    render(b, UIParameterPluginInputPort, t->getPluginInputPort());
    render(b, UIParameterPluginOutputPort, t->getPluginOutputPort());

    render(b, UIParameterSyncSource, t->getSyncSource());
    render(b, UIParameterTrackSyncUnit, t->getSyncTrackUnit());

    UserVariables* uv = t->getVariables();
    if (uv == nullptr) {
        b->add("/>\n");
    }
    else {
		b->add(">\n");
		b->incIndent();

        render(b, uv);

		b->decIndent();
		b->addEndTag(EL_SETUP_TRACK);
	}
}

void XmlRenderer::parse(XmlElement* e, SetupTrack* t)
{
	t->setName(e->getAttribute(ATT_NAME));
    t->setStartingPresetName(parseString(e, UIParameterStartingPreset));
    t->setFocusLock(parse(e, UIParameterFocus));
    t->setMono(parse(e, UIParameterMono));
    t->setGroup(parse(e, UIParameterGroup));
    t->setInputLevel(parse(e, UIParameterInput));
    t->setOutputLevel(parse(e, UIParameterOutput));
    t->setFeedback(parse(e, UIParameterFeedback));
    t->setAltFeedback(parse(e, UIParameterAltFeedback));
    t->setPan(parse(e, UIParameterPan));

    t->setAudioInputPort(parse(e, UIParameterAudioInputPort));
    t->setAudioOutputPort(parse(e, UIParameterAudioOutputPort));
    t->setPluginInputPort(parse(e, UIParameterPluginInputPort));
    t->setPluginOutputPort(parse(e, UIParameterPluginOutputPort));

    t->setSyncSource((SyncSource)parse(e, UIParameterSyncSource));
    t->setSyncTrackUnit((SyncTrackUnit)parse(e, UIParameterTrackSyncUnit));

    // should only have a single UserVariables 
	for (XmlElement* child = e->getChildElement() ; child != nullptr ; 
		 child = child->getNextElement()) {

		if (child->isName(EL_VARIABLES)) {
            UserVariables* uv = new UserVariables();
            parse(child, uv);
            t->setVariables(uv);
		}
	}
}

#define EL_VARIABLES "Variables"
#define EL_VARIABLE "Variable"
#define ATT_VALUE "value"

void XmlRenderer::render(XmlBuffer* b, UserVariables* container)
{
	b->addOpenStartTag(EL_VARIABLES);
    b->incIndent();
    
    for (UserVariable* v = container->getVariables() ; v != nullptr ; v = v->getNext()) {

        b->addOpenStartTag(EL_VARIABLE);
        b->addAttribute(ATT_NAME, v->getName());

        // note that we'll lose the type during serialization
        // gak this is ugly
        ExValue exv;
        v->getValue(&exv);
        const char* value = exv.getString();
        if (value != nullptr)
          b->addAttribute(ATT_VALUE, value);

        b->add("/>\n");
    }
    
    b->decIndent();
    b->addEndTag(EL_VARIABLES);
}

void XmlRenderer::parse(XmlElement* e, UserVariables* container)
{
    UserVariable* list = nullptr;
	UserVariable* last = nullptr;

	for (XmlElement* child = e->getChildElement() ; child != nullptr ; 
		 child = child->getNextElement()) {

		UserVariable* v = new UserVariable();
        v->setName(child->getAttribute(ATT_NAME));

        // we don't save the type, so a round trip will always stringify
        ExValue exv;
        exv.setString(e->getAttribute(ATT_VALUE));
        v->setValue(&exv);
        
		if (last == nullptr)
		  list = v;
		else
		  last->setNext(v);
		last = v;
	}

    container->setVariables(list);
}

//////////////////////////////////////////////////////////////////////
//
// BindingSet
//
//////////////////////////////////////////////////////////////////////

#define EL_BINDING "Binding"

#define ATT_DISPLAY_NAME "displayName"
#define ATT_TRIGGER "trigger"
#define ATT_VALUE "value"
#define ATT_CHANNEL "channel"
#define ATT_TRIGGER_VALUE "triggerValue"
#define ATT_TRIGGER_PATH "triggerPath"
#define ATT_TRIGGER_TYPE "triggerType"
#define ATT_TARGET_PATH "targetPath"
#define ATT_TARGET "target"
#define ATT_ACTION "action"
#define ATT_OPERATION "op"
#define ATT_ARGS "args"
#define ATT_SCOPE "scope"
#define ATT_TRACK "track"
#define ATT_GROUP "group"

void XmlRenderer::render(XmlBuffer* b, BindingSet* c)
{
	b->addOpenStartTag(EL_BINDING_SET);

	renderStructure(b, c);

	b->add(">\n");
	b->incIndent();

	for (Binding* binding = c->getBindings() ; binding != nullptr ; binding = binding->getNext()) {
        // this was annoying during testing, should be checking validity above
        // so we can at least see what went wrong
        // if (binding->isValid()) {
        render(b, binding);
        // }
    }

	b->decIndent();
	b->addEndTag(EL_BINDING_SET);
}

/**
 * Note that Binding is shared by both BindingSet and OscConfig
 *
 * What is now "actionName" has historically been saved as just "name"
 * which is usually obvious.  Continue with that.
 */
void XmlRenderer::render(XmlBuffer* b, Binding* binding)
{
    b->addOpenStartTag(EL_BINDING);

    // it reads better to lead with the action
    if (binding->action != nullptr)
      b->addAttribute(ATT_ACTION, binding->action->getName());

    b->addAttribute(ATT_NAME, binding->getActionName());
    b->addAttribute(ATT_SCOPE, binding->getScope());

    if (binding->trigger != nullptr) {
        b->addAttribute(ATT_TRIGGER, binding->trigger->getName());
    }

    if (binding->triggerMode != nullptr) {
        b->addAttribute(ATT_TRIGGER_TYPE, binding->triggerMode->getName());
    }
            
    b->addAttribute(ATT_VALUE, binding->triggerValue);

    if (binding->trigger != nullptr && Trigger::isMidi(binding->trigger)) {
        b->addAttribute(ATT_CHANNEL, binding->midiChannel);
    }
            
    b->addAttribute(ATT_ARGS, binding->getArguments());

    b->add("/>\n");
}

void XmlRenderer::parse(XmlElement* e, BindingSet* c)
{
	parseStructure(e, c);

	for (XmlElement* child = e->getChildElement() ; child != nullptr ; 
		 child = child->getNextElement()) {

		if (child->isName(EL_BINDING)) {
			Binding* mb = new Binding();

            parse(child, mb);
            
			// can't filter bogus functions yet, scripts aren't loaded
			c->addBinding(mb);
		}
	}
}

void XmlRenderer::parse(XmlElement* e, Binding* b)
{
    // trigger
    b->trigger = Trigger::find(e->getAttribute(ATT_TRIGGER));
    b->triggerMode = TriggerMode::find(e->getAttribute(ATT_TRIGGER_TYPE));
    b->triggerValue = e->getIntAttribute(ATT_VALUE);
    b->midiChannel = e->getIntAttribute(ATT_CHANNEL);

    // we've gone through two naming cycles on this
    // it was "target" for awhile, which I didn't like since it
    // was confusing with "scope", then I tried "operation" which was klunky
    // and now it's just "action"
    
    const char* typeName = e->getAttribute(ATT_ACTION);
    if (typeName == nullptr)
      typeName = e->getAttribute(ATT_OPERATION);
    if (typeName == nullptr)
      typeName = e->getAttribute(ATT_TARGET);
    
    b->action = ActionType::find(typeName);

    // now disliking calling this "action name" since it confuses
    // with "action type name"
    // maybe go back to target
    b->setActionName(e->getAttribute(ATT_NAME));

    b->setScope(e->getAttribute(ATT_SCOPE));
    b->setArguments(e->getAttribute(ATT_ARGS));
}

//////////////////////////////////////////////////////////////////////
//
// ScriptConfig
//
//////////////////////////////////////////////////////////////////////

#define EL_SCRIPT_CONFIG "ScriptConfig"
#define EL_SCRIPT_REF "ScripRef"
#define ATT_FILE "file"

void XmlRenderer::render(XmlBuffer* b, ScriptConfig* c)
{
    b->addStartTag(EL_SCRIPT_CONFIG);
    b->incIndent();

    for (ScriptRef* ref = c->getScripts() ; ref != nullptr ; ref = ref->getNext()) {
        b->addOpenStartTag(EL_SCRIPT_REF);
        b->addAttribute(ATT_FILE, ref->getFile());
        b->add("/>\n");
    }

    b->decIndent();
    b->addEndTag(EL_SCRIPT_CONFIG);
}

void XmlRenderer::parse(XmlElement* e, ScriptConfig* c)
{
    ScriptRef* list = nullptr;
    ScriptRef* last = nullptr;

    for (XmlElement* child = e->getChildElement() ; child != nullptr ; 
         child = child->getNextElement()) {
        ScriptRef* ref = new ScriptRef();
        ref->setFile(child->getAttribute(ATT_FILE));
        if (last == nullptr)
          list = ref;   
        else
          last->setNext(ref);
        last = ref;
    }

    c->setScripts(list);
}

//////////////////////////////////////////////////////////////////////
//
// SampleConfig
//
//////////////////////////////////////////////////////////////////////

// old name, now render EL_SAMPLE_CONFIG
#define EL_SAMPLE "Sample"
#define ATT_PATH "path"
#define ATT_SUSTAIN "sustain"
#define ATT_LOOP "loop"
#define ATT_CONCURRENT "concurrent"

void XmlRenderer::render(XmlBuffer* b, SampleConfig* c)
{
    // I changed the class name to SampleConfig but for backward
    // compatibility the element and class name were originally Samples
	b->addStartTag(EL_SAMPLE_CONFIG);
	b->incIndent();

    for (Sample* s = c->getSamples() ; s != nullptr ; s = s->getNext()) {

        b->addOpenStartTag(EL_SAMPLE);
        b->addAttribute(ATT_PATH, s->getFilename());
        b->addAttribute(ATT_SUSTAIN, s->isSustain());
        b->addAttribute(ATT_LOOP, s->isLoop());
        b->addAttribute(ATT_CONCURRENT, s->isConcurrent());
        // note that the data block is NOT serialized or parsed
        b->add("/>\n");
    }

	b->decIndent();
	b->addEndTag(EL_SAMPLE_CONFIG);
}

void XmlRenderer::parse(XmlElement* e, SampleConfig* c)
{
    Sample* samples = nullptr;
	Sample* last = nullptr;

	for (XmlElement* child = e->getChildElement() ; child != nullptr ; 
		 child = child->getNextElement()) {

		Sample* s = new Sample();

        s->setFilename(child->getAttribute(ATT_PATH));
        s->setSustain(child->getBoolAttribute(ATT_SUSTAIN));
        s->setLoop(child->getBoolAttribute(ATT_LOOP));
        s->setConcurrent(child->getBoolAttribute(ATT_CONCURRENT));

        if (last == nullptr)
		  samples = s;
		else
		  last->setNext(s);
		last = s;
	}

    c->setSamples(samples);
}

//////////////////////////////////////////////////////////////////////
//
// OscConfig
//
//////////////////////////////////////////////////////////////////////

#define EL_OSC_CONFIG "OscConfig"
#define EL_COMMENTS "Comments"

#define ATT_INPUT_PORT "inputPort"
#define ATT_OUTPUT_PORT "outputPort"
#define ATT_OUTPUT_HOST "outputHost"
#define ATT_TRACE "trace" 

#define EL_BINDING_SET "OscBindingSet"
#define EL_WATCHER "OscWatcher"
#define ATT_PATH "path"
#define ATT_TRACK "track"

void XmlRenderer::render(XmlBuffer* b, OscConfig* c)
{
	b->addOpenStartTag(EL_OSC_CONFIG);
	b->addAttribute(ATT_INPUT_PORT, c->getInputPort());
	b->addAttribute(ATT_OUTPUT_PORT, c->getOutputPort());
	b->addAttribute(ATT_OUTPUT_HOST, c->getOutputHost());
	b->add(">\n");
	b->incIndent();

	for (OscBindingSet* set = c->getBindings() ; set != nullptr ; set = set->getNext()) {
        render(b, set);
    }

    for (OscWatcher* w = c->getWatchers() ; w != nullptr ; w = w->getNext()) {
        render(b, w);
    }
    
	b->decIndent();
	b->addEndTag(EL_OSC_CONFIG);
}

void XmlRenderer::parse(XmlElement* e, OscConfig* c)
{
	OscBindingSet* lastBinding = nullptr;
    OscWatcher* lastWatcher = nullptr;

	c->setInputPort(e->getIntAttribute(ATT_INPUT_PORT));
	c->setOutputPort(e->getIntAttribute(ATT_OUTPUT_PORT));
	c->setOutputHost(e->getAttribute(ATT_OUTPUT_HOST));

	for (XmlElement* child = e->getChildElement() ; child != nullptr ; 
		 child = child->getNextElement()) {

		if (child->isName(EL_BINDING_SET)) {
			OscBindingSet* b = new OscBindingSet();
            if (lastBinding == nullptr)
              c->setBindings(b);
            else
              lastBinding->setNext(b);
            lastBinding = b;
              
            parse(child, b);
		}
        else if (child->isName(EL_WATCHER)) {
            OscWatcher* w = new OscWatcher();
            if (lastWatcher == nullptr)
              c->setWatchers(w);
            else 
              lastWatcher->setNext(w);
            lastWatcher = w;

            parse(child, w);
        }
    }
}

void XmlRenderer::render(XmlBuffer* b, OscBindingSet* obs)
{
	b->addOpenStartTag(EL_OSC_CONFIG);
	b->addAttribute(ATT_INPUT_PORT, obs->getInputPort());
	b->addAttribute(ATT_OUTPUT_PORT, obs->getOutputPort());
	b->addAttribute(ATT_OUTPUT_HOST, obs->getOutputHost());
	b->add(">\n");
	b->incIndent();

    if (obs->getComments() != nullptr) {
        b->addStartTag(EL_COMMENTS);
        b->add(obs->getComments());
        b->addEndTag(EL_COMMENTS);
    }

	for (Binding* binding = obs->getBindings() ; binding != nullptr ; 
		 binding = binding->getNext())
      render(b, binding);

	b->decIndent();
	b->addEndTag(EL_OSC_CONFIG);
}

void XmlRenderer::parse(XmlElement* e, OscBindingSet* obs)
{
	Binding* lastBinding = nullptr;

	obs->setInputPort(e->getIntAttribute(ATT_INPUT_PORT));
	obs->setOutputPort(e->getIntAttribute(ATT_OUTPUT_PORT));
	obs->setOutputHost(e->getAttribute(ATT_OUTPUT_HOST));
    obs->setName(e->getAttribute(ATT_NAME));

	for (XmlElement* child = e->getChildElement() ; child != nullptr ; 
		 child = child->getNextElement()) {

		if (child->isName(EL_BINDING)) {
			Binding* b = new Binding();
            if (lastBinding == nullptr)
              obs->setBindings(b);
            else
              lastBinding->setNext(b);
            lastBinding = b;

            parse(child, b);
		}
        else if (child->isName(EL_COMMENTS)) {
            obs->setComments(child->getContent());
        }
	}
}

void XmlRenderer::render(XmlBuffer* b, OscWatcher* w)
{
    b->addOpenStartTag(EL_WATCHER);
	b->addAttribute(ATT_PATH, w->getPath());
	b->addAttribute(ATT_NAME, w->getName());
    b->addAttribute(ATT_TRACK, w->getTrack());
    b->add("/>\n");
}

void XmlRenderer::parse(XmlElement* e, OscWatcher* w)
{
    w->setPath(e->getAttribute(ATT_PATH));
    w->setName(e->getAttribute(ATT_NAME));
    w->setTrack(e->getIntAttribute(ATT_TRACK));
}

//////////////////////////////////////////////////////////////////////
//
// UIConfig
//
// WTF is TrackStrip2?  Get better names for these
//
//////////////////////////////////////////////////////////////////////

#define ATT_NAME "name"
#define ATT_REFRESH "refreshInterval"
#define ATT_ALERT_INTERVALS "alertIntervals"
#define ATT_MESSAGE_DURATION "messageDuration"
#define ATT_WINDOW_WIDTH "width"
#define ATT_WINDOW_HEIGHT "height"
 
#define EL_LOCATIONS "Locations"
#define EL_LOCATION "Location"
#define ATT_X "x"
#define ATT_Y "y"
#define ATT_WIDTH "width"
#define ATT_HEIGHT "height"
#define ATT_DISABLED "disabled"

#define EL_BUTTONS "Buttons"
#define EL_BUTTON "Button"
#define ATT_BUTTON_NAME "target"
#define ATT_BUTTON_ARGS "arguments"

// don't really like these as top level things, would make more sense
// inside the <Location> element, consider generalizing <Location>
// to <Component> and allowing it to have arbitrary <Property>s.

#define EL_PARAMETERS "InstantParameters"
#define EL_PARAMETER "Parameter"

#define EL_OLD_TRACK_CONTROLS "TrackControls"
#define EL_FLOATING_TRACK_STRIP "FloatingTrackStrip"
#define EL_FLOATING_TRACK_STRIP2 "FloatingTrackStrip2"
#define EL_DOCKED_TRACK_STRIP "DockedTrackStrip"

#define EL_COMPONENT "Component"

void XmlRenderer::render(XmlBuffer* b, UIConfig* c)
{
	b->addOpenStartTag(EL_UI_CONFIG);

    // these won't ever have names currently
    b->addAttribute(ATT_NAME, c->getName());

    b->addAttribute(ATT_WINDOW_WIDTH, c->getWindowWidth());
    b->addAttribute(ATT_WINDOW_HEIGHT, c->getWindowHeight());
    b->addAttribute(ATT_REFRESH, c->getRefreshInterval());
    b->addAttribute(ATT_MESSAGE_DURATION, c->getMessageDuration());

    // this has never been used and I'm not even sure what it was for
    //b->addAttribute(ATT_ALERT_INTERVALS, mAlertIntervals);

	b->add(">\n");
	b->incIndent();

    std::vector<std::unique_ptr<UILocation>>* locations = c->getLocations();
	if (locations->size() > 0) {
		b->addStartTag(EL_LOCATIONS);
		b->incIndent();
		for (int i = 0 ; i < locations->size() ; i++) {
            // sweet jesus, vectors of unique_ptr are subtle
            // juce::<OwnedArray> is much more obvious
			UILocation* location = locations->at(i).get();
            b->addOpenStartTag(EL_LOCATION);
            b->addAttribute(ATT_NAME, location->getName());
            // these are normally always set
            b->addAttribute(ATT_X, location->x);
            b->addAttribute(ATT_Y, location->y);
            // these are set only if they override the defaults
            if (location->width > 0)
              b->addAttribute(ATT_WIDTH, location->width);
            if (location->height > 0)
              b->addAttribute(ATT_HEIGHT, location->height);
            b->addAttribute(ATT_DISABLED, location->disabled);
            b->add("/>\n");
		}
		b->decIndent();
		b->addEndTag(EL_LOCATIONS);
	}

    // assuming that the Button name is the same as a function name
    // will need more here for display names, old model allowed
    // these to be bound to anything though I doubt that was used
    std::vector<std::unique_ptr<UIButton>>* buttons = c->getButtons();
	if (buttons->size() > 0) {
		b->addStartTag(EL_BUTTONS);
		b->incIndent();
		for (int i = 0 ; i < buttons->size() ; i++) {
            UIButton* button = buttons->at(i).get();
            b->addOpenStartTag(EL_BUTTON);
            b->addAttribute(ATT_BUTTON_NAME, button->getName());
            b->addAttribute(ATT_BUTTON_ARGS, button->getArguments());
            b->add("/>\n");
		}
		b->decIndent();
		b->addEndTag(EL_BUTTONS);
	}
    
    renderList(b, EL_PARAMETERS, c->getParameters());
    renderList(b, EL_FLOATING_TRACK_STRIP, c->getFloatingStrip());
    renderList(b, EL_FLOATING_TRACK_STRIP2, c->getFloatingStrip2());
    renderList(b, EL_DOCKED_TRACK_STRIP, c->getDockedStrip());

    b->decIndent();

	b->addEndTag(EL_UI_CONFIG);
}

void XmlRenderer::parse(XmlElement* e, UIConfig* config)
{
    config->setName(e->getAttribute(ATT_NAME));
    config->setWindowWidth(e->getIntAttribute(ATT_WINDOW_WIDTH));
    config->setWindowHeight(e->getIntAttribute(ATT_WINDOW_HEIGHT));
    config->setRefreshInterval(e->getIntAttribute(ATT_REFRESH, DEFAULT_REFRESH_INTERVAL));
    config->setAlertIntervals(e->getIntAttribute(ATT_ALERT_INTERVALS, DEFAULT_ALERT_INTERVALS));
    config->setMessageDuration(e->getIntAttribute(ATT_MESSAGE_DURATION, DEFAULT_MESSAGE_DURATION));

	for (XmlElement* child = e->getChildElement() ; child != NULL ; 
		 child = child->getNextElement()) {

		if (child->isName(EL_LOCATIONS)) {
            // ugh, c++ unique_ptr semantics make building lists outside
            // and setting them hard, supposed to implement a "move constructor"
            // for now just let addLocation do it
			for (XmlElement* le = child->getChildElement() ; le != NULL ; 
				 le = le->getNextElement()) {
                UILocation* loc = new UILocation();
                loc->setName(le->getAttribute(ATT_NAME));
                loc->x = le->getIntAttribute(ATT_X);
                loc->y = le->getIntAttribute(ATT_Y);
                loc->width = le->getIntAttribute(ATT_WIDTH);
                loc->height = le->getIntAttribute(ATT_HEIGHT);
                loc->disabled = le->getBoolAttribute(ATT_DISABLED);
                config->addLocation(loc);
            }
		}
        else if (child->isName(EL_BUTTONS)) {
			for (XmlElement* be = child->getChildElement() ; be != NULL ; 
				 be = be->getNextElement()) {
                const char* name = be->getAttribute(ATT_BUTTON_NAME);
                // ignore malformed objects
                if (name != nullptr) {
                    UIButton* button = new UIButton();
                    button->setName(be->getAttribute(ATT_BUTTON_NAME));
                    button->setArguments(be->getAttribute(ATT_BUTTON_ARGS));                    
                    config->addButton(button);
                }
                
            }
        }

		if (child->isName(EL_PARAMETERS)) {
            config->setParameters(parseStringList(child));
		}
		else if (child->isName(EL_FLOATING_TRACK_STRIP)) {
            config->setFloatingStrip(parseStringList(child));
		}
		else if (child->isName(EL_FLOATING_TRACK_STRIP2)) {
            config->setFloatingStrip2(parseStringList(child));
		}
		else if (child->isName(EL_DOCKED_TRACK_STRIP)) {
            config->setDockedStrip(parseStringList(child));
        }
	}
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

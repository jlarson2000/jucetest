/**
 * Base class for components within a TrackStrip
 */

#include <JuceHeader.h>

#include "../../model/UIConfig.h"
#include "../../model/MobiusConfig.h"
#include "../../model/Parameter.h"

#include "Colors.h"
#include "TrackStrip.h"
#include "StripElement.h"

//////////////////////////////////////////////////////////////////////
//
// Definitions
//
// A set of static objects that define things about the elements
// that can be selected for display.  
//
//////////////////////////////////////////////////////////////////////

// todo: these should be SystemConsants for consistency and to add
// help text later, though if you construct them with Parameter
// we get that.  The old Parameter names don't always match what we have
// historically used to reference them in the UIConfig though, so keep
// them distinct, but need to rethink this

/**
 * Global registry
 */
std::vector<StripElementDefinition*> StripElementDefinition::Elements;

/**
 * Find a strip element definition by name
 */
StripElementDefinition* StripElementDefinition::find(const char* name)
{
	StripElementDefinition* found = nullptr;
	
	for (int i = 0 ; i < Elements.size() ; i++) {
		StripElementDefinition* d = Elements[i];
		if (StringEqualNoCase(d->getName(), name)) {
            found = d;
            break;
        }
	}
	return found;
}

/**
 * For elements that correespond to Parameters, pull the names
 * from the Parameter definition.
 *
 * Interesting order dependency here.  Both Parameter and StripElementDefinition
 * are static objects and one references the other.  What is the order in
 * which they are defined?  Is it certain that Parameter will be first?
 */
StripElementDefinition::StripElementDefinition(Parameter* p)
{
    parameter = p;
    name = nullptr;
    displayName = nullptr;
    Elements.push_back(this);
}

/**
 * Name is how we refer to them internally in the UIConfig
 * Display name is what we show in the UI
 */
StripElementDefinition::StripElementDefinition(const char* argName, const char* argDisplayName)
{
    parameter = nullptr;
    name = argName;
    displayName = argDisplayName;
    Elements.push_back(this);
}

const char* StripElementDefinition::getName()
{
    if (parameter != nullptr)
      return parameter->getName();
    else
      return name;
}

const char* StripElementDefinition::getDisplayName()
{
    if (parameter != nullptr)
      return parameter->getDisplayableName();
    else
      return displayName;
}

// I'm really growing to hate this little dance, find a better way!

StripElementDefinition StripInputObj {InputLevelParameter};
StripElementDefinition* StripDefinitionInput = &StripInputObj;

StripElementDefinition StripOutputObj {OutputLevelParameter};
StripElementDefinition* StripDefinitionOutput = &StripOutputObj;

StripElementDefinition StripFeedbackObj {FeedbackLevelParameter};
StripElementDefinition* StripDefinitionFeedback = &StripFeedbackObj;

StripElementDefinition StripAltFeedbackObj {AltFeedbackLevelParameter};
StripElementDefinition* StripDefinitionAltFeedback = &StripAltFeedbackObj;

StripElementDefinition StripPanObj {PanParameter};
StripElementDefinition* StripDefinitionPan = &StripPanObj;

// the defaults for the dock, also OutputLevel
StripElementDefinition StripTrackNumberObj {"trackNumber", "Track Number"};
StripElementDefinition* StripDefinitionTrackNumber = &StripTrackNumberObj;

StripElementDefinition StripLoopRadarObj {"loopRadar", "Loop Radar"};
StripElementDefinition* StripDefinitionLoopRadar = &StripLoopRadarObj;

// formerly called "loopStatus"
StripElementDefinition StripLoopStackObj {"loopStack", "Loop Stack"};
StripElementDefinition* StripDefinitionLoopStack = &StripLoopStackObj;

StripElementDefinition StripOutputMeterObj {"outputMeter", "Output Meter"};
StripElementDefinition* StripDefinitionOutputMeter = &StripOutputMeterObj;


// optional but popular
StripElementDefinition StripGroupNameObj {"groupName", "Group Name"};
StripElementDefinition* StripDefinitionGroupName = &StripGroupNameObj;

StripElementDefinition StripLoopThermometerObj {"loopMeter", "Loop Meter"};
StripElementDefinition* StripDefinitionLoopThermometer = &StripLoopThermometerObj;

// obscure options

// this was a little button we don't need if the track
// number is clickable for focus
StripElementDefinition StripFocusLockObj {"focusLock", "Focus Lock"};
StripElementDefinition* StripDefinitionFocusLock = &StripFocusLockObj;

StripElementDefinition StripPitchOctaveObj {PitchOctaveParameter};
StripElementDefinition* StripDefinitionPitchOctave = &StripPitchOctaveObj;

StripElementDefinition StripPitchStepObj {PitchStepParameter};
StripElementDefinition* StripDefinitionPitchStep = &StripPitchStepObj;

StripElementDefinition StripPitchBendObj {PitchBendParameter};
StripElementDefinition* StripDefinitionPitchBend = &StripPitchBendObj;

StripElementDefinition StripSpeedOctaveObj {SpeedOctaveParameter};
StripElementDefinition* StripDefinitionSpeedOctave = &StripSpeedOctaveObj;

StripElementDefinition StripSpeedStepObj {SpeedStepParameter};
StripElementDefinition* StripDefinitionSpeedStep = &StripSpeedStepObj;

StripElementDefinition StripSpeedBendObj {SpeedBendParameter};
StripElementDefinition* StripDefinitionSpeedBend = &StripSpeedBendObj;

StripElementDefinition StripTimeStretchObj {TimeStretchParameter};
StripElementDefinition* StripDefinitionTimeStretch = &StripTimeStretchObj;

// find a way to put these inside StripElement for namespace

const StripElementDefinition* StripDockDefaults[] = {
    StripDefinitionTrackNumber,
    // StripDefinitionLoopRadar,
    StripDefinitionLoopThermometer,
    StripDefinitionLoopStack,
    StripDefinitionOutput,
    StripDefinitionOutputMeter,
    nullptr
};

//////////////////////////////////////////////////////////////////////
//
// Component
//
//////////////////////////////////////////////////////////////////////

/**
 * Element that doesn't have a Parameter
 * Put the definition name in ComponentID for searching
 * with Juce::Component::findChildWithComponentID
 * Put the name in Component::name for JuceUtil trace
 */
StripElement::StripElement(TrackStrip* parent, StripElementDefinition* def)
{
    strip = parent;
    definition = def;
    setComponentID(def->getName());
    setName(def->getName());
}
        
StripElement::~StripElement()
{
}

void StripElement::configure(UIConfig* config)
{
}

void StripElement::update(MobiusState* state)
{
}

// these should probably be pure virtual
// any useful thing to do in a default implementation?

int StripElement::getPreferredWidth()
{
    return 50;
}

int StripElement::getPreferredHeight()
{
    return 20;
}

/**
 * todo: might want some default painting for borders, labels,
 * and size dragging.  Either the subclass must call back up to this
 * or we have a different paintElement function.
 */
void StripElement::paint(juce::Graphics& g)
{
    // start by bordering everything
    g.setColour(juce::Colour(MobiusRed));
    g.drawRect(getLocalBounds());
}


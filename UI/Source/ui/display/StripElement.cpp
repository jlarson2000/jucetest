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
//////////////////////////////////////////////////////////////////////

// todo: these should be SystemConsants for consistency and to add
// help text later, though if you construct them with Parameter
// we get that.  The old Parameter names don't always match what we have
// historically used to reference them in the UIConfig though, so keep
// them distinct, but need to rethink this

/**
 * Find a strip element definition by name
 */
StripElementDefinition* StripElementDefinition::getElement(const char* name)
{
	StripElementDefinition* found = nullptr;
	
	for (int i = 0 ; i < Elements.size() ; i++) {
		StripElementDefinition* d = Elements[i];
		if (StringEqualNoCase(d->name, name)) {
            found = d;
            break;
        }
	}
	return found;
}

std::vector<StripElementDefinition*> StripElementDefinition::Elements;

/**
 * Name is how we refer to them internally in the UIConfig
 * Display name is what we show in the UI
 */
StripElementDefinition::StripElementDefinition(const char* argName, const char* argDisplayName)
{
    name = argName;
    displayName = argDisplayName;
    
    Elements.push_back(this);
}

StripElementDefinition StripInputLevelObj {"inputLevel", "Input Level"};
StripElementDefinition* StripInputLevel = &StripInputLevelObj;

StripElementDefinition StripOutputLevelObj {"outputLevel", "Output Level"};
StripElementDefinition* StripOutputLevel = &StripOutputLevelObj;

StripElementDefinition StripFeedbackObj {"feedback", "Feedback"};
StripElementDefinition* StripFeedback = &StripFeedbackObj;

StripElementDefinition StripSecondaryFeedbackObj {"secondaryFeedback", "Secondary Feedback"};
StripElementDefinition* StripSecondaryFeedback = &StripSecondaryFeedbackObj;

StripElementDefinition StripPanObj {"pan", "Pan"};
StripElementDefinition* StripPan = &StripPanObj;


// the defaults for the dock, also OutputLevel
StripElementDefinition StripTrackNumberObj {"trackNumber", "Track Number"};
StripElementDefinition* StripTrackNumber = &StripTrackNumberObj;

StripElementDefinition StripLoopRadarObj {"loopRadar", "Loop Radar"};
StripElementDefinition* StripLoopRadar = &StripLoopRadarObj;

StripElementDefinition StripLoopStatusObj {"loopStatus", "Loop Status"};
StripElementDefinition* StripLoopStatus = &StripLoopStatusObj;

StripElementDefinition StripOutputMeterObj {"outputMeter", "Output Meter"};
StripElementDefinition* StripOutputMeter = &StripOutputMeterObj;


// optional but popular
StripElementDefinition StripGroupNameObj {"groupName", "Group Name"};
StripElementDefinition* StripGroupName = &StripGroupNameObj;

StripElementDefinition StripLoopThermometerObj {"loopMeter", "Loop Meter"};
StripElementDefinition* StripLoopThermometer = &StripLoopThermometerObj;

// obscure options

// this was a little button we don't need if the track
// number is clickable for focus
StripElementDefinition StripFocusLockObj {"focusLock", "Focus Lock"};
StripElementDefinition* StripFocusLock = &StripFocusLockObj;

StripElementDefinition StripPitchOctaveObj {"pitchOctave", "Pitch Octave"};
StripElementDefinition* StripPitchOctave = &StripPitchOctaveObj;

StripElementDefinition StripPitchStepObj {"pitchStep", "Pitch Step"};
StripElementDefinition* StripPitchStep = &StripPitchStepObj;

StripElementDefinition StripPitchBendObj {"pitchBend", "Pitch Bend"};
StripElementDefinition* StripPitchBend = &StripPitchBendObj;

StripElementDefinition StripSpeedOctaveObj {"speedOctave", "Speed Octave"};
StripElementDefinition* StripSpeedOctave = &StripSpeedOctaveObj;

StripElementDefinition StripSpeedStepObj {"speedStep", "Speed Step"};
StripElementDefinition* StripSpeedStep = &StripSpeedStepObj;

StripElementDefinition StripSpeedBendObj {"speedBend", "Speed Bend"};
StripElementDefinition* StripSpeedBend = &StripSpeedBendObj;

StripElementDefinition StripTimeStretchObj {"timeStretch", "Time Stretch"};
StripElementDefinition* StripTimeStretch = &StripTimeStretchObj;

// find a way to put these inside StripElement for namespace

const StripElementDefinition* StripDockDefaults[] = {
    StripTrackNumber,
    // StripLoopRadar,
    StripLoopThermometer,
    StripLoopStatus,
    StripOutputLevel,
    StripOutputMeter,
    nullptr
};

//////////////////////////////////////////////////////////////////////
//
// Component
//
//////////////////////////////////////////////////////////////////////

StripElement::StripElement(TrackStrip* parent, StripElementDefinition* def)
{
    setComponentID(def->name);
    setName(def->name);
    strip = parent;
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


/**
 * Base component for things that can go inside a TrackStrip.
 * There are two models here, the Juce Component model to display
 * the element and a definitional model to drive the configuration UI
 */

#pragma once

#include <JuceHeader.h>

#include <vector>

//////////////////////////////////////////////////////////////////////
//
// Definition
//
////////////////////////////////////////////////////////////////////////

/**
 * Define characteristics of strip elements.
 * Mostly this is just the name and how the contexts
 * in which they can be used.
 *
 * Would be nice to have a Component constructor in here
 * too but I need to brush up on lambdas or function pointers for that.
 *
 * Some of these correspond go Parameters, some are strictly for UI control.
 */
class StripElementDefinition
{
  public:

    static std::vector<StripElementDefinition*> Elements;
    static StripElementDefinition* find(const char* name);

    StripElementDefinition(class Parameter* p);
    StripElementDefinition(const char* argName, const char* argDisplayName);
    
    const char* getName();
    const char* getDisplayName();

    ~StripElementDefinition() {};

    // most correspond to Parameters
    class Parameter* parameter;
    
    // those that don't have names here
    const char* name;
    const char* displayName;

    // todo: flags to limit which elements can be in
    // the docked or floating strips

};

// the defaults for the floating strip
extern StripElementDefinition* StripDefinitionInput;
extern StripElementDefinition* StripDefinitionOutput;
extern StripElementDefinition* StripDefinitionFeedback;
extern StripElementDefinition* StripDefinitionAltFeedback;
extern StripElementDefinition* StripDefinitionPan;

// the defaults for the dock, also OutputLevel
extern StripElementDefinition* StripDefinitionTrackNumber;
extern StripElementDefinition* StripDefinitionLoopRadar;
extern StripElementDefinition* StripDefinitionLoopStack;
extern StripElementDefinition* StripDefinitionOutputMeter;

// optional but useful
extern StripElementDefinition* StripDefinitionGroupName;
extern StripElementDefinition* StripDefinitionLoopThermometer;

// obscure options

// this was a little button we don't need if the track
// number is clickable for focus, but it is smaller if you
// don't need numbers
extern StripElementDefinition* StripDefinitionFocusLock;

// these are seldom used Parameters and Pitch doesn't work
// very well, need to hide Pitch till I reimplement it
extern StripElementDefinition* StripDefinitionPitchOctave;
extern StripElementDefinition* StripDefinitionPitchStep;
extern StripElementDefinition* StripDefinitionPitchBend;
extern StripElementDefinition* StripDefinitionSpeedOctave;
extern StripElementDefinition* StripDefinitionSpeedStep;
extern StripElementDefinition* StripDefinitionSpeedBend;
extern StripElementDefinition* StripDefinitionTimeStretch;

// this would be better as a flag in the definittion
extern const StripElementDefinition* StripDockDefaults[];

//////////////////////////////////////////////////////////////////////
//
// Component
//
//////////////////////////////////////////////////////////////////////

class StripElement : public juce::Component
{
  public:
    
    StripElement(class TrackStrip*, StripElementDefinition* def);
    ~StripElement();

    virtual void configure(class UIConfig* config);
    virtual void update(class MobiusState* state);
    
    virtual int getPreferredWidth();
    virtual int getPreferredHeight();

    void paint(juce::Graphics& g) override;
    
  protected:

    StripElementDefinition* definition;
    class TrackStrip* strip;
    
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


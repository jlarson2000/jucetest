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
    static StripElementDefinition* getElement(const char* name);

    StripElementDefinition(const char* argName, const char* argDisplayName);
    ~StripElementDefinition() {};

    const char* name;
    const char* displayName;

    // todo: flags to limit which elements can be in
    // the docked or floating strips
};

// the defaults for the floating strip
extern StripElementDefinition* StripInputLevel;
extern StripElementDefinition* StripOutputLevel;
extern StripElementDefinition* StripFeedback;
extern StripElementDefinition* StripSecondaryFeedback;
extern StripElementDefinition* StripPan;

// the defaults for the dock, also OutputLevel
extern StripElementDefinition* StripTrackNumber;
extern StripElementDefinition* StripLoopRadar;
extern StripElementDefinition* StripLoopStatus;
extern StripElementDefinition* StripOutputMeter;

// optional but popular
extern StripElementDefinition* StripGroupName;
extern StripElementDefinition* StripLoopThermometer;

// obscure options

// this was a little button we don't need if the track
// number is clickable for focus
extern StripElementDefinition* StripFocusLock;

extern StripElementDefinition* StripPitchOctave;
extern StripElementDefinition* StripPitchStep;
extern StripElementDefinition* StripPitchBend;
extern StripElementDefinition* StripSpeedOctave;
extern StripElementDefinition* StripSpeedStep;
extern StripElementDefinition* StripSpeedBend;
extern StripElementDefinition* StripTimeStretch;

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

    class TrackStrip* strip;

};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


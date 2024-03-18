/*
 * Hmm, since these can be in two different container contexts, th
 * parent pointer will have to be more flexible.  Could define a
 * TrackStripOwner interface for them to implement but since there
 * are only two, just allow this to be constructed in both ways
 * and save a pointer to each.
 *
 * "Dockness" then becomes a side effect of being within TrackStrips
 * and "floating" is being within a StatusAreaWrapper.
 */

#include <JuceHeader.h>

#include "../../model/UIConfig.h"
#include "../../model/MobiusState.h"

#include "Colors.h"
#include "StripElement.h"
#include "StripElements.h"
#include "TrackStrips.h"
#include "TrackStrip.h"
#include "FloatingStripElement.h"

// eventually have one that takes a StatusAreaWrapper parent
TrackStrip::TrackStrip(TrackStrips* parent)
{
    setName("TrackStrip");
    strips = parent;
    floater = nullptr;
}

TrackStrip::TrackStrip(FloatingStripElement* parent)
{
    setName("TrackStrip");
    floater = parent;
    strips = nullptr;
}

TrackStrip::~TrackStrip()
{
}

/**
 * Set the track to follow, -1 means the active track.
 * For floaters, could give them a component to select the track.
 */
void TrackStrip::setFollowTrack(int t)
{
    followTrack = t;
    // won't set this after construction so don't need to repaint
}

/**
 * If we're a floating strip, this is the number of the floating
 * strip configuration to pull out of UIConfig.
 * Currently there are only two but we'll allow more.
 * The number is zero based.
 */
void TrackStrip::setFloatingConfig(int i)
{
    floatingConfig = i;
}

/**
 * If we follow a speicfic track return it.
 * If we're floating must have remembered it.
 */
int TrackStrip::getTrackNumber()
{
    int tnum = 0;
    if (followTrack >= 0) {
        tnum = followTrack;
    }
    else {
        // update needs to have saved it
        tnum = activeTrack;
    }
    return tnum;
}

void TrackStrip::layout(juce::Rectangle<int> parentBounds)
{
    // offset for border
    int leftOffset = 2;
    int topOffset = 2;
    int maxWidth = 0;

    for (int i = 0 ; i < elements.size() ; i++) {
        StripElement* el = elements[i];
        // assume for now these don't have complex layouts
        int width = el->getPreferredWidth();
        int height = el->getPreferredHeight();
        el->setBounds(leftOffset, topOffset, width, height);
        
        if (width > maxWidth)
          maxWidth = width;
        topOffset += height;
    }

    // add left/right border
    maxWidth += 4;
    // topOffset already had top border, add bottom
    topOffset += 2;
    
    setSize(maxWidth, topOffset);
}

void TrackStrip::resized()
{
    // did sizing in layout
    // could center the elements here or do it in layout
}

void TrackStrip::update(MobiusState* state)
{
    for (int i = 0 ; i < elements.size() ; i++) {
        StripElement* el = elements[i];
        el->update(state);
    }

    if (activeTrack != state->activeTrack) {
        activeTrack = state->activeTrack;
        repaint();
    }
}

void TrackStrip::paint(juce::Graphics& g)
{
    if (strips != nullptr) {
        // we're in the dock, border shows active
        if (activeTrack == followTrack) {
            g.setColour(juce::Colours::white);
            g.drawRect(getLocalBounds(), 2);
        }
    }
    else {
        // floater paints itself
    }
}

//////////////////////////////////////////////////////////////////////
//
// Configuration
//
//////////////////////////////////////////////////////////////////////

/**
 * Pull out the appropriate configuration.
 * If we're docked use getDockedStrip.
 * If we're floating use getFloatingStrip or getFloatingStrip2
 * until we can support more than 2.
  */
void TrackStrip::configure(UIConfig* config)
{
    StringList* elementNames = nullptr;

    // isn't there a removeAllChildComponent or something?
    for (int i = 0 ; i < elements.size() ; i++) {
      removeChildComponent(elements[i]);
    }
    elements.clear();

    if (strips != nullptr) {
        // we're docked
        elementNames = config->getDockedStrip();
        if (elementNames == nullptr || elementNames->size() == 0) {
            // convenience boostrapping of the usual set
            // during development
            elementNames = new StringList();
            // todo: use a damn vector
            for (int i = 0 ; StripDockDefaults[i] != nullptr ; i++) {
                const StripElementDefinition* def = StripDockDefaults[i];
                elementNames->add(def->name);
            }

            // add it to the passed config for memory management
            // but it won't be saved
            config->setDockedStrip(elementNames);
        }
    }
    else {
        // we're floating
        if (floatingConfig == 0) {
            elementNames = config->getFloatingStrip();
        }
        else {
            elementNames = config->getFloatingStrip2();
        }
    }
    
    // todo: need a lambda or subclass for these
    // god this is horrible
    if (elementNames != nullptr) {
        for (int i = 0 ; i < elementNames->size() ; i++) {
            const char* name = elementNames->getString(i);
            StripElement* el = nullptr;
            StripElementDefinition* def = StripElementDefinition::find(name);
            if (def == StripDefinitionTrackNumber) {
                el = new StripTrackNumber(this);
            }
            else if (def == StripDefinitionFocusLock) {
                el = new StripFocusLock(this);
            }
            else if (def == StripDefinitionLoopRadar) {
                el = new StripLoopRadar(this);
            }
            else if (def == StripDefinitionLoopThermometer) {
                el = new StripLoopThermometer(this);
            }
            else if (def == StripDefinitionOutput) {
                el = new StripOutput(this);
            }
            else if (def == StripDefinitionInput) {
                el = new StripInput(this);
            }
            else if (def == StripDefinitionFeedback) {
                el = new StripFeedback(this);
            }
            else if (def == StripDefinitionAltFeedback) {
                el = new StripAltFeedback(this);
            }
            else if (def == StripDefinitionLoopStack) {
                el = new StripLoopStack(this);
            }
        
            if (el != nullptr) {
                addAndMakeVisible(el);
                elements.add(el);
            }
        }
    }
    
    // now let each element configure
    for (int i = 0 ; i < elements.size() ; i++) {
        StripElement* el = elements[i];
        el->configure(config);
    }
}
        
/**
 * A few of them like LoopStack need MobiusConfig
 * This MUST be called after configure(UIConfig) which
 * is what builds the elements in the first place
 * ugh, don't like the order dependency, can't we have just
 * one configure() that passes both?
 */
void TrackStrip::configure(MobiusConfig* config)
{
    // now let each element configure
    for (int i = 0 ; i < elements.size() ; i++) {
        StripElement* el = elements[i];
        el->configure(config);
    }
}

/**
 * Called by one of the sub elements to perform an action.
 * Here we'll add the track scope and pass it along up
 */
void TrackStrip::doAction(UIAction* action)
{
    // sigh, TrackStrip.followTrack is -1 for active track
    // action is 0 based, but we use 0 meaning active elsewhere
    action->scopeTrack = followTrack + 1;
    
    if (strips != nullptr)
      strips->doAction(action);
    else if (floater != nullptr)
      floater->doAction(action);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

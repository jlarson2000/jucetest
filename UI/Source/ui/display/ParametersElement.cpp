
#include <JuceHeader.h>

#include "../../util/Trace.h"
#include "../../util/List.h"
#include "../../model/UIConfig.h"
#include "../../model/MobiusConfig.h"
#include "../../model/Preset.h"
#include "../../model/MobiusState.h"
#include "../../model/UIParameter.h"

#include "../../Supervisor.h"
#include "../../mobius/MobiusInterface.h"

#include "Colors.h"
#include "StatusArea.h"

#include "ParametersElement.h"

const int ParametersRowHeight = 20;
const int ParametersVerticalGap = 1;
const int ParametersValueWidth = 100;
const int ParametersHorizontalGap = 4;

ParametersElement::ParametersElement(StatusArea* area) :
    StatusElement(area, "ParametersElement")
{
}

ParametersElement::~ParametersElement()
{
}

void ParametersElement::configure(UIConfig* config)
{
    parameters.clear();
    
    // will probably want a sub-component list for these
    StringList* list = config->getParameters();
    if (list != nullptr) {
        for (int i = 0 ; i < list->size() ; i++) {
            const char* name = list->getString(i);
            UIParameter* p = UIParameter::find(name);
            if (p == nullptr) {
                trace("Invalid parameter name %s\n", name);
            }
            else {
                parameters.add(p);
            }
        }
    }
}

/**
 * Special case kludge for TrackPresetParameter
 * We need to map ordinal Preset numbers returned by
 * MobiusInterface into preset names for display.
 * Rather than having to go back to MobiusConfig down
 * in paint() just capture them here.  There are a very
 * small handful os parameters that need this support.
 * Presets, Setups, and BindingSets can all have names
 * but only TrackPreset is likely to be shown.
 */
void ParametersElement::configure(MobiusConfig* config)
{
    presetNames.clear();

    // assign them ordinals while we're at it,
    // this needs to be done higher!!
    int ordinal = 0;
    Preset* preset = config->getPresets();
    while (preset != nullptr) {
        preset->ordinal = ordinal;
        ordinal++;
        presetNames.add(juce::String(preset->getName()));
        preset = (Preset*)(preset->getNext());
    }
}

int ParametersElement::getPreferredHeight()
{
    return (ParametersRowHeight + ParametersVerticalGap) * parameters.size();
}

int ParametersElement::getPreferredWidth()
{
    juce::Font font = juce::Font(ParametersRowHeight);

    int maxName = 0;
    for (int i = 0 ; i < parameters.size() ; i++) {
        UIParameter* p = parameters[i];
        int nameWidth = font.getStringWidth(p->getDisplayableName());
        if (nameWidth > maxName)
          maxName = nameWidth;
    }

    // remember this for paint
    // StatusArea must resize after configure() is called
    maxNameWidth = maxName;
    
    // width of parmeter values is relatively constrained, the exception
    // being preset names.  For enumerated values, assume our static size is enough
    // but could be smarter.  Would require iteration of all possible enumeration
    // values which we don't have yet
    
    int maxValue = ParametersValueWidth;
    for (int i = 0 ; i < presetNames.size() ; i++) {
        int nameWidth = font.getStringWidth(presetNames[i]);
        if (nameWidth > maxValue)
          maxValue;
    }

    // also save this for paint
    maxValueWidth = maxValue;
    
    return maxNameWidth + ParametersHorizontalGap + maxValueWidth;
}

/**
 * Save the values of the parameters for display.
 * Since we save them for difference detection we also don't need
 * to go back to MobiusInterface in paint to get them again.
 */
void ParametersElement::update(MobiusState* state)
{
    bool changes = false;
    
    for (int i = 0 ; i < parameters.size() ; i++) {
        UIParameter* p = parameters[i];

        // here we need MobiusInterface to call getParameter
        // and we don't have a hierarchy walker like doAction
        // this is the first display element that needs to this
        // we can either walk up for getParameter, or just get
        // the whole shebang
        // now that we have this, don't really need to pass doAction
        // all the way up
        Supervisor* super = area->getSupervisor();
        MobiusInterface* mobius = super->getMobius();
        int value = mobius->getParameter(p, state->activeTrack);
        if (i < parameterValues.size()) {
            if (parameterValues[i] != value)
              changes = true;
        }
        else {
            // saved values is too short
            changes = true;
        }
        parameterValues.set(i, value);
    }

    if (changes)
      repaint();
}

void ParametersElement::resized()
{
}

void ParametersElement::paint(juce::Graphics& g)
{
    // borders, labels, etc.
    StatusElement::paint(g);

    g.setFont(juce::Font(ParametersRowHeight));

    int rowTop = 0;
    for (int i = 0 ; i < parameters.size() ; i++) {
        UIParameter* p = parameters[i];
        int value = parameterValues[i];
        juce::String strValue;
        
        // special case kludge for this one, if we start having
        // more think of a more general way to do this
        if (p == UIParameterStartingPreset) {
            if (i < presetNames.size())
              strValue = presetNames[value];
            else
              strValue = "???";
        }
        else {
            if (p->type == TypeEnum) {
                strValue = juce::String(p->getEnumName(value));
            }
            else if (p->type == TypeBool) {
                if (value)
                  strValue = juce::String("true");
                else
                  strValue = juce::String("false");
            }
            else {
                strValue = juce::String(value);
            }
        }
        
        // old mobius uses dim yellow
        g.setColour(juce::Colours::beige);
        g.drawText(juce::String(p->getDisplayableName()),
                   0, rowTop, maxNameWidth, ParametersRowHeight,
                   juce::Justification::centredRight);

        g.setColour(juce::Colours::yellow);
        g.drawText(strValue,
                   maxNameWidth + ParametersHorizontalGap, rowTop,
                   ParametersValueWidth, ParametersRowHeight,
                   juce::Justification::centredLeft);

        rowTop = rowTop + ParametersRowHeight + ParametersVerticalGap;
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


/**
 * A component that displays a conigurable set of StatusElements
 * to display and control Mobius runtime state.
 *
 * Found at the in the center of MobiusDisplay.
 */

#pragma once

#include <JuceHeader.h>

#include "ModeElement.h"
#include "BeatersElement.h"
#include "LoopMeterElement.h"
#include "CounterElement.h"
#include "FloatingStripElement.h"

class StatusArea : public juce::Component
{
  public:
    
    StatusArea(class MobiusDisplay*);
    ~StatusArea();

    void configure(class UIConfig* config);
    bool saveConfiguration(class UIConfig* config);
    
    void update(class MobiusState* state);
    void doAction(class UIAction* action);
    
    // element callback to save location changes after dragging
    void saveLocation(StatusElement* e);

    void resized() override;
    void paint(juce::Graphics& g) override;
    
  private:

    class MobiusDisplay* display;
    juce::Array<StatusElement*> elements;
    
    ModeElement mode {this};
    BeatersElement beaters {this};
    LoopMeterElement meter {this};
    CounterElement counter {this};
    FloatingStripElement floater {this};
    
    void addElement(StatusElement* el);
    void addMissing(StatusElement* el);
    
    void drawText(juce::Graphics& g, const char* text, int x, int y);
    
};


    

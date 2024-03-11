/**
 * Basic level meter
 */

#pragma once

#include <JuceHeader.h>

#include "../../model/MobiusState.h"
#include "../../Supervisor.h"

#include "StatusElement.h"

class LayerElement : public StatusElement, public Supervisor::ActionListener
{
  public:
    
    LayerElement(class StatusArea* area);
    ~LayerElement();

    void configure(class UIConfig* config) override;
    void update(class MobiusState* state) override;
    int getPreferredWidth() override;
    int getPreferredHeight() override;

    void resized() override;
    void paint(juce::Graphics& g) override;

    bool doAction(class UIAction* a);
    
  private:

    void orientShit();
    void initTestLoop();

    int lastTrack = 0;
    int lastLoop = 0;
    int lastLayerCount = 0;
    int lastLostCount = 0;
    bool lastCheckpoint = false;
    
    // the loop to display on the next paint()
    MobiusLoopState* sourceLoop = nullptr;

    // redirected loop state for testing
    MobiusLoopState testLoop;
    bool doTest = false;
};

/**
 * Helper class to iterate over a logical layer list
 * pulling state from the physical MobiusState model
 *
 * You know, if we made this an inner class we could
 * dispense with the state passing.
 */
class LayerCursor
{
  public:

    LayerCursor(MobiusLoopState* source) {
        loop = source;
    }
    ~LayerCursor() {}

    // set the position within the logical model
    void orient();

    // what we can't display
    int getUndoLoss();
    int getRedoLoss();

    // characteristics of the layer associated with a visible bar
    bool isVoid(int bar);
    bool isGhost(int bar);
    bool isActive(int bar);
    bool isCheckpoint(int bar);
    MobiusLayerState* getState(int bar);

  private:

    int ghostStart = 0;
    int undoStart = 0;
    int activeIndex = 0;
    int redoStart = 0;
    int redoGhostStart = 0;
    int voidStart = 0;
    
    // position within the logical layer list of the first bar in the display
    int viewBase = 0;

    int undoLoss = 0;
    int redoLoss = 0;
    
    // source model
    MobiusLoopState* loop;
};
    

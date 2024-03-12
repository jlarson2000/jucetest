/**
 * Simple layer list.
 * 
 */

#include <JuceHeader.h>

#include "../../util/Trace.h"
#include "../../model/UIConfig.h"
#include "../../model/MobiusState.h"
#include "../../model/ModeDefinition.h"
#include "../../model/UIAction.h"
#include "../../Supervisor.h"

#include "Colors.h"
#include "StatusArea.h"
#include "LayerElement.h"

const int LayerBarHeight = 40;
const int LayerBarWidth = 10;
const int LayerBarMax = 20;
const int LayerBarGap = 2;
const int LayerInset = 2;
const int LayerLossHeight = 12;

LayerElement::LayerElement(StatusArea* area) :
    StatusElement(area, "LayerElement")
{
    testLoop.init();
    // sigh, expected to be non-null which it would ordinally
    // be in the period before the first call to update()
    sourceLoop = &testLoop;

    Supervisor* s = Supervisor::Instance;
    s->addActionListener(this);
}

LayerElement::~LayerElement()
{
    Supervisor* s = Supervisor::Instance;
    s->removeActionListener(this);
}

void LayerElement::configure(UIConfig* config)
{
    // todo: could adjust the diameter
}

int LayerElement::getPreferredHeight()
{
    return LayerBarHeight + (LayerInset * 2);
}

int LayerElement::getPreferredWidth()
{
    return (LayerBarMax * LayerBarWidth) +
        ((LayerBarMax - 1) * LayerBarGap) +
        (LayerInset * 2);
}

void LayerElement::resized()
{
}

/**
 * Making the assumption that MobiusState is stable
 * and will live until the call to paint() so we don't
 * have to make a full structure copy.  In theory any
 * display decisions we make here could have changed
 * by the time paint() happens but any anomolies
 * wouldn't last long.
 *
 * For change detection need at minimum look at:
 *    activeTrack, activeLoop, layerCount, lostLayers
 *
 * Redo counts can't change without also changing layer counts.
 * Example from layerCount=10 the active layer is always
 * index 9.  If you Undo, layerCount drops to 9 and
 * redoCount increases by 1.  
 *
 * You can't create more redo layers without "moving" the
 * active layer.  You can in theory reduce the redoCount
 * though an action that prunes them but we don't have that yet.
 * If we ever do, then will have to include redo counts in
 * refresh detection.
 *
 * Checkpoint state can only change in what was previously the
 * active layer.  You can't randomly toggle checkpoint status
 * on other layers.  So while each layer has a checpointed flag,
 * we only need to remember the state of the last active one.
 *
 * Making a further assumption that MobiusState is stable
 * indefintely and will live across calls to update, so we can
 * detect track/loop changes simply by comparing the MobiusLoopState
 * pointer we used the last time.
 *
 * Ugh, lostLayers and lostRedo have to factor into this too.
 * Assuming a display model that looks like this:
 *
 *      .....X....
 *
 * With lostLayers on the left of 1 and lostRedo on the right of 1.
 * If you undo again, and favor putting the active layer in the center
 * the numers on the edges change from 0 to 2.  Actually no, that would
 * change the layer count.  We would end up displaying the same bars
 * but the lost numbers would change.
 * 
 */
void LayerElement::update(MobiusState* state)
{
    MobiusTrackState* track = &(state->tracks[state->activeTrack]);
    MobiusLoopState* loop = &(track->loops[track->activeLoop]);

    if (doTest) {
        // substitute our test loop
        loop = &testLoop;
    }

    // don't need to save lastTrack and lastLoop if we just
    // saved the last MobiusLoopState pointer
    bool needsRepaint = (lastTrack != state->activeTrack ||
                         lastLoop != track->activeLoop ||
                         lastLayerCount != loop->layerCount ||
                         lastLostCount != loop->lostLayers);

    // checkpoint is a little harder
    bool newCheckpoint = false;
    int active = loop->layerCount - 1;
    if (active > 0) {
        newCheckpoint = loop->layers[active].checkpoint;
        if (lastCheckpoint != newCheckpoint)
          needsRepaint = true;
    }

    if (needsRepaint) {
        lastTrack = state->activeTrack;
        lastLoop = track->activeLoop;
        lastLayerCount = loop->layerCount;
        lastLostCount = loop->lostLayers;
        lastCheckpoint = newCheckpoint;
        
        // remember the full state for paint
        sourceLoop = loop;
        repaint();
    }
}

/**
 * If we override paint, does that mean we control painting
 * the children, or is that going to cascade?
 */
void LayerElement::paint(juce::Graphics& g)
{
    // borders, labels, etc.
    StatusElement::paint(g);

    // figure out where to start and how much loss
    LayerCursor lc(sourceLoop);
    lc.orient();

    int preLoss = lc.getUndoLoss();
    if (preLoss > 0) {
        g.setFont(juce::Font(LayerLossHeight));
        g.setColour(juce::Colours::white);
        g.drawText(juce::String(preLoss),
                   LayerInset, LayerInset,
                   30, LayerLossHeight,
                   juce::Justification::left);
    }

    int postLoss = lc.getRedoLoss();
    if (postLoss) {
        g.setFont(juce::Font(LayerLossHeight));
        g.setColour(juce::Colours::white);
        g.drawText(juce::String(postLoss),
                   getWidth() - LayerInset - 30, LayerInset,
                   30, LayerLossHeight, 
                   juce::Justification::right);
    }

    int barLeft = LayerInset;
    int barTop = LayerInset + LayerLossHeight;

    for (int i = 0 ; i < LayerBarMax ; i++) {

        if (lc.isCheckpoint(i))
          g.setColour(juce::Colours::red);
        else 
          g.setColour(juce::Colours::grey);

        g.drawRect(barLeft, barTop, LayerBarWidth, LayerBarHeight);

        if (!lc.isVoid(i)) {
            if (lc.isGhost(i)) {
                g.setColour(juce::Colours::lightblue);
            }
            else if (lc.isActive(i))
              g.setColour(juce::Colours::yellow);
            else
              g.setColour(juce::Colours::yellow.darker());
            
            g.fillRect(barLeft+1, barTop+1, LayerBarWidth-2, LayerBarHeight-2);
        }
        barLeft += LayerBarWidth + LayerBarGap;
    }
}

//////////////////////////////////////////////////////////////////////
// LayerCursor
//////////////////////////////////////////////////////////////////////

/**
 * The hard part.  Calculate various regions of the virtual layer list
 * and determine the ideal starting point for the visible bars.
 *
 * A loop at runtime will have a number of layers.  These are divided into
 * three categories:
 *
 *     active - the layer being played
 *     undo   - layers created before the active layer
 *     redo   - layers created after the active layer
 *
 * Within the full set of undo and redo layers, the active layer can move
 * back and forth among them.
 *
 * For display purposes we have these conceptual areas:
 * 
 *   void - an area that has no layers
 *   ghost - an area that has layers, but were not included in MobiusState
 *   physical - an area that has layers, that were included in MobiusState
 *
 * Think of physical layers being in the center divided between undo and redo.
 * They have a corresponding MobiusLayerState.
 *
 * Physical layers are surrounded on both sides by ghost or lost layers.  These
 * are real layers in the Loop/Layer model but could not be included in the
 * MobiusState due to space constraints.
 *
 * Void is the area outside of ghost layers that have no correspondence
 * to the Loop/Layer model.  Void is encountered if the display is larger than
 * what is needed to display all of the physical and ghost layers.
 *
 * The math here could be compressed and optimized to a degree, but you lose
 * clarity over what the numbers mean.
 */ 
void LayerCursor::orient()
{
    // offsets into the regions of the logical layer list
    ghostStart = 0;
    undoStart = loop->lostLayers;
    activeIndex = undoStart + loop->layerCount - 1;
    redoStart = activeIndex + 1;
    redoGhostStart = redoStart + loop->redoCount;
    voidStart = redoGhostStart + loop->lostRedo;

    // viewBase has the base index of the visible view into
    // the logical layer list, it is initialized to zero
    // but may have changed if we had to scroll to keep active visible
    // retain the last view base if possible so the user sees
    // movemeent in the active layer rather than recentering and
    // seeing the layers shift aroundd the active layer.
    //viewBase = 0;

    // adjust viewBase to make activeIndex visible
    int viewLast = viewBase + LayerBarMax - 1;
    if (activeIndex <= viewLast) {
        // active fits, leave the base
    }
    else {
        // "scroll" to put activeIndex within the view
        int center = floor((float)LayerBarMax / 2.0f);
        viewBase = activeIndex - center;
        viewLast = viewBase + LayerBarMax - 1;
    }

    undoLoss = 0;
    if (viewBase > ghostStart)
      undoLoss = viewBase - ghostStart;

    redoLoss = 0;
    if (loop->redoCount > 0) {
        int redoLast = redoStart + loop->redoCount - 1;
        if (redoLast > viewLast)
          redoLoss = redoLast - viewLast;
    }
}

int LayerCursor::getUndoLoss()
{
    return undoLoss;
}

int LayerCursor::getRedoLoss()
{
    return redoLoss;
}

/**
 * Return the LayerState for the layer corresponding to this bar.
 * If the index points to a void or ghosted layer, nullptr is returned.
 */
MobiusLayerState* LayerCursor::getState(int bar)
{
    MobiusLayerState* layer = nullptr;

    int virtualIndex = viewBase + bar;

    if (virtualIndex >= undoStart) {
        if (virtualIndex < redoStart) {
            int structureIndex = virtualIndex - undoStart;
            if (structureIndex < 0 || structureIndex >= loop->layerCount)
              Trace(1, "LayerCursor says Jeff is bad at math\n");
            else 
              layer = &(loop->layers[structureIndex]);
        }
        else if (virtualIndex < redoGhostStart) {
            int structureIndex = virtualIndex - redoStart;
            if (structureIndex < 0 || structureIndex >= loop->redoCount)
              Trace(1, "LayerCursor says Jeff is bad at math\n");
            else
              layer = &(loop->redoLayers[structureIndex]);
        }
    }
    return layer;
}

/**
 * Return true if the bar at this position is over a layer that
 * does not exist.
 */
bool LayerCursor::isVoid(int bar)
{
    bool voided = false;
    
    int virtualIndex = viewBase + bar;

    // void on the left would only be true if we allowed the display to
    // slide past the left edge to center or right justify the active layer
    if (virtualIndex < 0) {
        // base index too far into left space
        voided = true;
    }
    else if (virtualIndex >= voidStart) {
        // ran off the end on the right
        voided = true;
    }

    return voided;
}
    
/**
 * Return true if the bar at this position is over a layer that
 * exists, but could not be included in the MobiusState model.
 * i.e. it is a "lost" layer for either undo or redo.
 */
bool LayerCursor::isGhost(int bar)
{
    bool ghost = false;
    
    int virtualIndex = viewBase + bar;
    
    if (virtualIndex >= ghostStart) {
        if (virtualIndex < undoStart) {
            // undo ghost on the left
            ghost = true;
        }
        else if (virtualIndex >= redoGhostStart && virtualIndex < voidStart) {
            // redo ghost on the right
            ghost = true;
        }
    }
    
    return ghost;
}

/**
 * Return true if the bar at this position is over the active layer
 */
bool LayerCursor::isActive(int bar)
{
    int virtualIndex = viewBase + bar;
    return (virtualIndex == activeIndex);
}

bool LayerCursor::isCheckpoint(int bar)
{
    bool checkpoint = false;
    MobiusLayerState* layer = getState(bar);
    if (layer != nullptr)
      checkpoint = layer->checkpoint;
    return checkpoint;
}

//////////////////////////////////////////////////////////////////////
// Test Actions
//////////////////////////////////////////////////////////////////////

bool LayerElement::doAction(UIAction* action)
{
    bool handled = false;

    // override MobiusStateMaxLayers and MaxRedo to simulate
    // constraints
    int maxLayers = 5;
    int maxRedo = 5;

    // the number of physical layers the starting state thinks we have
    // this may not make sense if loss would have actually fit witin
    // the available slots, will be adjusted later
    int actualLayers = testLoop.layerCount + testLoop.lostLayers;
    int actualRedos = testLoop.redoCount + testLoop.lostRedo;

    if (strcmp(action->actionName, "Undo") == 0) {
        Trace(1, "LayerElement: Undo\n");
        if (actualLayers > 1) {
            actualLayers--;
            actualRedos++;
        }
        handled = true;
    }
    else if (strcmp(action->actionName, "Redo") == 0) {
        Trace(1, "LayerElement: Redo\n");
        if (actualRedos > 0) {
            actualLayers++;
            actualRedos--;
        }
        handled = true;
    }
    else if (strcmp(action->actionName, "Coverage") == 0) {
        Trace(1, "LayerElement: Init\n");
        initTestLoop();
        actualLayers = testLoop.layerCount + testLoop.lostLayers;
        actualRedos = testLoop.redoCount + testLoop.lostRedo;
        handled = true;
    }

    else if (strcmp(action->actionName, "Debug") == 0) {
        Trace(1, "LayerElement: Add layer\n");
        // pretend we have a new layer
        actualLayers++;
        handled = true;
    }

    // whatever the beginning values initTestLoop left there,
    // recalculate loss based on the actual size and the state constraints

    if (actualLayers > maxLayers) {
        testLoop.layerCount = maxLayers;
        testLoop.lostLayers = actualLayers - maxLayers;
    }
    else {
        testLoop.layerCount = actualLayers;
        testLoop.lostLayers = 0;
    }

    if (actualRedos > maxRedo) {
        testLoop.redoCount = maxRedo;
        testLoop.lostRedo = actualRedos - maxRedo;
    }
    else {
        testLoop.redoCount = actualRedos;
        testLoop.lostRedo = 0;
    }

    return handled;
}

void LayerElement::initTestLoop()
{
    testLoop.init();

    // mock up some layer state
    testLoop.layerCount = 3;
    testLoop.redoCount = 3;

    // sigh, expected to be non-null which it would ordinally
    // be in the period before the first call to update()
    sourceLoop = &testLoop;
    doTest = true;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

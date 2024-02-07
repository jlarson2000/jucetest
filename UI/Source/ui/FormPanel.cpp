/**
 * A collection of FieldGrids that may be contained with a Form tab.
 *
 * Grids are expected to be heap allocated and ownership transfers
 * to the FormPanel.
 *
 * Normally there will only be one FieldGrid, if there is more than one
 * it is layed out vertically.
 * The panel name becomes the tab name if there is more than one
 * panel in a form.
 */

#include <JuceHeader.h>
#include "FormPanel.h"

FormPanel::FormPanel()
{
    setName("FormPanel");
}

FormPanel::FormPanel(juce::String argTabName)
{
    setName("FormPanel");
    tabName = argTabName;
}

void FormPanel::addGrid(FieldGrid* grid)
{
    grids.add(grid);
    addAndMakeVisible(grid);
}

FieldGrid* FormPanel::getGrid(int index)
{
    // juce::Array is supposed to deal with out of range indexes
    return grids[index];
}

void FormPanel::gatherFields(juce::Array<Field*>& fields)
{
    for (int i = 0 ; i < grids.size() ; i++) {
        FieldGrid* grid = grids[i];
        grid->gatherFields(fields);
    }
}

//////////////////////////////////////////////////////////////////////
//
// Rendering and Layout
//
//////////////////////////////////////////////////////////////////////

/**
 * No special rendering, cascade ot the grids.
 * Calculate initial minimum size.
 */
void FormPanel::render()
{
    // make sure the grids are rendered
    for (int i = 0 ; i < grids.size() ; i++) {
        FieldGrid* grid = grids[i];
        grid->render();
    }
    
    juce::Rectangle<int> size = getMinimumSize();
    setSize(size.getWidth(), size.getHeight());
}

juce::Rectangle<int> FormPanel::getMinimumSize()
{
    int maxWidth = 0;
    int maxHeight = 0;
    
    for (int i = 0 ; i < grids.size() ; i++) {
        FieldGrid* grid = grids[i];
        if (grid->getWidth() > maxWidth)
          maxWidth = grid->getWidth();
        maxHeight += grid->getHeight();
    }

    return juce::Rectangle<int> {0, 0, maxWidth, maxHeight};
}

/**
 * Should only have one grid but if we have more than
 * one assume they stack.
 *
 * We often have a larger container when we're in a tabbed component
 * so center.
 */
void FormPanel::resized()
{
    int gridOffset = 0;

    int maxHeight = 0;
    for (int i = 0 ; i < grids.size() ; i++) {
        FieldGrid* grid = grids[i];
        maxHeight += grid->getHeight();
    }

    gridOffset = (getHeight() - maxHeight) / 2;

    for (int i = 0 ; i < grids.size() ; i++) {
        FieldGrid* grid = grids[i];
        int centerOffset = (getWidth() - grid->getWidth()) / 2;
        grid->setTopLeftPosition(centerOffset, gridOffset);
        gridOffset += grid->getHeight();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

        
    

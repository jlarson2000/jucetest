
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
    addChildComponent(grid);
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

void FormPanel::render()
{
    for (int i = 0 ; i < grids.size() ; i++) {
        FieldGrid* grid = grids[i];
        grid->render();
    }
    autoSize();
}

    
/**
 * Should only have one grid but if we have more than
 * one assume they stack.
 */
void FormPanel::autoSize()
{
    int maxWidth = 0;
    int maxHeight = 0;
    
    for (int i = 0 ; i < grids.size() ; i++) {
        FieldGrid* grid = grids[i];
        grid->autoSize();
        if (grid->getWidth() > maxWidth)
          maxWidth = grid->getWidth();
        maxHeight += grid->getHeight();
    }

    setSize(maxWidth, maxHeight);
}

void FormPanel::resized()
{
    int gridOffset = 0;
    
    for (int i = 0 ; i < grids.size() ; i++) {
        FieldGrid* grid = grids[i];
        grid->setTopLeftPosition(0, gridOffset);
        gridOffset += grid->getHeight();
    }
}

        
    

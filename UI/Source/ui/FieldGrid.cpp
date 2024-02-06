/**
 * A field grid is a set of Fields that can be arranged in columns.
 *
 */

#include <JuceHeader.h>

#include "Field.h"
#include "FieldGrid.h"

//////////////////////////////////////////////////////////////////////
//
// FieldGrid
//
//////////////////////////////////////////////////////////////////////

FieldGrid::FieldGrid()
{
    setName("FieldGrid");
}

FieldGrid::~FieldGrid()
{
}

void FieldGrid::add(Field* f, int column)
{
    juce::OwnedArray<Field>* fieldColumn = nullptr;
    
    if (column >= columns.size()) {
        fieldColumn = new juce::OwnedArray<Field>();
        columns.set(column, fieldColumn);
    }
    else {
        fieldColumn = columns[column];
    }

    fieldColumn->add(f);
}

/**
 * Add pointers to the Fields we contain to the Array.
 * This is used in cases where something needs to iterate
 * over all the Fields within a container hierarchy.
 *
 * This little traversal happens three times now.  If we keep
 * doing this either define a FieldIterator or better might
 * be just to maintain them on a single list, put the column on the Field
 * and have render() figure it out.
 */
void FieldGrid::gatherFields(juce::Array<Field*>& fields)
{
    for (int col = 0 ; col < columns.size() ; col++) {
        juce::OwnedArray<Field>* column = columns[col];
        if (column != nullptr) {
            for (int row = 0 ; row < column->size() ; row++) {
                Field* f = (*column)[row];
                fields.add(f);
            }
        }
    }
}

/**
 * Iterate over the fields we contain and render them as Juce components.
 * This happens twice now, does it make sense to define a FieldIterator?
 */
void FieldGrid::render()
{
    for (int col = 0 ; col < columns.size() ; col++) {
        juce::OwnedArray<Field>* column = columns[col];
        if (column != nullptr) {
            for (int row = 0 ; row < column->size() ; row++) {
                Field* f = (*column)[row];
                f->render();
                addAndMakeVisible(f);
            }
        }
    }
    autoSize();
}

/**
 * Calculate the minimum size for this grid.
 * TODO: Start using Panel here
 * Hmm, may as well do layout here too rather than just size...think
 */
void FieldGrid::autoSize()
{
    int maxWidth = 0;
    int maxHeight = 0;
    
    for (int col = 0 ; col < columns.size() ; col++) {
        juce::OwnedArray<Field>* column = columns[col];
        if (column != nullptr) {
            int colWidth = 0;
            int colHeight = 0;
            for (int row = 0 ; row < column->size() ; row++) {
                Field* f = (*column)[row];
                colHeight += f->getHeight();
                if (f->getWidth() > colWidth)
                  colWidth = f->getWidth();
            }
            maxWidth += colWidth;
            if (colHeight > maxHeight)
              maxHeight = colHeight;
        }
    }

    setSize(maxWidth, maxHeight);
}

/**
 * TODO: Would like each grid to auto-size the label
 * column so we don't have to hard wire it.
 *
 * TODO: Might as well combine positining with autoSize
 * since they do about the same thing.
 */
void FieldGrid::resized()
{
    int colOffset = 0;
    
    for (int col = 0 ; col < columns.size() ; col++) {
        juce::OwnedArray<Field>* column = columns[col];
        int maxWidth = 0;
        if (column != nullptr) {
            int rowOffset = 0;
            for (int row = 0 ; row < column->size() ; row++) {
                Field* f = (*column)[row];
                f->setTopLeftPosition(colOffset, rowOffset);
                rowOffset += f->getHeight();
                if (f->getWidth() > maxWidth)
                  maxWidth = f->getWidth();
            }
        }
        colOffset += maxWidth;
    }
}

void FieldGrid::paint(juce::Graphics& g)
{
    // give it an obvious background
    // need to work out  borders
    g.fillAll (juce::Colours::beige);
}


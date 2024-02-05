
#include <string>
#include <sstream>

#include <JuceHeader.h>

#include "Form.h"
#include "../util/qtrace.h"


Form::Form() :
    tabs {juce::TabbedButtonBar::Orientation::TabsAtTop}
{
    setName("Form");

    // adjust tab bar colors
    juce::TabbedButtonBar& bar = tabs.getTabbedButtonBar();
    bar.setColour(juce::TabbedButtonBar::ColourIds::tabTextColourId, juce::Colours::grey);
    bar.setColour(juce::TabbedButtonBar::ColourIds::frontTextColourId, juce::Colours::white);
            
    // add space between the bar and the grid
    // ugh, this leaves a border around it with the background color
    // may be better to twiddle with the content component?
    // doesn't really help because the first component is still adjacent to the
    // indent color 
    tabs.setIndent(4);
    
}

Form::~Form()
{
}

/**
 * Convenience method for most configuration forms that use
 * parameter fields
 */
void Form::add(const char* tab, Parameter* p, int column)
{
    add(new ParameterField(p), tab, column);
}

/**
 * Called during form rendering to add a field to a grid/tab
 * at the specified column.
 */
void Form::add(Field* f, const char* tab, int column)
{
    FieldGrid* grid = nullptr;
    
    if (tab != nullptr) {
        for (int i = 0 ; i < grids.size() ; i++) {
            FieldGrid* g = grids[i];
            if (g->getName() == tab) {
                grid = g;
                break;
            }
        }
    }
    else {
        // just dump them into the first tab
        if (grids.size() > 0)
          grid = grids[0];
    }

    // boostrap one if the tab is empty
    if (grid == nullptr) {
        grid = new FieldGrid();
        if (tab != nullptr)
          grid->setName(tab);
        else
          grid->setName("???");
        grids.add(grid);
    }

    grid->add(f, column);
}

/**
 * If there is more than one grid, the grids are rendered
 * inside a TabbedComponent which will manage visibility.
 * Otherwise the single grid is added directly as a child
 * component of the form so we don't see a tab button bar.
 */
void Form::render()
{
    for (int i = 0 ; i < grids.size() ; i++) {
        FieldGrid* grid = grids[i];
        grid->render();
    }

    if (grids.size() > 0) {
        // only add a tab bar if we have more than one
        if (grids.size() > 1) {
            for (int i = 0 ; i < grids.size() ; i++) {
                FieldGrid* grid = grids[i];
                // last flag is deleteComponentWhenNotNeeded
                // we manage deletion of the grids
                tabs.addTab(juce::String(grid->getName()), juce::Colours::black, grid, false);
            }
            // if for some reason you want something other than
            // the first tab selected you can call tis
            // tabs.setCurrentTabIndex(index, false);
            addChildComponent(tabs);
            tabs.setVisible(true);
        }
        else {
            FieldGrid* first = grids[0];
            addChildComponent(first);
            first->setVisible(true);
        }
    }
}

/**
 * Give all the tab grids the full size.
 * Need to factor out the tab buttons.
 */
void Form::resized()
{
    if (grids.size() > 1) {
        // we used a tab component
        tabs.setSize(getWidth(), getHeight());
    }
    else if (grids.size() > 0) {
        // we have a directd grid child
        FieldGrid* first = grids[0];
        first->setSize(getWidth(), getHeight());
    }
}

void Form::paint(juce::Graphics& g)
{
    g.fillAll (juce::Colours::beige);
}

//////////////////////////////////////////////////////////////////////
//
// FormIterator
//
//////////////////////////////////////////////////////////////////////

Form::Iterator::Iterator(Form* argForm)
{
    form = argForm;
    advance();
}

Form::Iterator::~Iterator()
{
}

void Form::Iterator::reset()
{
    tabIndex = 0;
    colIndex = 0;
    fieldIndex = 0;
    nextField = nullptr;
    advance();
}

bool Form::Iterator::hasNext()
{
    return (nextField != nullptr);
}

Field* Form::Iterator::next()
{
    Field* retval = nextField;
    advance();
    return retval;
}

void Form::Iterator::advance()
{
    nextField = nullptr;
    
    while (nextField == nullptr && tabIndex < form->grids.size()) {
        FieldGrid* grid = form->grids[tabIndex];
        while (nextField == nullptr && colIndex < grid->getColumns()) {
            juce::OwnedArray<Field>* fields = grid->getColumn(colIndex);
            // should not have nulls in here but this is harder to enforce
            if (fields != nullptr && fieldIndex < fields->size()) {
                nextField = (*fields)[fieldIndex];
                fieldIndex++;
            }
            else {
                colIndex++;
                fieldIndex = 0;
            }
        }
        if (nextField == nullptr) {
            tabIndex++;
            colIndex = 0;
            fieldIndex = 0;
        }
    }
}    


#include <JuceHeader.h>

#include "../../util/Trace.h"
#include "../../util/Util.h"
#include "../../util/MidiUtil.h"
#include "../../model/Binding.h"
#include "../common/ButtonBar.h"

#include "BindingTable.h"

// should put this inside the class but we've got
// the "static member with in-class initializer must have non-volatile
// const integral type or be line" folderol to figure out
const char* NewBindingName = "[New]";

BindingTable::BindingTable()
{
    setName("BindingTable");

    initTable();
    addAndMakeVisible(table);

    commands.add("New");
    commands.add("Update");
    commands.add("Delete");
    commands.autoSize();
    commands.addListener(this);

    addAndMakeVisible(commands);
}

BindingTable::~BindingTable()
{
}

/**
 * Populate internal state with a list of Bindings
 * from a configuration object.
 * The list is copied and ownership is retained by the caller.
 */

void BindingTable::setBindings(class Binding* src)
{
    bindings.clear();
    while (src != nullptr) {
        Binding* copy = new Binding(src);
        bindings.add(copy);
    }

    // should only be doing this once but I suppose we could
    // trigger a repaint if it comes in later
    table.updateContent();
}

void BindingTable::add(Binding* src)
{
    bindings.add(new Binding(src));
}

void BindingTable::updateContent()
{
    table.updateContent();
}

/**
 * Returns the list of Bindings that have been modified
 * and clears internal state.  Ownership of the list passes
 * to the caller.
 */
Binding* BindingTable::captureBindings()
{
    Binding* capture = nullptr;
    Binding* last = nullptr;

    trace("BindingTable::capture %d\n", bindings.size());

    while (bindings.size() > 0) {
        Binding* b = bindings.removeAndReturn(0);
        // clearing lingering chain pointer for cascaded delete
        b->setNext(nullptr);
        trace("%s\n", b->getActionName());
        // filter out uninitialized rows
        if (StringEqual(b->getActionName(), NewBindingName)) {
            trace("filtering %s\n", b->getActionName());
            delete b;
        }
        else {
            if (last == nullptr)
              capture = b;
            else
              last->setNext(b);
            last = b;
        }
    }
    
    table.updateContent();

    return capture;
}

/**
 * Delete contained Bindings and prepare for renewal.
 */
void BindingTable::clear()
{
    bindings.clear();
    table.updateContent();
}

bool BindingTable::isNew(Binding* b)
{
    return StringEqual(b->getActionName(), NewBindingName);
}

//////////////////////////////////////////////////////////////////////
//
// Layout
//
//////////////////////////////////////////////////////////////////////

/**
 * Set starting table properties
 */
void BindingTable::initTable()
{
    // from the example
    table.setColour (juce::ListBox::outlineColourId, juce::Colours::grey);      // [2]
    table.setOutlineThickness (1);

    // usually want this off but I guess could support for multiple deletes?
    table.setMultipleSelectionEnabled (false);
    // any reason not to want this?
    // only relevant if multi selection enabled
    table.setClickingTogglesRowSelection(true);

    // unclear what the defaults are here, 
    // The default row height from ListBox is 22
    // Interesting, I don't think rows will squish based on the
    // overall table size.  Unless I guess if you change them
    // in response to resized() 
    table.setHeaderHeight(22);
    table.setRowHeight(22);

    initColumns();
}

/**
 * Set the column titles and initial widths.
 * Column Ids must start from 1 and must be unique.
 *
 * NOTE: Assuming this can only be called once, really
 * should clear the current header and start over if called again.
 *
 * Pick some reasonable default widths but need to be smarter
 */
void BindingTable::initColumns()
{
    juce::TableHeaderComponent& header = table.getHeader();

    targetColumn = 1;
    triggerColumn = 2;
    argumentsColumn = 3;
    scopeColumn = 4;

    // columnId, width, minWidth, maxWidth, propertyFlags, insertIndex
    // minWidth defaults to 30
    // maxWidth to -1
    // propertyFlags=defaultFlags
    // insertIndex=-1
    // propertyFlags has various options for visibility, sorting, resizing, dragging
    // example used 1 based column ids, is that necessary?

    header.addColumn(juce::String("Target"), targetColumn,
                     100, 30, -1,
                     juce::TableHeaderComponent::defaultFlags);
                     
    // trigger is optional for buttons
    if (!noTrigger) {
        header.addColumn(juce::String("Trigger"), triggerColumn,
                         100, 30, -1,
                         juce::TableHeaderComponent::defaultFlags);
        
    }

    header.addColumn(juce::String("Arguments"), argumentsColumn,
                     100, 30, -1,
                     juce::TableHeaderComponent::defaultFlags);
    
    header.addColumn(juce::String("Scope"), scopeColumn,
                     50, 30, -1,
                     juce::TableHeaderComponent::defaultFlags);

    //header.setSortColumnId (1, true);

    // unclear what the default header height is
    // the default row height from ListBox is 22
}

const int CommandButtonGap = 10;

int BindingTable::getPreferredWidth()
{
    // adapt to column configuration
    return 500;
}

int BindingTable::getPreferredHeight()
{
    int height = 400;
    
    // gap between buttons
    height += CommandButtonGap;
    
    commands.autoSize();
    height += commands.getHeight();

    return height;
}

/**
 * Always put buttons at the bottom, and let the table
 * be as large as it wants.
 */
void BindingTable::resized()
{
    juce::Rectangle<int> area = getLocalBounds();

    commands.setBounds(area.removeFromBottom(commands.getHeight()));
    area.removeFromBottom(CommandButtonGap);

    table.setBounds(area);
}    

//////////////////////////////////////////////////////////////////////
//
// Command Buttons
//
//////////////////////////////////////////////////////////////////////

/**
 * ButtonBar::Listener
 */
void BindingTable::buttonClicked(juce::String name)
{
    // is this the best way to compare them?
    if (name == juce::String("New")) {
        if (listener != nullptr) {
            Binding* neu = listener->bindingNew();
            if (neu == nullptr) {
                // generate a placeholder
                neu = new Binding();
                neu->setActionName(NewBindingName);
            }
            
            bindings.add(neu);
            table.updateContent();
            // select it, it will be the last
            table.selectRow(bindings.size() - 1);
        }
    }
    else if (name == juce::String("Update")) {
        int row = table.getSelectedRow();
        if (row >= 0) {
            Binding* b = bindings[row];
            if (listener != nullptr) {
                // listener updates the binding but we retain ownership
                listener->bindingUpdate(b);
                table.updateContent();
                // had to add this, why?  I guess just changing the
                // model without altering the row count doesn't trigger it
                table.repaint();
            }
        }
    }
    else if (name == juce::String("Delete")) {
        int row = table.getSelectedRow();
        if (row >= 0) {
            Binding* b = bindings[row];
            if (listener != nullptr) {
                // listener is allowed to respond, but it does not take
                // ownership of the Binding
                listener->bindingDelete(b);
            }
            bindings.remove(row);
            table.updateContent();
            // auto-select the one after it?
        }
    }
}

//////////////////////////////////////////////////////////////////////
//
// Table Cell Rendering
//
//////////////////////////////////////////////////////////////////////

/**
 * Callback from TableListBoxModel to derive the text to paint in this cell.
 * Row is zero based columnId is 1 based and is NOT a column index, you have
 * to map it to the logical column if allowing column reording in the table.
 *
 */
juce::String BindingTable::getCellText(int row, int columnId)
{
    juce::String cell;
    Binding* b = bindings[row];
    
    if (columnId == targetColumn) {
        // start with just the name and add prefixes when necessary
        cell = juce::String(b->getActionName());
    }
    else if (columnId == triggerColumn) {
        if (listener != nullptr)
          cell = listener->renderTriggerCell(b);
        else
          cell = "???";
    }
    else if (columnId == scopeColumn) {
        // BindingPanel should probably render this
        cell = formatScopeText(b);
    }
    else if (columnId == argumentsColumn) {
        cell = juce::String(b->getArguments());
    }

    return cell;
}

/**
 * I think the old way stored these as text and they were
 * parsed at runtime into the mTrack and mGroup numbers
 * Need a lot more here as we refine what scopes mean.
 */ 
juce::String BindingTable::formatScopeText(Binding* b)
{
    if (b->getScope() == nullptr)
      return juce::String("Global");
    else
      return juce::String(b->getScope());
}

//////////////////////////////////////////////////////////////////////
//
// TableListBoxModel
//
//////////////////////////////////////////////////////////////////////

/**
 * The maximum of all column rows.
 * This is independent of the table size.
 */
int BindingTable::getNumRows()
{
    return bindings.size();
}

/**
 * Taken from the example to show alternate row backgrounds.
 * Colors look reasonable, don't really need to mess with
 * LookAndFeel though.
 *
 * Graphics will be initialized to the size of the visible row.
 * Width and height are passed, I guess in case you want to do something
 * fancier than just filling the entire thing.  Could be useful
 * for borders, though Juce might provide something for selected rows/cells already.
 */
void BindingTable::paintRowBackground(juce::Graphics& g, int rowNumber,
                                      int /*width*/, int /*height*/,
                                      bool rowIsSelected)
{
    // I guess this makes an alternate color that is a variant of the existing background
    // color rather than having a hard coded unrelated color
    auto alternateColour = getLookAndFeel().findColour (juce::ListBox::backgroundColourId)
        .interpolatedWith (getLookAndFeel().findColour (juce::ListBox::textColourId), 0.03f);

    if (rowIsSelected) {
        g.fillAll (juce::Colours::lightblue);
    }
    else if (rowNumber % 2) {
        g.fillAll (alternateColour);
    }
}

/**
 * Based on the example.
 * If the row is selected it will have a light blue backgound and we'll paint the
 * text in dark blue.  Otherwise we use whatever the text color is set in the ListBox
 * 
 * Example had font hard coded as Font(14.0f) which is fine if you let the row heiught
 * default to 22 but ideally this should be proportional to the row height if it can be changed.
 * 14 is 63% of 22
 */
void BindingTable::paintCell(juce::Graphics& g, int rowNumber, int columnId,
                             int width, int height, bool rowIsSelected)
{
    g.setColour (rowIsSelected ? juce::Colours::darkblue : getLookAndFeel().findColour (juce::ListBox::textColourId));
    
    // how expensive is this, should we be caching it after the row height changes?
    g.setFont(juce::Font(height * .66));

    juce::String cell = getCellText(rowNumber, columnId);

    // again from the table example
    // x, y, width, height, justification, useEllipses
    // example gave it 2 on the left, I guess to give it a little padding next to the cell border
    // same on the right with the width reduction
    // height was expected to be tall enough
    // centeredLeft means "centered vertically but placed on the left hand side"
    g.drawText (cell, 2, 0, width - 4, height, juce::Justification::centredLeft, true);

    // this is odd, it fills a little rectangle on the right edge 1 pixel wide with
    // the background color, I'm guessing if the text is long enough, perhaps with elippses,
    // this erases part of to make it look better?
    //g.setColour (getLookAndFeel().findColour (juce::ListBox::backgroundColourId));
    //g.fillRect (width - 1, 0, 1, height);
}

/**
 * MouseEvent has various characters of the mouse click such as the actual x/y coordinate
 * offsetFromDragStart, numberOfClicks, etc.  Not interested in those right now.
 *
 * Can pass the row/col to the listener.
 * Can use ListBox::isRowSelected to get the selected row
 * Don't know if there is tracking of a selected column but we don't need that yet.
 */
void BindingTable::cellClicked(int rowNumber, int columnId, const juce::MouseEvent& event)
{
    if (listener != nullptr) {
        if (rowNumber >= bindings.size()) {
            trace("Binding row out of range! %d\n", rowNumber);
        }
        else {
            Binding* b = bindings[rowNumber];
            listener->bindingSelected(b);
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


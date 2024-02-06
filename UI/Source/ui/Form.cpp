/**
 * Component model for Mobius configuration forms.
 * 
 * A form consists of a list of FormPanels.  If there is more than
 * one panel a tabbed component is added to select the visible panel.
 *
 * Panels almost always contain a FieldGrid though they can have other things.
 *
 * In theory panels can contain more than one grid that we might like to
 * put a labeled group border around.  If we end up there will need
 * more complicate Field adder methods.
 */

#include <string>
#include <sstream>

#include <JuceHeader.h>

#include "ParameterField.h"
#include "FormPanel.h"
#include "Form.h"

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

void Form::add(FormPanel* panel)
{
    panels.add(panel);
}

/**
 * Called during form rendering to add a field to a panel/grid
 * at the specified column.
 *
 * Making assumptions right now that each panel can contain only
 * one grid.
 *
 */
void Form::add(Field* f, const char* tab, int column)
{
    FormPanel* panel = nullptr;

    if (tab == nullptr) {
        // simple form, no tabs
        if (panels.size() > 0)
          panel = panels[0];
    }
    else {
        // find panel by ame
        for (int i = 0 ; i < panels.size() ; i++) {
            FormPanel* p = panels[i];
            if (p->getTabName() == tab) {
                panel = p;
                break;
            }
        }
    }

    if (panel == nullptr) {
        // give it something to catch errors
        if (tab == nullptr) tab = "???";
        panel = new FormPanel(juce::String(tab));
        panels.add(panel);
    }
    
    // once we allow panels to have more than
    // one grid will need to name them
    FieldGrid* grid = panel->getGrid(0);
    if (grid == nullptr) {
        grid = new FieldGrid();
        panel->addGrid(grid);
    }

    grid->add(f, column);
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
 * Traverse the hierarchy to find all contained Fields.
 */
void Form::gatherFields(juce::Array<Field*>& fields)
{
    for (int i = 0 ; i < panels.size() ; i++){
        FormPanel* p = panels[i];
        if (p != nullptr)
          p->gatherFields(fields);
    }
}

/**
 * If there is more than one grid, the grids are rendered
 * inside a TabbedComponent which will manage visibility.
 * Otherwise the single grid is added directly as a child
 * component of the form so we don't see a tab button bar.
 */
void Form::render()
{
    for (int i = 0 ; i < panels.size() ; i++) {
        FormPanel* panel = panels[i];
        panel->render();
    }

    // only add a tab bar if we have more than one
    if (panels.size() > 1) {
        for (int i = 0 ; i < panels.size() ; i++) {
            FormPanel* panel = panels[i];
            // last flag is deleteComponentWhenNotNeeded
            // we manage deletion of the grids
            tabs.addTab(juce::String(panel->getTabName()), juce::Colours::black, panel, false);
        }
        // if for some reason you want something other than
        // the first tab selected you can call tis
        // tabs.setCurrentTabIndex(index, false);
        addChildComponent(tabs);
        tabs.setVisible(true);
    }
    else if (panels.size() > 0) {
        FormPanel* first = panels[0];
        addChildComponent(first);
        // necessary?
        first->setVisible(true);
    }

    autoSize();
}

void Form::autoSize()
{
    int maxWidth = 0;
    int maxHeight = 0;
    
    for (int i = 0 ; i < panels.size() ; i++) {
        FormPanel* panel = panels[i];
        panel->autoSize();
        if (panel->getWidth() > maxWidth)
          maxWidth = panel->getWidth();
        if (panel->getHeight() > maxHeight)
          maxHeight = panel->getHeight();
    }

    // this isn't exactly right because it doesn't account for the
    // tab bar, hmm, should be looking at the font
    if (panels.size() > 1) {
        maxHeight += 10;
    }

    setSize(maxWidth, maxHeight);
}
        
/**
 * Give all the tab grids the full size.
 * Need to factor out the tab buttons.
 */
void Form::resized()
{
    if (panels.size() > 1) {
        // we used a tab component
        tabs.setSize(getWidth(), getHeight());
    }
    else {
        // is this necessary, wait shouldn't we be doing this for all of them?
        FormPanel* first = panels[0];
        first->setTopLeftPosition(0, 0);
    }
}

void Form::paint(juce::Graphics& g)
{
    g.fillAll (juce::Colours::beige);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

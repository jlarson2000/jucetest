/**
 * Component model for Mobius configuration forms.
 * 
 * A form consists of a list of FormPanels.  If there is more than
 * one FormPanel a tabbed component is added to select the visible panel.
 */

#pragma once

#include <JuceHeader.h>

#include "Field.h"
#include "FormPanel.h"

class Form : public juce::Component
{
  public:

    Form();
    ~Form();

    void add(FormPanel* panel);

    // will want more options here
    void add(Field* f, const char* tab = nullptr, int column = 0);

    void add(Field* f, int column) {
        add(f, nullptr, column);
    }
    
    // convience for most config panels
    void add(const char* tab, class Parameter* p, int column = 0);

    void render();
    void autoSize();
    void gatherFields(juce::Array<Field*>& fields);

    void resized() override;
    void paint (juce::Graphics& g) override;
    
  private:
    
    juce::OwnedArray<FormPanel> panels;
    juce::TabbedComponent tabs;

};


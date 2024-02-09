/*
 * Provides a basic headered, scrolling table.
 */

#pragma once

#include <JuceHeader.h>
#include "JLabel.h"

class SimpleTable : public juce::Component, public juce::TableListBoxModel
{
  public:

    class Listener {
      public:
        virtual void tableTouched(SimpleTable* t) = 0;
    };
    
    SimpleTable();
    ~SimpleTable();

    void setColumnTitles(juce::StringArray& src) {
        columnTitles = src;
    }

    void setListener(Listener* l) {
        listener = l;
    }

    void render();

    // juce::Component overrides
    
    void resized() override;
    void paint (juce::Graphics& g) override;

    int getNumRows() override;
    void paintRowBackground (juce::Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected) override;
    void paintCell (juce::Graphics& g, int rowNumber, int columnId,
                    int width, int height, bool rowIsSelected) override;
    void cellClicked(int rowNumber, int columnId, const juce::MouseEvent& event) override;


  private:
    
    Listener* listener = nullptr;
    juce::StringArray columnTitles;

    juce::TableListBox table { {} /* component name */, this /* TableListBoxModel */};
    juce::Font font           { 14.0f };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleTable)
};

/**
 * A table showing a list of Bindings being edited.
 *
 * Where should cell formatting go, in the paretent or in the Binding?
 * Having it in binding makes the interface here simpler, but adds
 * a lot of display stuff to Binding which is a simple model.
 * 
 */

#pragma once

#include <JuceHeader.h>

#include "../common/ButtonBar.h"

class BindingTable : public juce::Component, public juce::TableListBoxModel, public ButtonBar::Listener
{
  public:
    
    BindingTable();
    ~BindingTable();

    class Listener {
      public:
        virtual juce::String renderTriggerCell(class Binding* b) = 0;
        virtual void bindingSelected(class Binding* b) = 0;
        virtual void bindingUpdate(class Binding* b) = 0;
        virtual void bindingDelete(class Binding* b) = 0;
        virtual class Binding* bindingNew() = 0;
    };

    // currently expected to be a linked list from the old model
    // copies the list and ownership is retained by the caller
    void setBindings(class Binding* bindings);
    void add(class Binding* binding);
    void updateContent();
    
    // return the edited list, ownership transfers to the caller
    class Binding* captureBindings();

    void clear();

    // option to hide the trigger column
    void setNoTrigger(bool b) {
        noTrigger = b;
    }
    
    void setListener(Listener* l) {
        listener = l;
    }

    int getPreferredWidth();
    int getPreferredHeight();

    // return true if this was creatred as a placeholder row for a new binding
    bool isNew(Binding* b);

    // ButtonBar::Listener
    void buttonClicked(juce::String name);

    // Component
    void resized();

    // TableListBoxModel
    
    int getNumRows() override;
    void paintRowBackground (juce::Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected) override;
    void paintCell (juce::Graphics& g, int rowNumber, int columnId,
                    int width, int height, bool rowIsSelected) override;
    void cellClicked(int rowNumber, int columnId, const juce::MouseEvent& event) override;

  private:

    juce::OwnedArray<class Binding> bindings;
    bool noTrigger = false;
    Listener* listener = nullptr;
    int targetColumn = 0;
    int triggerColumn = 0;
    int argumentsColumn = 0;
    int scopeColumn = 0;
    
    ButtonBar commands;
    juce::TableListBox table { {} /* component name */, this /* TableListBoxModel */};

    void initTable();
    void initColumns();

    juce::String getCellText(int row, int columnId);
    juce::String formatTriggerText(class Binding* b);
    juce::String formatScopeText(class Binding* b);
    
};
    

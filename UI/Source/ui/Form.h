/**
 * Component model for Mobius configuration forms.
 * 
 */

#pragma once

#include <JuceHeader.h>
#include "Field.h"

class Form : public juce::Component
{
  public:

    // surely there is some library/Juce support for an iterator interface?
    class Iterator
    {
      public:
    
        Iterator(Form* form);
        ~Iterator();

        void reset();
        bool hasNext();
        Field* next();

      private:

        void advance();
        
        Form* form;
        int tabIndex = 0;
        int colIndex = 0;
        int fieldIndex = 0;
        Field* nextField = nullptr;
    };

    Form();
    ~Form();

    // will want more options here
    void add(Field* f, const char* tab = nullptr, int column = 0);

    void add(Field* f, int column) {
        add(f, nullptr, column);
    }
    
    void render();

  protected:

    juce::OwnedArray<FieldSet> tabs;

};

    

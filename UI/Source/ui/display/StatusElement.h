/**
 * Base class of a compone that displays a piece of Mobius
 * runtime state and optionally supports actions.
 */

#pragma once

#include <JuceHeader.h>

class StatusElement : public juce::Component
{
  public:
    
    StatusElement(class StatusArea*);
    ~StatusElement();

    virtual void configure(class UIConfig* config);
    virtual void update(class MobiusState* state);
    virtual int getPreferredWidth();
    virtual int getPreferredHeight();
    
  private:

    class StatusArea* area;
    

};


    

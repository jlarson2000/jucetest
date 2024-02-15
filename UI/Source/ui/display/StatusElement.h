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

    void configure(UIConfig* config);

    void resized() override;
    void paint(juce::Graphics& g) override;

  private:

    class StatusArea* area;
    

};


    

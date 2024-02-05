/**
 * A basic container component with automatic layout options.
 */

#pragma once

#include <JuceHeader.h>

class Panel : public juce::Component
{
  public:

    enum Orientation {
        Vertical,
        Horizontal
    };

    Panel();
    Panel(Orientation);
    ~Panel() override;

    setOrientation(Orientation);
    
    void resized();

  private:

    Orientation orientation = Vertical;
    void layout();
    
};



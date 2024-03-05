/**
 * Status element to display a configued set of Parameter values
 * and allow temporary editing.
 */

#pragma once

#include <JuceHeader.h>

#include "StatusElement.h"

class ParametersElement : public StatusElement
{
  public:
    
    ParametersElement(class StatusArea* area);
    ~ParametersElement();

    void configure(class UIConfig* config) override;
    void configure(class MobiusConfig* config) override;

    void update(class MobiusState* state) override;
    int getPreferredWidth() override;
    int getPreferredHeight() override;

    void resized() override;
    void paint(juce::Graphics& g) override;

    void doAction(class UIAction* action);
    
  private:

    juce::Array<class UIParameter*> parameters;
    juce::StringArray presetNames;
    juce::Array<int> parameterValues;
    int maxNameWidth = 0;
    int maxValueWidth = 0;
};

    

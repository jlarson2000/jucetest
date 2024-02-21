/**
 * Base class of a compone that displays a piece of Mobius
 * runtime state and optionally supports actions.
 */

#pragma once

#include <JuceHeader.h>

class StatusElement : public juce::Component
{
  public:
    
    StatusElement(class StatusArea*, const char* id);
    ~StatusElement();

    virtual void configure(class UIConfig* config);
    virtual void configure(class MobiusConfig* config);
    virtual void update(class MobiusState* state);
    virtual int getPreferredWidth();
    virtual int getPreferredHeight();

    void paint(juce::Graphics& g) override;
    
    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& e) override;

  protected:
    
    class StatusArea* area;

  private:

    juce::ComponentDragger dragger;
    // testing hack, StatusRotary now has one too
    bool dragging = false;
  
};


    

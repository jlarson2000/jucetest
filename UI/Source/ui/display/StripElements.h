/**
 * Implementations for the initial set
 * of StripElements.  Decide whether to break these out into files
 * or collect under a folder since we're going to have a lot of them.
 */

#pragma once

#include "StripElement.h"
#include "StripRotary.h"

class TrackNumberElement : public StripElement
{
  public:
    
    TrackNumberElement(class TrackStrip* parent);
    ~TrackNumberElement();

    int getPreferredHeight() override;
    int getPreferredWidth() override;

    void paint(juce::Graphics& g) override;
    
  private:
};

class LoopRadarElement : public StripElement
{
  public:
    
    LoopRadarElement(class TrackStrip* parent);
    ~LoopRadarElement();

    int getPreferredHeight() override;
    int getPreferredWidth() override;

    void update(MobiusState* state) override;
    void paint(juce::Graphics& g) override;

  private:

    long loopFrames = 0;
    long loopFrame = 0;

};

class LoopThermometerElement : public StripElement
{
  public:
    
    LoopThermometerElement(class TrackStrip* parent);
    ~LoopThermometerElement();

    int getPreferredHeight() override;
    int getPreferredWidth() override;

    void update(MobiusState* state) override;
    void paint(juce::Graphics& g) override;

  private:

    long loopFrames = 0;
    long loopFrame = 0;

};

class OutputLevelElement : public StripRotary
{
  public:
    
    OutputLevelElement(class TrackStrip* parent);
    ~OutputLevelElement();

    void update(MobiusState* state) override;

  private:

    long level = 0;

};
    

    
    
        

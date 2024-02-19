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

class FocusLockElement : public StripElement
{
  public:
    
    FocusLockElement(class TrackStrip* parent);
    ~FocusLockElement();

    int getPreferredHeight() override;
    int getPreferredWidth() override;

    void update(MobiusState* state) override;
    void paint(juce::Graphics& g) override;
    
  private:
    bool focusLock = false;
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
    void sliderValueChanged(juce::Slider* slider) override;

  private:

    long level = 0;

};
    
class InputLevelElement : public StripRotary
{
  public:
    
    InputLevelElement(class TrackStrip* parent);
    ~InputLevelElement();

    void update(MobiusState* state) override;
    void sliderValueChanged(juce::Slider* slider) override;

  private:

    long level = 0;

};
    
class FeedbackElement : public StripRotary
{
  public:
    
    FeedbackElement(class TrackStrip* parent);
    ~FeedbackElement();

    void update(MobiusState* state) override;
    void sliderValueChanged(juce::Slider* slider) override;

  private:

    long level = 0;

};
    
class SecondaryFeedbackElement : public StripRotary
{
  public:
    
    SecondaryFeedbackElement(class TrackStrip* parent);
    ~SecondaryFeedbackElement();

    void update(MobiusState* state) override;
    void sliderValueChanged(juce::Slider* slider) override;

  private:

    long level = 0;

};
    
class PanElement : public StripRotary
{
  public:
    
    PanElement(class TrackStrip* parent);
    ~PanElement();

    void update(MobiusState* state) override;
    void sliderValueChanged(juce::Slider* slider) override;

  private:

    long level = 0;

};
    

    
    
        

/**
 * Implementations strip elements that are not Parameters
 */

#pragma once

#include "StripElement.h"
#include "StripRotary.h"

class StripTrackNumber : public StripElement
{
  public:
    
    StripTrackNumber(class TrackStrip* parent);
    ~StripTrackNumber();

    int getPreferredHeight() override;
    int getPreferredWidth() override;

    void paint(juce::Graphics& g) override;
    
  private:
};

class StripFocusLock : public StripElement
{
  public:
    
    StripFocusLock(class TrackStrip* parent);
    ~StripFocusLock();

    int getPreferredHeight() override;
    int getPreferredWidth() override;

    void update(MobiusState* state) override;
    void paint(juce::Graphics& g) override;
    
  private:
    bool focusLock = false;
};

class StripLoopRadar : public StripElement
{
  public:
    
    StripLoopRadar(class TrackStrip* parent);
    ~StripLoopRadar();

    int getPreferredHeight() override;
    int getPreferredWidth() override;

    void update(MobiusState* state) override;
    void paint(juce::Graphics& g) override;

  private:

    long loopFrames = 0;
    long loopFrame = 0;

};

class StripLoopThermometer : public StripElement
{
  public:
    
    StripLoopThermometer(class TrackStrip* parent);
    ~StripLoopThermometer();

    int getPreferredHeight() override;
    int getPreferredWidth() override;

    void update(MobiusState* state) override;
    void paint(juce::Graphics& g) override;

  private:

    long loopFrames = 0;
    long loopFrame = 0;

};
    
class StripLoopStack : public StripElement
{
  public:
    
    StripLoopStack(class TrackStrip* parent);
    ~StripLoopStack();

    void configure(class MobiusConfig* config);
    void configure(UIConfig* config);
    
    int getPreferredHeight() override;
    int getPreferredWidth() override;

    void update(MobiusState* state) override;
    void paint(juce::Graphics& g) override;
    
  private:

    int maxLoops = 0;
    class MobiusTrackState* track = nullptr;
    int lastActive = -1;
    long lastFrame = 0;
    
};
        
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

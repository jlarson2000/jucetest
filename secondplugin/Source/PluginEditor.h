/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class SecondpluginAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                          private juce::Slider::Listener
{
public:
    SecondpluginAudioProcessorEditor (SecondpluginAudioProcessor&);
    ~SecondpluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized () override;

private:
    // Slider listener
    void sliderValueChanged(juce::Slider* slider) override;
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SecondpluginAudioProcessor& audioProcessor;

    juce:: Slider midiVolume;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SecondpluginAudioProcessorEditor)
};

#pragma once

#include <JuceHeader.h>
// #include "MainMenu.h"
#include "SimpleMenu.h"
//#include "PopupTest.h"
#include "ConfigPopup.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent, ConfigPopup::Listener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

    // callbacks from the menu
    void showPresets();
    void showSetups();

    // callbacks from the popup
    void configPopupClosed(ConfigPopup* p);

  private:

    juce::Label titleLabel;
    juce::Label anotherLabel;

    // class MainMenu mainMenu;
    class SimpleMenu mainMenu;

    PresetPopup presetPopup;
    SetupPopup setupPopup;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

/**
 * Simple component to contain random tests.
 */

#pragma once

#include <JuceHeader.h>

class TestContentPanel : public juce::Component
{
  public:

    TestContentPanel();
    ~TestContentPanel();

    void resized() override;
    void paint (juce::Graphics& g) override;

  private:

    juce::TabbedComponent tabs {juce::TabbedButtonBar::Orientation::TabsAtTop};
};

class TestPanel : public juce::Component
{
  public:


    TestPanel();
    ~TestPanel() override;

    void center();

    // Component
    void resized() override;
    void paint (juce::Graphics& g) override;

  protected:
    
    TestContentPanel content;

  private:

    juce::TabbedComponent tabs {juce::TabbedButtonBar::Orientation::TabsAtTop};

};

class TestPanelSub : public TestPanel
{
  public:

    TestPanelSub();
    ~TestPanelSub() override;

  private:

    juce::TabbedComponent subtabs {juce::TabbedButtonBar::Orientation::TabsAtTop};

};    

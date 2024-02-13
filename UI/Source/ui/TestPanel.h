/**
 * Simple component to contain random tests.
 */

#pragma once

#include <JuceHeader.h>
#include "common/JLabel.h"
#include "common/Panel.h"
#include "common/SimpleRadio.h"

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

class TestPanel : public juce::Component, SimpleRadio::Listener
{
  public:


    TestPanel();
    ~TestPanel() override;

    void center();

    void radioSelected(SimpleRadio* radio, int index);

    // Component
    void resized() override;
    void paint (juce::Graphics& g) override;

  protected:
    
    TestContentPanel content;

  private:

    juce::TabbedComponent tabs {juce::TabbedButtonBar::Orientation::TabsAtTop};

    JLabel label;
    Panel panel;
    
};

class TestPanelSub : public TestPanel
{
  public:

    TestPanelSub();
    ~TestPanelSub() override;

  private:

    juce::TabbedComponent subtabs {juce::TabbedButtonBar::Orientation::TabsAtTop};

};    

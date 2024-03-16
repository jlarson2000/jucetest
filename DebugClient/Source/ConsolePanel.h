/*
 * Extension of TextEditor for experimentation.
 */

#pragma once

#include <JuceHeader.h>

class ConsolePanel : public juce::TextEditor, juce::TextEditor::Listener
{
  public:

    ConsolePanel();
    ~ConsolePanel();

    void add(const juce::String& m);
    void prompt(const juce::String& m);

    void textEditorReturnKeyPressed(TextEditor& ed) override;

  private:

};

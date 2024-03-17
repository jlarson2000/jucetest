/*
 * Extension of TextEditor for experimentation.
 */

#pragma once

#include <JuceHeader.h>

class ConsolePanel : public juce::TextEditor, juce::TextEditor::Listener
{
  public:

    class Listener {
      public:
        virtual void lineReceived(juce::String line) = 0;
    };

    ConsolePanel();
    ~ConsolePanel();

    void setListener(Listener* l) {
        listener = l;
    }
    
    void add(const juce::String& m);
    void addAndPrompt(const juce::String& m);
    void prompt();
    void newline();
    
    void textEditorReturnKeyPressed(TextEditor& ed) override;

  private:

    juce::String getLastLine();
    Listener* listener = nullptr;
    
};

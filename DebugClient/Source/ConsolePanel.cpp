/**
 * Simple extension of TextEditor trying to make something that looks
 * like a command line console.
 *
 * Listener callbacks for textChanged, returnKeyPressed, escapeKeyPressed, focusLost
 *
 */

#include <JuceHeader.h>

#include "ConsolePanel.h"

ConsolePanel::ConsolePanel()
{
    // see notes on opaque above
    // oh, this does the opposite of what I thought, if you set this you MUST
    // paint out the entire area
    //setOpaque(true);
    
    // always want multiple lines
    setMultiLine (true);
    // used in the example, but I don't think it's relevant if read-only?
    // set to false for a "console"
    setReturnKeyStartsNewLine (false);
    // true in LogPanel, false here
    setReadOnly (false);
    // oh yeah, bring on the scroll
    setScrollbarsShown (true);
    // false for LogPanel, true here
    setCaretVisible (true);

    // this looks interesting
    // If enabled, right-clicking (or command-clicking on the Mac) will pop up a
    // menu of options such as cut/copy/paste, undo/redo, etc.
    setPopupMenuEnabled (true);

    // colors from the example, start with these
    // other colors are textColourId, highlightColourId, highlightedTextColourId
    // focusedOutlineColourId
    // can also change caret colours using CaretComponent::caretColourId

    // can be transparent whatever that means
    setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x32ffffff));
    // if not transparent, draws a box around the edge
    // also focusedOutlineColourId is a different color when focused
    setColour (juce::TextEditor::outlineColourId, juce::Colour (0x1c000000));
    // if non-transparent, draws an inner shadow around the edge 
    setColour (juce::TextEditor::shadowColourId, juce::Colour (0x16000000));

    // hmm, when I added this to a beige panel, it just showed in a lighter shade
    // of beige, in the demo the main components has a black backround and the log
    // was a little lighter so the demo back ground colour must have transparency
    // in it.  this is interesting, but I don't have time to explore it yet, just
    // fix a color
    setColour(juce::TextEditor::backgroundColourId, juce::Colours::grey);
    

    // textColourId "used when text is added", it does not change the colour of
    // existing text so this could be nice for formatting log words
    // can use applyColourToAllText to change all existing text

    // highlightColourId "fill the background of highlighted sections"
    // can be transparent if you don't want highlighting

    // listen to thyself
    addListener(this);
}

ConsolePanel::~ConsolePanel()
{
}

void ConsolePanel::add(const juce::String& m)
{
    moveCaretToEnd();
    insertTextAtCaret (m + juce::newLine);
}

void ConsolePanel::prompt(const juce::String& m)
{
    moveCaretToEnd();
    insertTextAtCaret (m);
}

void ConsolePanel::textEditorReturnKeyPressed(TextEditor& ed)
{
    add("Width " + juce::String(getTextWidth()));
    add("Height " + juce::String(getTextHeight()));
    prompt("> ");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

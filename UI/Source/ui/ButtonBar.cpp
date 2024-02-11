
#include <JuceHeader.h>

#include "Panel.h"
#include "ButtonBar.h"

ButtonBar::ButtonBar()
{
    setName("ButtonBar");
}

ButtonBar::~ButtonBar()
{
}

void ButtonBar::add(juce::String name)
{
    juce::TextButton* b = new juce::TextButton(name);
    b->addListener(this);
    buttons.add(b);
    addAndMakeVisible(b);
}

void ButtonBar::resized()
{
    int height = getHeight();

    // this seems not entirely accurate since we don't
    // explicitly set the button font, it will apparently
    // be an unknown percentage of the height given
    // but we'll err on the larger side
    juce::Font font = juce::Font(height);
    int maxWidth = 0;

    const juce::Array<Component*>& children = getChildren();
    for (int i = 0 ; i < children.size() ; i++) {
        Component* child = children[i];
        juce::TextButton* b = dynamic_cast<juce::TextButton*>(child);
        if (b != nullptr) {
            juce::String text = b->getButtonText();
            int width = font.getStringWidth(text);
            if (width > maxWidth)
              maxWidth = width;
        }
    }

    // padding on each side
    maxWidth += 10;

    int totalWidth = maxWidth * children.size();
    int centerOffset = (getWidth() - totalWidth) / 2;

    for (int i = 0 ; i < children.size() ; i++) {
        Component* child = children[i];
        child->setBounds(centerOffset, 0, maxWidth, height);
        centerOffset += maxWidth;
    }
}

void ButtonBar::buttonClicked(juce::Button* b)
{
    if (listener != nullptr)
      listener->buttonClicked(b->getButtonText());
}



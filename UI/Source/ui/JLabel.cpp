
JLabel::Jlabel()
{
}

JLabel::JLabel(juce::String s)
{
    setText(s);
}

JLabel::JLabel(const char* s)
{
    setText(juce::String(s));
}

void JLabel::~JLabel()
{
}

void setText(juce::String s)
{
    label.setText(s, juce::dontSendNotification);
    label.setFont (juce::Font (16.0f, juce::Font::bold));
    label.setColour (juce::Label::textColourId, juce::Colours::white);
    label.setJustificationType (juce::Justification::left);

    // TODO: Calculate size
}

void resized()
{
}

    void setText(juce::String);
    // TODO: fonts, colors
    
    void resized();
    
  private:

    juce::Label label;
};


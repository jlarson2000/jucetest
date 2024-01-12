//
// 

#include <JuceHeader.h>

#include "MainComponent.h"


class MenusDemo    : public juce::Component,
                     public juce::ApplicationCommandTarget,
                     public juce::MenuBarModel
{
  public:

    /** A list of the commands that this demo responds to. */
    enum CommandIDs
    {
        menuPositionInsideWindow = 1,
        menuPositionGlobalMenuBar,
        menuPositionBurgerMenu,
        outerColourRed,
        outerColourGreen,
        outerColourBlue,
        innerColourRed,
        innerColourGreen,
        innerColourBlue
    };

    //==============================================================================
    /** Represents the possible menu positions. */
    enum class MenuBarPosition
    {
        window,
        global,
        burger
    };

    MenusDemo()
    {
        menuBar.reset (new juce::MenuBarComponent (this));
        addAndMakeVisible (menuBar.get());
        setApplicationCommandManagerToWatch (&commandManager);
        commandManager.registerAllCommandsForTarget (this);

        // this ensures that commands invoked on the DemoRunner application are correctly
        // forwarded to this demo
        commandManager.setFirstCommandTarget (this);

        // this lets the command manager use keypresses that arrive in our window to send out commands
        addKeyListener (commandManager.getKeyMappings());

        setWantsKeyboardFocus (true);

        setSize (500, 300);
    }

    ~MenusDemo() override
    {
        commandManager.setFirstCommandTarget (nullptr);
    }

    void resized() override
    {
        auto b = getLocalBounds();

        menuBar->setBounds (b.removeFromTop (juce::LookAndFeel::getDefaultLookAndFeel()
                                             .getDefaultMenuBarHeight()));
    }
    
    // 
    // MenuBarModel
    // 

    juce::StringArray getMenuBarNames() override
    {
        return { "Menu Position", "Outer Colour", "Inner Colour" };
    }

    juce::PopupMenu getMenuForIndex (int menuIndex, const juce::String& /*menuName*/) override
    {
        juce::PopupMenu menu;

        if (menuIndex == 0)
        {
            menu.addCommandItem (&commandManager, CommandIDs::menuPositionInsideWindow);
           #if JUCE_MAC
            menu.addCommandItem (&commandManager, CommandIDs::menuPositionGlobalMenuBar);
           #endif
            menu.addCommandItem (&commandManager, CommandIDs::menuPositionBurgerMenu);
        }
        else if (menuIndex == 1)
        {
            menu.addCommandItem (&commandManager, CommandIDs::outerColourRed);
            menu.addCommandItem (&commandManager, CommandIDs::outerColourGreen);
            menu.addCommandItem (&commandManager, CommandIDs::outerColourBlue);
        }
        else if (menuIndex == 2)
        {
            menu.addCommandItem (&commandManager, CommandIDs::innerColourRed);
            menu.addCommandItem (&commandManager, CommandIDs::innerColourGreen);
            menu.addCommandItem (&commandManager, CommandIDs::innerColourBlue);
        }

        return menu;
    }

    void menuItemSelected (int /*menuItemID*/, int /*topLevelMenuIndex*/) override {}



    //==============================================================================
    // The following methods implement the ApplicationCommandTarget interface, allowing
    // this window to publish a set of actions it can perform, and which can be mapped
    // onto menus, keypresses, etc.

    juce::ApplicationCommandTarget* getNextCommandTarget() override
    {
        // return &outerCommandTarget;
        return nullptr;
    }

    void getAllCommands (juce::Array<juce::CommandID>& c) override
    {
        juce::Array<juce::CommandID> commands {
            CommandIDs::menuPositionInsideWindow,
            CommandIDs::menuPositionGlobalMenuBar,
            CommandIDs::menuPositionBurgerMenu
        };
        c.addArray (commands);
    }

    void getCommandInfo (juce::CommandID commandID, juce::ApplicationCommandInfo& result) override
    {
        switch (commandID)
        {
            case CommandIDs::menuPositionInsideWindow:
                result.setInfo ("Inside Window", "Places the menu bar inside the application window", "Menu", 0);
                result.setTicked (true);
                result.addDefaultKeypress ('w', juce::ModifierKeys::shiftModifier);
                break;
            case CommandIDs::menuPositionGlobalMenuBar:
                result.setInfo ("Global", "Uses a global menu bar", "Menu", 0);
                result.setTicked (false);
                result.addDefaultKeypress ('g', juce::ModifierKeys::shiftModifier);
                break;
            case CommandIDs::menuPositionBurgerMenu:
                result.setInfo ("Burger Menu", "Uses a burger menu", "Menu", 0);
                result.setTicked (false);
                result.addDefaultKeypress ('b', juce::ModifierKeys::shiftModifier);
                break;
            default:
                break;
        }
    }

    bool perform (const juce::ApplicationCommandTarget::InvocationInfo& info) override
    {
        switch (info.commandID)
        {
            case CommandIDs::menuPositionInsideWindow:
                break;
            case CommandIDs::menuPositionGlobalMenuBar:
                break;
            case CommandIDs::menuPositionBurgerMenu:
                break;
            default:
                return false;
        }

        return true;
    }

  private:
    juce::ApplicationCommandManager commandManager;
    std::unique_ptr<juce::MenuBarComponent> menuBar;
};
    
//==============================================================================

MainComponent::MainComponent()
{
    // add subcomponents before setting size
    addAndMakeVisible (titleLabel);
    titleLabel.setFont (juce::Font (16.0f, juce::Font::bold));
    titleLabel.setText ("Click in the white box to enter some text...", juce::dontSendNotification);
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::lightgreen);
    titleLabel.setJustificationType (juce::Justification::centred);

    addAndMakeVisible (anotherLabel);
    anotherLabel.setFont (juce::Font (16.0f, juce::Font::bold));
    anotherLabel.setText ("More...", juce::dontSendNotification);
    anotherLabel.setColour (juce::Label::textColourId, juce::Colours::lightgreen);
    anotherLabel.setJustificationType (juce::Justification::centred);

    testMenu = new MenusDemo();
    addAndMakeVisible(testMenu);


    // Make sure you set the size of the component after
    // you add any child components.
    setSize (800, 600);

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (2, 2);
    }
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
    bufferToFill.clearActiveBufferRegion();
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // You can add your drawing code here!
    g.fillAll (juce::Colours::black);

    /*
    juce::Rectangle<int> r = titleLabel.getBounds();
    g.setColour(juce::Colours::sandybrown);
    g.drawRect(r);
    */
}

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    titleLabel   .setBounds (10,  100, getWidth() - 20,  30);
    anotherLabel   .setBounds (10,  130, getWidth() - 20,  30);
}

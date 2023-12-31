/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SecondpluginAudioProcessor::SecondpluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

SecondpluginAudioProcessor::~SecondpluginAudioProcessor()
{
}

//==============================================================================
const juce::String SecondpluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SecondpluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SecondpluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SecondpluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SecondpluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SecondpluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SecondpluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SecondpluginAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SecondpluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void SecondpluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SecondpluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void SecondpluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SecondpluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SecondpluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }

    // picking up with Tutorial 2
    // this looks equivalent to the example loop above
    buffer.clear();

    // example creates a new MIDI buffer containing modified events
    // rather than direcdtly modifying the passed events, not sure why
    // maybe the passed events are shared by other plugins in the host?
    
    juce::MidiBuffer processedMidi;

    // collection iterator like Java, haven't used this in C++ much
    // read up on what "const auto" does
    // this is operator overloading?
    for (const auto metadata : midiMessages) {

        // what the fuck kind of declaration is this?
        // oh, this is interesting, it's a deduced type "new with C++ 11"
        // read more...
        // https://stackoverflow.com/questions/7576953/what-is-the-meaning-of-the-auto-keyword
        auto message = metadata.getMessage();
        
        const auto time = metadata.samplePosition;
        if (message.isNoteOn() || message.isNoteOff()) {
            // creates a new message
            /*
            message = juce::MidiMessage::noteOn(message.getChannel(),
                                                message.getNoteNumber(),
                                                // hmm, why is cast necessary?
                                                // oh, noteOnVel was declared float because it comes from the slider, downcast avoids a warning I guess
                                                (juce::uint8)noteOnVel);
            */
            // do pitch instead
            int shift = (int)((int)noteOnVel % 12);
            message = juce::MidiMessage::noteOn(message.getChannel(),
                                                message.getNoteNumber() + shift,
                                                message.getVelocity());
        }

        // add either the unprocessed or processed message to the new buffer
        // time is interesting, assume it is relative offset within the buffer
        // somehow, relative to zero?
        processedMidi.addEvent(message, time);
    }

    // replace with our mods
    midiMessages.swapWith(processedMidi);
}

//==============================================================================
bool SecondpluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SecondpluginAudioProcessor::createEditor()
{
    return new SecondpluginAudioProcessorEditor (*this);
}

//==============================================================================
void SecondpluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SecondpluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SecondpluginAudioProcessor();
}

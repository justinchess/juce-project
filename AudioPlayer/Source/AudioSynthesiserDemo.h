/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2020 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             AudioSynthesiserDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Simple synthesiser application.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2019, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        AudioSynthesiserDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "DemoUtilities.h"
#include "AudioLiveScrollingDisplay.h"

//==============================================================================
/** Our demo synth sound is just a basic sine wave.. */
struct SineWaveSound : public SynthesiserSound
{
    SineWaveSound() {}

    bool appliesToNote (int /*midiNoteNumber*/) override    { return true; }
    bool appliesToChannel (int /*midiChannel*/) override    { return true; }
};

//==============================================================================
/** Our demo synth voice just plays a sine wave.. */
struct SineWaveVoice  : public SynthesiserVoice
{
    SineWaveVoice() {}

    bool canPlaySound (SynthesiserSound* sound) override
    {
        return dynamic_cast<SineWaveSound*> (sound) != nullptr;
    }

    void startNote (int midiNoteNumber, float velocity,
                    SynthesiserSound*, int /*currentPitchWheelPosition*/) override
    {
        currentAngle = 0.0;
        level = velocity * 0.15;
        tailOff = 0.0;

        auto cyclesPerSecond = MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        auto cyclesPerSample = cyclesPerSecond / getSampleRate();

        angleDelta = cyclesPerSample * MathConstants<double>::twoPi;
    }

    void stopNote (float /*velocity*/, bool allowTailOff) override
    {
        if (allowTailOff)
        {
            // start a tail-off by setting this flag. The render callback will pick up on
            // this and do a fade out, calling clearCurrentNote() when it's finished.

            if (tailOff == 0.0) // we only need to begin a tail-off if it's not already doing so - the
                tailOff = 1.0;  // stopNote method could be called more than once.
        }
        else
        {
            // we're being told to stop playing immediately, so reset everything..
            clearCurrentNote();
            angleDelta = 0.0;
        }
    }

    void pitchWheelMoved (int /*newValue*/) override                              {}
    void controllerMoved (int /*controllerNumber*/, int /*newValue*/) override    {}

    void renderNextBlock (AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
    {
        if (angleDelta != 0.0)
        {
            if (tailOff > 0.0)
            {
                while (--numSamples >= 0)
                {
                    auto currentSample = (float) (std::sin (currentAngle) * level * tailOff);

                    for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                        outputBuffer.addSample (i, startSample, currentSample);

                    currentAngle += angleDelta;
                    ++startSample;

                    tailOff *= 0.99;

                    if (tailOff <= 0.005)
                    {
                        clearCurrentNote();

                        angleDelta = 0.0;
                        break;
                    }
                }
            }
            else
            {
                while (--numSamples >= 0)
                {
                    auto currentSample = (float) (std::sin (currentAngle) * level);

                    for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                        outputBuffer.addSample (i, startSample, currentSample);

                    currentAngle += angleDelta;
                    ++startSample;
                }
            }
        }
    }

    using SynthesiserVoice::renderNextBlock;

private:
    double currentAngle = 0.0, angleDelta = 0.0, level = 0.0, tailOff = 0.0;
};

//==============================================================================
// This is an audio source that streams the output of our demo synth.
struct SynthAudioSource  : public AudioSource
{
    SynthAudioSource (MidiKeyboardState& keyState)  : keyboardState (keyState)
    {
        // Add some voices to our synth, to play the sounds..
        for (auto i = 0; i < 4; ++i)
        {
            synth.addVoice (new SineWaveVoice());   // These voices will play our custom sine-wave sounds..
            synth.addVoice (new SamplerVoice());    // and these ones play the sampled sounds
        }

        // ..and add a sound for them to play...
        setUsingSineWaveSound();
    }

    void setUsingSineWaveSound()
    {
        synth.clearSounds();
        synth.addSound (new SineWaveSound());
    }

    void setUsingSampledSound()
    {
        WavAudioFormat wavFormat;

        std::unique_ptr<AudioFormatReader> audioReader (wavFormat.createReaderFor (createAssetInputStream ("cello.wav").release(), true));

        BigInteger allNotes;
        allNotes.setRange (0, 128, true);

        synth.clearSounds();
        synth.addSound (new SamplerSound ("demo sound",
                                          *audioReader,
                                          allNotes,
                                          74,   // root midi note
                                          0.1,  // attack time
                                          0.1,  // release time
                                          10.0  // maximum sample length
                                          ));
    }

    void prepareToPlay (int /*samplesPerBlockExpected*/, double sampleRate) override
    {
        midiCollector.reset (sampleRate);

        synth.setCurrentPlaybackSampleRate (sampleRate);
    }

    void releaseResources() override {}

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        // the synth always adds its output to the audio buffer, so we have to clear it
        // first..
        bufferToFill.clearActiveBufferRegion();

        // fill a midi buffer with incoming messages from the midi input.
        MidiBuffer incomingMidi;
        midiCollector.removeNextBlockOfMessages (incomingMidi, bufferToFill.numSamples);

        // pass these messages to the keyboard state so that it can update the component
        // to show on-screen which keys are being pressed on the physical midi keyboard.
        // This call will also add midi messages to the buffer which were generated by
        // the mouse-clicking on the on-screen keyboard.
        keyboardState.processNextMidiBuffer (incomingMidi, 0, bufferToFill.numSamples, true);

        // and now get the synth to process the midi events and generate its output.
        synth.renderNextBlock (*bufferToFill.buffer, incomingMidi, 0, bufferToFill.numSamples);
    }

    //==============================================================================
    // this collects real-time midi messages from the midi input device, and
    // turns them into blocks that we can process in our audio callback
    MidiMessageCollector midiCollector;

    // this represents the state of which keys on our on-screen keyboard are held
    // down. When the mouse is clicked on the keyboard component, this object also
    // generates midi messages for this, which we can pass on to our synth.
    MidiKeyboardState& keyboardState;

    // the synth itself!
    Synthesiser synth;
};

//==============================================================================
class AudioSynthesiserDemo  : public Component
{
public:
    AudioSynthesiserDemo()
    {
        addAndMakeVisible (keyboardComponent);
		keyboardComponent.setSize(2000, 400);
        //addAndMakeVisible (sineButton);
        sineButton.setRadioGroupId (321);
        sineButton.setToggleState (true, dontSendNotification);
        sineButton.onClick = [this] { synthAudioSource.setUsingSineWaveSound(); };

        //addAndMakeVisible (sampledButton);
        sampledButton.setRadioGroupId (321);
        sampledButton.onClick = [this] { synthAudioSource.setUsingSampledSound(); };

        //addAndMakeVisible (liveAudioDisplayComp);
        audioDeviceManager.addAudioCallback (&liveAudioDisplayComp);
        audioSourcePlayer.setSource (&synthAudioSource);

       #ifndef JUCE_DEMO_RUNNER
        RuntimePermissions::request (RuntimePermissions::recordAudio,
                                     [this] (bool granted)
                                     {
                                         int numInputChannels = granted ? 2 : 0;
                                         audioDeviceManager.initialise (numInputChannels, 2, nullptr, true, {}, nullptr);
                                     });
       #endif

        audioDeviceManager.addAudioCallback (&audioSourcePlayer);
        audioDeviceManager.addMidiInputDeviceCallback ({}, &(synthAudioSource.midiCollector));

        setOpaque (true);
    //    setSize (640, 480);
		for (int i = 0; i < 8; i++) {
			addAndMakeVisible(Slider[i]);
			Slider[i].onValueChange = [this] {

			};
			double volDefaultValue = 0.5;
			Slider[i].setRange(0.0, 1.0);
			Slider[i].setNumDecimalPlacesToDisplay(2);
			Slider[i].setTextBoxStyle(juce::Slider::TextBoxBelow,
				false,
				40,
				Slider[i].getTextBoxHeight()
			);
			Slider[i].setSliderStyle(Slider::SliderStyle::LinearVertical);
			Slider[i].setValue(volDefaultValue);
			Slider[i].setSkewFactorFromMidPoint(volDefaultValue);
		}
		for (int i = 0; i < 8; i++) {
			addAndMakeVisible(Label[i]);
			char ch[100];
			sprintf(ch, "f%d", i);
			String str(ch);
			Label[i].setText(str, juce::dontSendNotification);
		}
    }

    ~AudioSynthesiserDemo() override
    {
        audioSourcePlayer.setSource (nullptr);
        audioDeviceManager.removeMidiInputDeviceCallback ({}, &(synthAudioSource.midiCollector));
        audioDeviceManager.removeAudioCallback (&audioSourcePlayer);
        audioDeviceManager.removeAudioCallback (&liveAudioDisplayComp);
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
		g.setColour(Colour::fromRGB(78, 30, 30));
		g.fillRect(4, 5, getWidth() - 8, 20 + getHeight() / 3);
		g.setColour(Colour::fromRGB(37, 16, 10));
		for (int i = 0; i < 8; i++) {
			g.fillRect(16 + i * getWidth() / 8, 10, (getWidth() - 160) / 8 - 2, getHeight() / 3);
		}
		g.setColour(Colour::fromRGB(78, 30, 30));
		g.fillRect(4, 35 + getHeight() / 3, getWidth() * 3 / 4, getHeight() / 3 - 45);
		g.fillRect(getWidth() * 3 /4 + 10, 35 + getHeight() / 3, getWidth() / 4 - 14, getHeight() / 3 - 45);
		g.setColour(Colour::fromRGB(255, 255, 255));
		g.drawText("ADRS", getWidth() * 3 / 8 - 20, getHeight() / 3 + 40, 40, 20, juce::Justification::centred);
		g.drawText("Attack", getWidth() * 1 / 20 - 25, getHeight() / 3 + 60, 50, 15, juce::Justification::centred);
		g.drawText("Decay", getWidth() * 5 / 20 - 25, getHeight() / 3 + 60, 50, 15, juce::Justification::centred);
		g.drawText("Sustain", getWidth() * 9 / 20 - 25, getHeight() / 3 + 60, 50, 15, juce::Justification::centred);
		g.drawText("Release", getWidth() * 13 / 20 - 25, getHeight() / 3 + 60, 50, 15, juce::Justification::centred);
		g.drawText("Gain", getWidth() * 7 / 8 - 25, getHeight() / 3 + 60, 50, 15, juce::Justification::centred);
		g.drawText("Output", getWidth() * 7 / 8 - 20, getHeight() / 3 + 40, 40, 20, juce::Justification::centred);
	}

    void resized() override
    {
		for (int i = 0; i < 8; i++) {
			Slider[i].setBounds(16 + i * getWidth() / 8, 10, (getWidth() - 160) / 8 - 2, getHeight() / 3);
			Label[i].setBounds(16 + i * getWidth() / 8, 10, (getWidth() - 240) / 8 - 2, 30);
		}
        keyboardComponent   .setBounds (8, getHeight() * 2 / 3, getWidth() - 16, getHeight() / 3);
    }

private:
    // if this PIP is running inside the demo runner, we'll use the shared device manager instead
   #ifndef JUCE_DEMO_RUNNER
    AudioDeviceManager audioDeviceManager;
   #else
    AudioDeviceManager& audioDeviceManager { getSharedAudioDeviceManager (0, 2) };
   #endif

    MidiKeyboardState keyboardState;
    AudioSourcePlayer audioSourcePlayer;
    SynthAudioSource synthAudioSource        { keyboardState };
    MidiKeyboardComponent keyboardComponent  { keyboardState, MidiKeyboardComponent::horizontalKeyboard};

	juce::Slider		Slider[8];
	juce::Label			Label[8];

    ToggleButton sineButton     { "Use sine wave" };
    ToggleButton sampledButton  { "Use sampled sound" };

    LiveScrollingAudioDisplay liveAudioDisplayComp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioSynthesiserDemo)
};

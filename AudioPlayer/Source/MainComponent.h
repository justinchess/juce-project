#pragma once

#include <JuceHeader.h>
#include <juce_gui_basics\juce_gui_basics.h>
#include "DJAudioPlayer.h"
#include "DeckGUI.h"
#include "PlaylistComponent.h"
#include "Mixture.h"
#include "AudioSynthesiserDemo.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent,
						public juce::Button::Listener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;
	void buttonClicked(juce::Button* button) override;

	void reset();
private:
    //==============================================================================
    // Your private member variables go here...

    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbCache{100};

    DJAudioPlayer player1{formatManager};
    DJAudioPlayer player2{formatManager};
	DJAudioPlayer player3{ formatManager };
	
    DJAudioPlayer playerForParsingMetaData{formatManager};

	AudioSynthesiserDemo synthesiser;
public:
    DeckGUI deckGUI1{1, &player1, formatManager, thumbCache};
    DeckGUI deckGUI2{2, &player2, formatManager, thumbCache};
	DeckGUI deckGUI3{3, &player3, formatManager, thumbCache };
	//PlaylistComponent playlistComponent{ &deckGUI1, &deckGUI2, &playerForParsingMetaData };
private:
	DJAudioPlayer mix_player{ formatManager };
	Mixture mixture{ 0, &mix_player, formatManager, thumbCache };
	PlaylistComponent playlistComponent{ &deckGUI1, &deckGUI2, &deckGUI3, &mixture, &playerForParsingMetaData };

    juce::MixerAudioSource mixerSource;


	bool				switchFlag;
	juce::TextButton	switchButton{ "Switch to Synthesizer" };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

extern MainComponent *g_mainComponent;
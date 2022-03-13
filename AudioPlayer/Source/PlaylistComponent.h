/*
  ==============================================================================

    PlaylistComponent.h
    Created: 24 Jul 2020 3:29:26pm
    Author:  Blair

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <algorithm>
#include <fstream>
#include "Track.h"
#include "DeckGUI.h"
#include "DJAudioPlayer.h"
#include "Mixture.h"
//==============================================================================
/*
*/
class PlaylistComponent  : public juce::Component,
                           public juce::TableListBoxModel,
                           public juce::Button::Listener,
                           public juce::TextEditor::Listener
{
public:
    PlaylistComponent(DeckGUI* _deckGUI1, 
                      DeckGUI* _deckGUI2, 
					  DeckGUI* _deckGUI3,
					  Mixture* _mixture,
                      DJAudioPlayer* _playerForParsingMetaData
                     );
    ~PlaylistComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    int getNumRows() override;
    void paintRowBackground(juce::Graphics& g, 
                            int rowNumber, 
                            int width,
                            int height,
                            bool rowIsSelected
                           ) override;
    void paintCell(juce::Graphics& g,
                   int rowNumber,
                   int columnId,
                   int width,
                   int height,
                   bool rowIsSelected
                  ) override;
    
    Component* refreshComponentForCell(int rowNumber, 
                                       int columnId, 
                                       bool isRowSelected, 
                                       Component* existingComponentToUpdate) override;
    void buttonClicked(juce::Button* button) override;
private:
    std::vector<Track> tracks;
    
    juce::TextButton importButton{ "IMPORT TRACKS" };
    juce::TextEditor searchField;
    juce::TableListBox library;
    juce::TextButton addToPlayer1Button{ "ADD TO DECK 1" };
    juce::TextButton addToPlayer2Button{ "ADD TO DECK 2" };
	juce::TextButton addToPlayer3Button{ "ADD TO DECK 3" };

    DeckGUI* deckGUI1;
    DeckGUI* deckGUI2;
	DeckGUI* deckGUI3;
	Mixture* mixture;
    DJAudioPlayer* playerForParsingMetaData;
    
    juce::String getLength(juce::URL audioURL);
    juce::String secondsToMinutes(double seconds);

    void importToLibrary();
    void searchLibrary(juce::String searchText);
    void saveLibrary();
    void loadLibrary();
    void deleteFromTracks(int id);
    bool isInTracks(juce::String fileNameWithoutExtension);
    int whereInTracks(juce::String searchText);
    void loadInPlayer(DeckGUI* deckGUI);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlaylistComponent)
};

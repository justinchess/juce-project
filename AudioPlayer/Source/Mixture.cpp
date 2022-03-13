/*
==============================================================================

Mixture.cpp
Created: 23 Jul 2020 12:16:53pm
Author:  Blair

==============================================================================
*/

#include <JuceHeader.h>
#include "Mixture.h"
#include "MainComponent.h"

//==============================================================================
Mixture::Mixture(int _id,
	DJAudioPlayer* _player,
	juce::AudioFormatManager& formatManager,
	juce::AudioThumbnailCache& thumbCache
) : player(_player),
id(_id),
waveformDisplay(id, formatManager, thumbCache)
{
	// add all components and make visible
	addAndMakeVisible(playButton);
	addAndMakeVisible(stopButton);
	addAndMakeVisible(mixButton);
	addAndMakeVisible(saveButton);
	addAndMakeVisible(volSlider);
	addAndMakeVisible(volLabel);
	addAndMakeVisible(speedSlider);
	addAndMakeVisible(speedLabel);
	addAndMakeVisible(posSlider);
	addAndMakeVisible(posLabel);
//	addAndMakeVisible(reverbPlot1);
//	addAndMakeVisible(reverbPlot2);
	addAndMakeVisible(waveformDisplay);

	// add listeners
	playButton.addListener(this);
	stopButton.addListener(this);
	mixButton.addListener(this);
	saveButton.addListener(this);
	volSlider.addListener(this);
	speedSlider.addListener(this);
	posSlider.addListener(this);
	reverbSlider.addListener(this);
	reverbPlot1.addListener(this);
	reverbPlot2.addListener(this);

	//configure volume slider and label
	double volDefaultValue = 0.5;
	volSlider.setRange(0.0, 1.0);
	volSlider.setNumDecimalPlacesToDisplay(2);
	volSlider.setTextBoxStyle(juce::Slider::TextBoxLeft,
		false,
		50,
		volSlider.getTextBoxHeight()
	);
	volSlider.setValue(volDefaultValue);
	volSlider.setSkewFactorFromMidPoint(volDefaultValue);
	volLabel.setText("Volume", juce::dontSendNotification);
	volLabel.attachToComponent(&volSlider, true);

	//configure speed slider and label
	double speedDefaultValue = 1.0;
	speedSlider.setRange(0.25, 4.0); //reaches breakpoint if sliderValue == 0
	speedSlider.setNumDecimalPlacesToDisplay(2);
	speedSlider.setTextBoxStyle(juce::Slider::TextBoxLeft,
		false,
		50,
		speedSlider.getTextBoxHeight()
	);
	speedSlider.setValue(speedDefaultValue);
	speedSlider.setSkewFactorFromMidPoint(speedDefaultValue);
	speedLabel.setText("Speed", juce::dontSendNotification);
	speedLabel.attachToComponent(&speedSlider, true);

	//configure position slider and label
	posSlider.setRange(0.0, 1.0);
	posSlider.setNumDecimalPlacesToDisplay(2);
	posSlider.setTextBoxStyle(juce::Slider::TextBoxLeft,
		false,
		50,
		posSlider.getTextBoxHeight()
	);
	posLabel.setText("Position", juce::dontSendNotification);
	posLabel.attachToComponent(&posSlider, true);

	//configure reverb slider
	reverbSlider.setRange(0.0, 1.0);
	reverbSlider.setNumDecimalPlacesToDisplay(2);

	//configure reverb plots
	//reverbPlot1.setRange(0.0, 1.0);
	//reverbPlot2.setRange(0.0, 1.0);
	//reverbPlot1.setGridLineCount(2);
	reverbPlot1.setTooltip("x: damping\ny: room size");
	reverbPlot2.setTooltip("x: dry level\ny: wet level");

	startTimer(500);
}

Mixture::~Mixture()
{
	stopTimer();
}

void Mixture::paint(juce::Graphics& g)
{
	/* This demo code just fills the component's background and
	draws some placeholder text to get you started.

	You should replace everything in this method with your own
	drawing code..
	*/

	g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));   // clear the background

	g.setColour(juce::Colours::grey);
	g.drawRect(getLocalBounds(), 1);   // draw an outline around the component
}

void Mixture::resized()
{
	/*This method is where you should set the bounds of any child
	components that your component contains..*/
	auto sliderLeft = getWidth() / 9;
	auto mainRight = getWidth();// -getHeight() / 2;
	auto plotRight = getWidth() - mainRight; // should == getHeight() / 2

											 //                   x start, y start, width, height
	playButton.setBounds(0, 7 * getHeight() / 8, mainRight / 4, getHeight() / 8);
	stopButton.setBounds(mainRight / 4, 7 * getHeight() / 8, mainRight / 4, getHeight() / 8);
	mixButton.setBounds(2 * mainRight / 4, 7 * getHeight() / 8, mainRight / 4, getHeight() / 8);
	saveButton.setBounds(3 * mainRight / 4, 7 * getHeight() / 8, mainRight / 4, getHeight() / 8);
	volSlider.setBounds(sliderLeft, 4 * getHeight() / 8, mainRight - sliderLeft, getHeight() / 8);
	speedSlider.setBounds(sliderLeft, 5 * getHeight() / 8, mainRight - sliderLeft, getHeight() / 8);
	posSlider.setBounds(sliderLeft, 6 * getHeight() / 8, mainRight - sliderLeft, getHeight() / 8);
	//    reverbPlot1.setBounds(mainRight, 0, plotRight, getHeight() / 2);
	//    reverbPlot2.setBounds(mainRight, getHeight()/2, plotRight, getHeight() / 2);
	waveformDisplay.setBounds(0, 0, mainRight, 4 * getHeight() / 8);
}

void Mixture::buttonClicked(juce::Button* button)
{
	if (button == &playButton)
	{
		DBG("Play button was clicked ");
		player->play();

	}
	if (button == &stopButton)
	{
		DBG("Stop button was clicked ");
		player->stop();
	}
	if (button == &mixButton)
	{
		DBG("Mix button was clicked ");
		String fileName = "out.wav";
		File outputFile = File(fileName);
		if (outputFile.exists())
			outputFile.deleteFile();
		outputFile.create();

		arrFiles.clear();
		if(g_mainComponent->deckGUI1.fileName.length() > 0)
			arrFiles.add(g_mainComponent->deckGUI1.fileName);
		if (g_mainComponent->deckGUI2.fileName.length() > 0)
			arrFiles.add(g_mainComponent->deckGUI2.fileName);
		if (g_mainComponent->deckGUI3.fileName.length() > 0)
			arrFiles.add(g_mainComponent->deckGUI3.fileName);
		if (arrFiles.size() < 2) {
			juce::AlertWindow::showMessageBox(juce::AlertWindow::AlertIconType::InfoIcon,
				"Can't mix decks",
				"Please add deck information",
				"OK",
				nullptr
			);
			return;
		}
		//arrFiles.add("D:\\mine\\juce\\NewMixer\\Stems\\Bridge\\Gtr1.wav");
		//arrFiles.add("D:\\mine\\juce\\NewMixer\\Stems\\Bridge\\Organ.wav");
		totalNumSamples = 0;
		mixerSource.removeAllInputs();
		
		juce::AudioFormatManager formatManager;
		formatManager.registerBasicFormats();
		for each (String filePath in arrFiles)
		{
			File inputFile = File(filePath);
			AudioFormatReader* reader = formatManager.createReaderFor(inputFile);
			totalNumSamples = totalNumSamples > reader->lengthInSamples ? totalNumSamples : 
				reader->lengthInSamples;
			if (reader != nullptr)
			{
				AudioFormatReaderSource *newSource = new AudioFormatReaderSource(reader, true);
				newSource->prepareToPlay(512, 48000.000000000000);
				mixerSource.addInputSource(newSource, true);
			}
		}
		mixerSource.prepareToPlay(512, 48000.000000000000);
		ScopedPointer <OutputStream> outStream(outputFile.createOutputStream());
		StringPairArray metaData = WavAudioFormat::createBWAVMetadata("", "", "", Time::getCurrentTime(), 0, "");
		ScopedPointer <AudioFormatWriter> writer(wav.createWriterFor(outStream, 48000.000000000000, 2, 24, metaData, 0));
		if (writer != nullptr)
		{
			outStream.release();
			bool ok = writer->writeFromAudioSource(mixerSource, totalNumSamples, 512);
			writer = nullptr;
		}
		loadFile(juce::URL{ outputFile });
	}
	if (button == &saveButton)
	{
		DBG("Save button was clicked ");
		juce::FileChooser chooser{ "Save as..." };

		if (chooser.browseForFileToSave(true))
		{
			saveFile(chooser.getResult().getFullPathName());
			DBG(chooser.getResult().getFileName());
		}
	}
}


void Mixture::sliderValueChanged(juce::Slider* slider)
{
	if (slider == &volSlider)
	{
		DBG("Volume slider moved " << slider->getValue());
		player->setGain(slider->getValue());
	}
	if (slider == &speedSlider)
	{
		DBG("Speed slider moved " << slider->getValue());
		player->setSpeed(slider->getValue());
	}
	if (slider == &posSlider)
	{
		DBG("Position slider moved " << slider->getValue());
		player->setPositionRelative(slider->getValue());
	}
}

void Mixture::coordPlotValueChanged(CoordinatePlot* coordinatePlot)
{
	DBG("Mixture::coordPlotValueChanged called");
	if (coordinatePlot == &reverbPlot1)
	{
		DBG("Deck " << id << ": ReverbPlot1 was clicked");
		player->setRoomSize(coordinatePlot->getY());
		player->setDamping(coordinatePlot->getX());
	}
	if (coordinatePlot == &reverbPlot2)
	{
		DBG("Deck " << id << ": ReverbPlot2 was clicked");
		player->setWetLevel(coordinatePlot->getY());
		player->setDryLevel(coordinatePlot->getX());
	}
}

bool Mixture::isInterestedInFileDrag(const juce::StringArray& files)
{
	DBG("Mixture::isInterestedInFileDrag called. "
		+ std::to_string(files.size()) + " file(s) being dragged.");
	return true;
}

void Mixture::filesDropped(const juce::StringArray& files, int x, int y)
{

	DBG("Mixture::filesDropped at " + std::to_string(x)
		+ "x and " + std::to_string(y) + "y");
	if (files.size() == 1)
	{
		loadFile(juce::URL{ juce::File{ files[0] } });
	}
}

void Mixture::loadFile(juce::URL audioURL)
{
	DBG("Mixture::loadFile called");
	player->loadURL(audioURL);
	waveformDisplay.loadURL(audioURL);
}

void Mixture::timerCallback()
{
	//check if the relative position is greater than 0
	//otherwise loading file causes error
	if (player->getPositionRelative() > 0)
	{
		posSlider.setValue(player->getPositionRelative());
		waveformDisplay.setPositionRelative(player->getPositionRelative());
	}
	if (player->getPositionRelative() >= 1) {
		posSlider.setValue(0);
		waveformDisplay.setPositionRelative(0);
	}
}

void Mixture::saveFile(String audioURL)
{
	File tempFile = File("out.wav");
	if (tempFile.exists())
		tempFile.deleteFile();

	DBG("Mixture::saveFile called");
	File outputFile = File(audioURL);
	if (outputFile.exists())
		outputFile.deleteFile();
	outputFile.create();

	totalNumSamples = 0;
	mixerSource.removeAllInputs();

	juce::AudioFormatManager formatManager;
	formatManager.registerBasicFormats();
	for each (String filePath in arrFiles)
	{
		File inputFile = File(filePath);
		AudioFormatReader* reader = formatManager.createReaderFor(inputFile);
		totalNumSamples = totalNumSamples > reader->lengthInSamples ? totalNumSamples :
			reader->lengthInSamples;
		if (reader != nullptr)
		{
			AudioFormatReaderSource *newSource = new AudioFormatReaderSource(reader, true);
			newSource->prepareToPlay(512, 48000.000000000000);
			mixerSource.addInputSource(newSource, true);
		}
	}
	mixerSource.prepareToPlay(512, 48000.000000000000);
	ScopedPointer <OutputStream> outStream(outputFile.createOutputStream());
	StringPairArray metaData = WavAudioFormat::createBWAVMetadata("", "", "", Time::getCurrentTime(), 0, "");
	ScopedPointer <AudioFormatWriter> writer(wav.createWriterFor(outStream, 48000.000000000000, 2, 24, metaData, 0));
	if (writer != nullptr)
	{
		outStream.release();
		bool ok = writer->writeFromAudioSource(mixerSource, totalNumSamples, 512);
		writer = nullptr;

		juce::AlertWindow::showMessageBox(juce::AlertWindow::AlertIconType::InfoIcon,
			"Info",
			"Successfully Saved.",
			"OK",
			nullptr
		);
	}
}
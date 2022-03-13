#include "MainComponent.h"

MainComponent *g_mainComponent;

//==============================================================================
MainComponent::MainComponent()
{
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (944, 600);

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

    addAndMakeVisible(deckGUI1);
    addAndMakeVisible(deckGUI2);
	addAndMakeVisible(deckGUI3);
	addAndMakeVisible(mixture);
	addAndMakeVisible(synthesiser);
    addAndMakeVisible(playlistComponent);

    formatManager.registerBasicFormats();
	g_mainComponent = this;

	switchFlag = true;
	addAndMakeVisible(switchButton);
	switchButton.addListener(this);
	reset();
}

void MainComponent::reset() {
	deckGUI1.setVisible(switchFlag);
	deckGUI2.setVisible(switchFlag);
	deckGUI3.setVisible(switchFlag);
	synthesiser.setVisible(!switchFlag);
	if (!switchFlag) {
		switchButton.setButtonText("Switch to Decks");
	}
	else {
		switchButton.setButtonText("Switch to Synthesizer");
	}
}
void MainComponent::buttonClicked(juce::Button* button) {
	switchFlag = !switchFlag;
	reset();
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
	String fileName = "out.wav";
	File outputFile = File(fileName);
	if (outputFile.exists())
		outputFile.deleteFile();
	outputFile.create();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()

    mixerSource.addInputSource(&player1, false);
    mixerSource.addInputSource(&player2, false);
	mixerSource.addInputSource(&player3, false);
	mixerSource.addInputSource(&mix_player, false);
    player1.prepareToPlay(samplesPerBlockExpected, sampleRate);
    player2.prepareToPlay(samplesPerBlockExpected, sampleRate);
	player3.prepareToPlay(samplesPerBlockExpected, sampleRate);
	mix_player.prepareToPlay(samplesPerBlockExpected, sampleRate);

}
void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    mixerSource.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
    mixerSource.removeAllInputs();
    mixerSource.releaseResources();
    player1.releaseResources();
    player2.releaseResources();
	player3.releaseResources();
	mix_player.releaseResources();
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.

    //playlistComponent.setBounds(0, 0, getWidth() / 3, getHeight());
    //deckGUI1.setBounds(getWidth() / 3, 0, 2 * getWidth() / 3, getHeight() / 2);
    //deckGUI2.setBounds(getWidth() / 3, getHeight() / 2, 2 * getWidth() / 3, getHeight() / 2);
    int columns = 100;
    auto playlistRight = 28 * getWidth() / columns;
    playlistComponent.setBounds(getWidth() - playlistRight, 50, playlistRight, getHeight() - 50);
    mixture.setBounds(0, 0, getWidth() - playlistRight, getHeight() / 4);
	deckGUI1.setBounds(0, getHeight() / 4, getWidth() - playlistRight, getHeight() / 4);
	deckGUI2.setBounds(0, getHeight() * 2 / 4, getWidth() - playlistRight, getHeight() / 4);
	deckGUI3.setBounds(0, getHeight() * 3 / 4, getWidth() - playlistRight, getHeight() / 4);
	synthesiser.setBounds(0, getHeight() / 4, getWidth() - playlistRight, getHeight() * 3 / 4);
	switchButton.setBounds(getWidth() - playlistRight, 0, playlistRight, 40);
}
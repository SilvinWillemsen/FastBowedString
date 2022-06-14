#include "ModalStiffStringView.h"

//==========================================================================
ModalStiffStringView::ModalStiffStringView()
{
	mPlayButton.addListener(this);
	mResetButton.addListener(this);
	mGainSlider.addListener(this);
	mInputPosSlider.addListener(this);
	mReadPosSlider.addListener(this);
	mBowPressureSlider.addListener(this);
	mBowSpeedSlider.addListener(this);

	mGainSlider.setRange(0.0, 5000.0, 0.1);
	mGainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
	mGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);

	addAndMakeVisible(mGainLabel);
	mGainLabel.setText("Gain", juce::dontSendNotification);
	mGainLabel.attachToComponent(&mGainSlider, false);
	mGainLabel.setJustificationType(juce::Justification::centred);

	mInputPosSlider.setRange(0.0, 100.0, 0.01);
	mInputPosSlider.setValue(0.733 * 100.0, juce::sendNotification);
	mInputPosSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
	mInputPosSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
	mInputPosSlider.setTextValueSuffix(" %");

	mReadPosSlider.setRange(0.0, 100.0, 0.01);
	mReadPosSlider.setValue(0.53 * 100.0, juce::sendNotification);
	mReadPosSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
	mReadPosSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
	mReadPosSlider.setTextValueSuffix(" %");

	addAndMakeVisible(mInputPosLabel);
	mInputPosLabel.setText("Input Pos", juce::dontSendNotification);
	mInputPosLabel.attachToComponent(&mInputPosSlider, false);
	mInputPosLabel.setJustificationType(juce::Justification::centred);

	addAndMakeVisible(mReadPosLabel);
	mReadPosLabel.setText("Read Pos", juce::dontSendNotification);
	mReadPosLabel.attachToComponent(&mReadPosSlider, false);
	mReadPosLabel.setJustificationType(juce::Justification::centred);

	mBowPressureSlider.setRange(0.0, 100.0, 0.001);
	mBowPressureSlider.setValue(10.0, juce::sendNotification);
	mBowPressureSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
	mBowPressureSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);

	addAndMakeVisible(mBowPressureLabel);
	mBowPressureLabel.setText("Bow Pressure", juce::dontSendNotification);
	mBowPressureLabel.attachToComponent(&mBowPressureSlider, false);
	mBowPressureLabel.setJustificationType(juce::Justification::centred);

	mBowSpeedSlider.setRange(0.0, 2.0, 0.0001);
	mBowSpeedSlider.setValue(0.2, juce::sendNotification);
	mBowSpeedSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
	mBowSpeedSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
	mBowSpeedSlider.setTextValueSuffix("m/s");

	addAndMakeVisible(mBowSpeedLabel);
	mBowSpeedLabel.setText("Bow Speed", juce::dontSendNotification);
	mBowSpeedLabel.attachToComponent(&mBowSpeedSlider, false);
	mBowSpeedLabel.setJustificationType(juce::Justification::centred);
}

ModalStiffStringView::~ModalStiffStringView()
{
}

//==========================================================================
void ModalStiffStringView::paint(juce::Graphics& g)
{

	/*std::vector<float> vState = mpStiffStringProcessor->GetStringState();
	DBG(vState.size());*/
    g.setColour (Colours::cyan);
    g.strokePath (VisualiseState (g), PathStrokeType(2.0f));

}
juce::Path ModalStiffStringView::VisualiseState (juce::Graphics& g)
{
    double visualScaling = 10;
    
    // String-boundaries are in the vertical middle of the component
    double stringBoundaries = getHeight() / 2.0;
    
    // Initialise path
    Path stringPath;
    
    // Start path
    stringPath.startNewSubPath (0, stringBoundaries);
    
    double spacing = getWidth() / static_cast<double>(mVisualizationPoints);
    double x = spacing;
    
    std::vector<float> vSumModes (mVisualizationPoints, 0);
    std::vector<float> curStringState = mpStiffStringProcessor->GetStringState();
    for (int l = 0; l < mVisualizationPoints; ++l)
        for (int m = 0; m < mStringModesNumber; ++m)
            vSumModes[l] += curStringState[m] * mVisualizationModes[l][m];

    for (int l = 0; l < mVisualizationPoints; l++)
    {
        // Needs to be -u, because a positive u would visually go down
        float newY = -vSumModes[l] * visualScaling * getHeight() + stringBoundaries;
        
        // if we get NAN values, make sure that we don't get an exception
        if (isnan(newY))
            newY = 0;
        
        stringPath.lineTo (x, newY);
        x += spacing;
    }
    
    return stringPath;
}


void ModalStiffStringView::resized()
{
	int vButtonsWidth = 100;
	int vButtonHeigth = 30;
	int vGainSliderDims = 100;
	int vSpacing = 50;

	mPlayButton.setButtonText("PLAY/PAUSE");
	mPlayButton.setBounds(getWidth() / 2 - getWidth() / 4 - vButtonsWidth / 2, getHeight() - getHeight() / 4 + vGainSliderDims / 2 - vButtonHeigth, vButtonsWidth, vButtonHeigth);
	mPlayButton.setClickingTogglesState(true);

	mResetButton.setBounds(getWidth() / 2 + getWidth() / 4 - vButtonsWidth / 2, getHeight() - getHeight() / 4 + vGainSliderDims / 2 - vButtonHeigth, vButtonsWidth, vButtonHeigth);
	mResetButton.setButtonText("RESET STATE");
	mResetButton.setClickingTogglesState(false);

	mInputPosSlider.setBounds(getWidth() / 2 - getWidth() / 4 - vButtonsWidth / 2, getHeight() - getHeight() / 4 - vSpacing - vButtonHeigth, vButtonsWidth, vButtonHeigth);
	mReadPosSlider.setBounds(getWidth() / 2 + getWidth() / 4 - vButtonsWidth / 2, getHeight() - getHeight() / 4 - vSpacing - vButtonHeigth, vButtonsWidth, vButtonHeigth);

	mBowPressureSlider.setBounds(getWidth() / 2 - getWidth() / 8 - vButtonsWidth / 2, getHeight() - getHeight() / 4 - vSpacing - vButtonHeigth, vButtonsWidth, vButtonHeigth);
	mBowSpeedSlider.setBounds(getWidth() / 2 + getWidth() / 8 - vButtonsWidth / 2, getHeight() - getHeight() / 4 - vSpacing - vButtonHeigth, vButtonsWidth, vButtonHeigth);

	mGainSlider.setBounds(getWidth() / 2 - vGainSliderDims / 2, getHeight() - getHeight() / 4, vGainSliderDims, vGainSliderDims);

	addAndMakeVisible(mPlayButton);
	addAndMakeVisible(mResetButton);
	addAndMakeVisible(mGainSlider);
	addAndMakeVisible(mInputPosSlider);
	addAndMakeVisible(mReadPosSlider);
	addAndMakeVisible(mBowPressureSlider);
	addAndMakeVisible(mBowSpeedSlider);
}

void ModalStiffStringView::buttonClicked(juce::Button* apButton)
{
	if (apButton == &mPlayButton)
	{
		mpStiffStringProcessor->SetPlayState(apButton->getToggleState());
	}
	else if (apButton == &mResetButton)
	{
		if (mPlayButton.getToggleState())
		{
			mPlayButton.setToggleState(false, juce::sendNotification);
		}
		mpStiffStringProcessor->ResetStringStates();
	}
}

void ModalStiffStringView::sliderValueChanged(juce::Slider* apSlider)
{
	if (apSlider == &mGainSlider)
	{
		mpStiffStringProcessor->SetGain(static_cast<float>(mGainSlider.getValue()));
	}
	else if (apSlider == &mInputPosSlider)
	{
		//Value is in percentage if string length
		auto vValue = juce::jlimit<float>(0.f, 1.f, mInputPosSlider.getValue() / 100.0);
		mpStiffStringProcessor->SetInputPos(vValue);
	}
	else if (apSlider == &mReadPosSlider)
	{
		//Value is in percentage if string length
		auto vValue = juce::jlimit<float>(0.f, 1.f, mReadPosSlider.getValue() / 100.0);
		mpStiffStringProcessor->SetReadPos(vValue);
	}
	else if (apSlider == &mBowPressureSlider)
	{
		mpStiffStringProcessor->SetBowPressure(static_cast<float>(apSlider->getValue()));
	}
	else if (apSlider == &mBowSpeedSlider)
	{
		mpStiffStringProcessor->SetBowSpeed(static_cast<float>(apSlider->getValue()));
	}
}

void ModalStiffStringView::SetProcessor(std::shared_ptr<ModalStiffStringProcessor> apProcessor)
{
	jassert(apProcessor);
	if (apProcessor)
	{
		mpStiffStringProcessor = apProcessor;
	}
	mStringModesNumber = mpStiffStringProcessor->GetModesNumber();
	SetVisualizationModes();
}

void ModalStiffStringView::SetVisualizationModes()
{
	mVisualizationPoints = 51;
	float vStep = 1.f / (mVisualizationPoints - 1); //position expressed in normalized percentage
	mVisualizationModes = std::vector<std::vector<float>>(mVisualizationPoints, std::vector<float>(mStringModesNumber, 0));
	for (int i = 0; i < mVisualizationPoints; ++i)
	{
		mpStiffStringProcessor->GetModesAtLocation(mVisualizationModes[i], vStep * i);
	}
}


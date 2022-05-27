#include "ModalStiffStringView.h"

//==========================================================================
ModalStiffStringView::ModalStiffStringView()
{
	mPlayButton.addListener(this);
	mResetButton.addListener(this);
	mGainSlider.addListener(this);
	mInputPosSlider.addListener(this);
	mReadPosSlider.addListener(this);

	mGainSlider.setRange(0.0, 5000.0, 0.1);
	mGainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
	mGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);

	mInputPosSlider.setRange(0.0, 1.0, 0.0001);
	mInputPosSlider.setValue(0.733, juce::sendNotification);
	mInputPosSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
	mInputPosSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);

	mReadPosSlider.setRange(0.0, 1.0, 0.0001);
	mReadPosSlider.setValue(0.53, juce::sendNotification);
	mReadPosSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
	mReadPosSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
}

ModalStiffStringView::~ModalStiffStringView()
{
}

//==========================================================================
void ModalStiffStringView::paint(juce::Graphics&)
{
}

void ModalStiffStringView::resized()
{
	int vButtonsWidth = 100;
	int vButtonHeigth = 30;
	int vGainSliderDims = 100;
	int vSpacing = 10;

	mPlayButton.setButtonText("PLAY/PAUSE");
	mPlayButton.setBounds(getWidth() / 2 - getWidth() / 4 - vButtonsWidth / 2, getHeight() - getHeight() / 4 + vButtonHeigth + vSpacing, vButtonsWidth, vButtonHeigth);
	mPlayButton.setClickingTogglesState(true);

	mResetButton.setBounds(getWidth() / 2 + getWidth() / 4 - vButtonsWidth / 2, getHeight() - getHeight() / 4 + vButtonHeigth + vSpacing, vButtonsWidth, vButtonHeigth);
	mResetButton.setButtonText("RESET STATE");
	mResetButton.setClickingTogglesState(false);

	mInputPosSlider.setBounds(getWidth() / 2 - getWidth() / 4 - vButtonsWidth / 2, getHeight() - getHeight() / 4, vButtonsWidth, vButtonHeigth);

	mReadPosSlider.setBounds(getWidth() / 2 + getWidth() / 4 - vButtonsWidth / 2, getHeight() - getHeight() / 4, vButtonsWidth, vButtonHeigth);

	mGainSlider.setBounds(getWidth() / 2 - vGainSliderDims / 2, getHeight() - getHeight() / 4, vGainSliderDims, vGainSliderDims);

	addAndMakeVisible(mPlayButton);
	addAndMakeVisible(mResetButton);
	addAndMakeVisible(mGainSlider);
	addAndMakeVisible(mInputPosSlider);
	addAndMakeVisible(mReadPosSlider);
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
		auto vValue = juce::jlimit<float>(0.f, 100.f, mInputPosSlider.getValue());
		mpStiffStringProcessor->ChangeInputPos(vValue);
	}
	else if (apSlider == &mReadPosSlider)
	{
		//Value is in percentage if string length
		auto vValue = juce::jlimit<float>(0.f, 100.f, mReadPosSlider.getValue());
		mpStiffStringProcessor->ChangeReadPos(vValue);
	}
}

void ModalStiffStringView::SetProcessor(std::shared_ptr<ModalStiffStringProcessor> apProcessor)
{
	jassert(apProcessor);
	if (apProcessor)
	{
		mpStiffStringProcessor = apProcessor;
	}
}


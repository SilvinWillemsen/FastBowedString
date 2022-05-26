#include "ModalStiffStringView.h"

//==========================================================================
ModalStiffStringView::ModalStiffStringView()
{
	mPlayButton.addListener(this);
	mResetButton.addListener(this);
	mGainSlider.addListener(this);

	mGainSlider.setRange(0.0, 5000.0, 0.1);
	mGainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
	mGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
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
	int vSliderDim = 100;

	mPlayButton.setButtonText("PLAY/PAUSE");
	mPlayButton.setBounds(getWidth() / 2 - getWidth() / 4 - vButtonsWidth / 2, getHeight() - getHeight() / 4, vButtonsWidth, vButtonHeigth);
	mPlayButton.setClickingTogglesState(true);

	mResetButton.setBounds(getWidth() / 2 + getWidth() / 4 - vButtonsWidth / 2, getHeight() - getHeight() / 4, vButtonsWidth, vButtonHeigth);
	mResetButton.setButtonText("RESET STATE");
	mResetButton.setClickingTogglesState(false);

	mGainSlider.setBounds(getWidth() / 2 - vSliderDim / 2, getHeight() - getHeight() / 4, vSliderDim, vSliderDim);

	addAndMakeVisible(mPlayButton);
	addAndMakeVisible(mResetButton);
	addAndMakeVisible(mGainSlider);
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
}

void ModalStiffStringView::SetProcessor(std::shared_ptr<ModalStiffStringProcessor> apProcessor)
{
	jassert(apProcessor);
	if (apProcessor)
	{
		mpStiffStringProcessor = apProcessor;
	}
}


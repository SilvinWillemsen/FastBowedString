/*
  ==============================================================================

    ModalStiffStringView.h
    Created: 04/05/2022
    Author:  Riccardo Russo

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ModalStiffStringProcessor.h"

class ModalStiffStringView
    : public juce::Component
    , public juce::Button::Listener
    , public juce::Slider::Listener
{
public:
    ModalStiffStringView();
    ~ModalStiffStringView();

    //==========================================================================
    // juce::Component
    void paint(juce::Graphics&) override;
    void resized() override;

    //==========================================================================
    void buttonClicked(juce::Button* apButton) override;
    void sliderValueChanged(juce::Slider* apSlider) override;

    //==========================================================================
    void SetProcessor(std::shared_ptr<ModalStiffStringProcessor> apProcessor);

private:
    std::shared_ptr<ModalStiffStringProcessor> mpStiffStringProcessor;
    bool mPlayState{ false };

    juce::TextButton mPlayButton;
    juce::TextButton mResetButton;

    juce::Slider mGainSlider;
    juce::Label mGainLabel;

    juce::Slider mInputPosSlider;
    juce::Slider mReadPosSlider;
    juce::Label mInputPosLabel;
    juce::Label mReadPosLabel;

    juce::Slider mBowPressureSlider;
    juce::Label mBowPressureLabel;

    juce::Slider mBowSpeedSlider;
    juce::Label mBowSpeedLabel;

    int mVisualizationPoints{ 0 };
    std::vector<std::vector<float>> mVisualizationModes;

    int mStringModesNumber{ 0 };
    void SetVisualizationModes();
    juce::Path VisualiseState(juce::Graphics& g);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModalStiffStringView)
};


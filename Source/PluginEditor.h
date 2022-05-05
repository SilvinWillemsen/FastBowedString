/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Global.h"
#include "PluginProcessor.h"
#include "Bowed1DWaveFirstOrder.h"
//==============================================================================
/**
*/
class FastBowedStringAudioProcessorEditor  : public juce::AudioProcessorEditor, public Timer
{
public:
    FastBowedStringAudioProcessorEditor (FastBowedStringAudioProcessor&);
    ~FastBowedStringAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    FastBowedStringAudioProcessor& audioProcessor;
    
    std::shared_ptr<Bowed1DWaveFirstOrder> bowed1DWaveFirstOrder;
    std::unique_ptr<Label> dbgLabel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FastBowedStringAudioProcessorEditor)
};

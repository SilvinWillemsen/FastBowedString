/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FastBowedStringAudioProcessorEditor::FastBowedStringAudioProcessorEditor (FastBowedStringAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    // Get the bowed1dWave pointer to put it in the application (see resized())
    bowed1DWave = p.getBowed1DWavePtr();
    
    // Add the bowed1DWave to the application and make it visible (a JUCE must-have)
    addAndMakeVisible (bowed1DWave.get());
    
    // Refresh the graphics at a rate of 15 Hz
    startTimerHz (15);
    
    
    dbgLabel = std::make_unique<Label>();
    dbgLabel->setColour (Label::textColourId, Colours::white);
    dbgLabel->setColour (Label::backgroundColourId, Colours::transparentBlack);
    dbgLabel->setFont (Font (18.0f));

    dbgLabel->setJustificationType (Justification::centred);
    
    addAndMakeVisible (dbgLabel.get());

    setSize (400, 300);
}

FastBowedStringAudioProcessorEditor::~FastBowedStringAudioProcessorEditor()
{
}

//==============================================================================
void FastBowedStringAudioProcessorEditor::paint (juce::Graphics& g)
{
    
}

void FastBowedStringAudioProcessorEditor::resized()
{
    // Position the bowed1DWave in the application (fully encompassing the application bounds)
    if (bowed1DWave != nullptr)
        bowed1DWave->setBounds(getLocalBounds());
    
    dbgLabel->setBounds (0, getHeight() - 50, 100, 50);
}

void FastBowedStringAudioProcessorEditor::timerCallback()
{
    // this function gets called from the JUCE backend at the rate specified by the startTimerHz (see constructor of this class)
    dbgLabel->setText (String (audioProcessor.getCurSample()), dontSendNotification);
    repaint();
}

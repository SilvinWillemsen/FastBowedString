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
#if TIME_DOMAIN_STRING
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    // Get the bowed1dWave pointer to put it in the application (see resized())
    bowed1DWaveFirstOrder = p.getBowed1DWaveFirstOrderPtr();
    
    // Add the bowed1DWave to the application and make it visible (a JUCE must-have)
    addAndMakeVisible (bowed1DWaveFirstOrder.get());
    
    
    dbgLabel = std::make_unique<Label>();
    dbgLabel->setColour (Label::textColourId, Colours::white);
    dbgLabel->setColour (Label::backgroundColourId, Colours::transparentBlack);
    dbgLabel->setFont (Font (18.0f));

    dbgLabel->setJustificationType (Justification::centred);
    
    addAndMakeVisible (dbgLabel.get());
#else
    mpModalStiffString = std::make_unique<ModalStiffStringView>();
    addAndMakeVisible(*mpModalStiffString);
    mpModalStiffString->SetProcessor(p.GetModalStringProcessor());
#endif
    // Refresh the graphics at a rate of 15 Hz
    startTimerHz (15);

    setSize (800, 600);
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
#if TIME_DOMAIN_STRING
    // Position the bowed1DWave in the application (fully encompassing the application bounds)
    if (bowed1DWaveFirstOrder != nullptr)
        bowed1DWaveFirstOrder->setBounds(getLocalBounds());
    
    dbgLabel->setBounds (0, getHeight() - 50, getWidth(), 50);
#else
    mpModalStiffString->setBounds(getLocalBounds());
#endif
}

void FastBowedStringAudioProcessorEditor::timerCallback()
{
#if TIME_DOMAIN_STRING
    // this function gets called from the JUCE backend at the rate specified by the startTimerHz (see constructor of this class)
    dbgLabel->setText (String (audioProcessor.getDebugString()), dontSendNotification);
#endif
    repaint();
}

/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Global.h"
#include "Bowed1DWaveFirstOrder.h"

//==============================================================================
/**
*/
class FastBowedStringAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    FastBowedStringAudioProcessor();
    ~FastBowedStringAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    std::shared_ptr<Bowed1DWaveFirstOrder> getBowed1DWaveFirstOrderPtr() { return bowed1DWaveFirstOrder; };
    
    // Function for the editor to retrieve information about the state of the plugin (current sample number, or "diffsum")
    String getDebugString();
private:
    //==============================================================================
    double fs; // sample rate
    
    std::shared_ptr<Bowed1DWaveFirstOrder> bowed1DWaveFirstOrder;
    
    // Current sample (debugging purposes only)
    unsigned long curSample = 0;
    
    // Current buffer (debugging purposes only)
    unsigned long curBuffer = 0;

    // Cumulative time for calculating the reference, optimised matrix, or optimised vector method (debugging purposes only)
    double cumulativeTimePerBufferRef = 0;
    double cumulativeTimePerBufferOpt = 0;
    double cumulativeTimePerBufferOptVec = 0;
    
    // Average time per sample of calculating the reference, optimised matrix, or optimised vector method (debugging purposes only)
    double avgTimeRef;
    double avgTimeOpt;
    double avgTimeOptVec;

    // Sum of the difference between the refence and the optimised states (debugging purposes only)
    double diffsum;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FastBowedStringAudioProcessor)
};

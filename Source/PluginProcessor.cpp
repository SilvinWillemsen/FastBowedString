/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FastBowedStringAudioProcessor::FastBowedStringAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

FastBowedStringAudioProcessor::~FastBowedStringAudioProcessor()
{
}

//==============================================================================
const juce::String FastBowedStringAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FastBowedStringAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FastBowedStringAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FastBowedStringAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FastBowedStringAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FastBowedStringAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FastBowedStringAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FastBowedStringAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FastBowedStringAudioProcessor::getProgramName (int index)
{
    return {};
}

void FastBowedStringAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FastBowedStringAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    
#if TIME_DOMAIN_STRING
    // Initialise Bowed 1D Wave equation class with k
    bowed1DWaveFirstOrder = std::make_shared<Bowed1DWaveFirstOrder> (1.0 / sampleRate);
#else
    if (!mpModalStiffStringProcessor)
    {
        mpModalStiffStringProcessor = std::make_shared<ModalStiffStringProcessor>(1.0 / sampleRate);
    }
    else if (mSampleRate != sampleRate)
    {
        mpModalStiffStringProcessor->SetTimeStep(1.0 / sampleRate);
    }   
#endif

    // save samplerate and block size
    mSampleRate = sampleRate;
    mBlockSize = samplesPerBlock;
}

void FastBowedStringAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FastBowedStringAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void FastBowedStringAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    //// In case we have more outputs than inputs, this code clears any output
    //// channels that didn't contain input data, (because these aren't
    //// guaranteed to be empty - they may contain garbage).
    //// This is here to avoid people getting screaming feedback
    //// when they first compile a plugin, but obviously you don't need to keep
    //// this code if your algorithm always overwrites all the output channels.
    //for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    //    buffer.clear (i, 0, buffer.getNumSamples());

    float vOutput = 0.0;
    
#if TIME_DOMAIN_STRING
    // Get pointers to output locations
    float* const channelData1 = buffer.getWritePointer(0);
    float* const channelData2 = totalNumOutputChannels > 1 ? buffer.getWritePointer(1) : nullptr;
    std::vector<float* const*> curChannel{ &channelData1, &channelData2 };

    // Run all schemes and compare their runtimes (only makes sense in release mode)
#ifdef RUN_ALL
    // Increment the current buffer (only for time measurement)
    ++curBuffer;

    // Get the current time
    double now = Time::getMillisecondCounterHiRes();

    // Calculate one buffer of the reference method
    for (int i = 0; i < buffer.getNumSamples(); ++i)
        bowed1DWaveFirstOrder->calculateFirstOrderRef();
    cumulativeTimePerBufferRef += (Time::getMillisecondCounterHiRes() - now);
    
    // Calculate average time per sample for the reference method
    avgTimeRef = cumulativeTimePerBufferRef / (curBuffer * buffer.getNumSamples());
    
    now = Time::getMillisecondCounterHiRes();
    
    // Calculate one buffer of the optimised matrix form
    for (int i = 0; i < buffer.getNumSamples(); ++i)
        bowed1DWaveFirstOrder->calculateFirstOrderOpt();
    cumulativeTimePerBufferOpt += (Time::getMillisecondCounterHiRes() - now);
    
    // Calculate average time per sample for the optimised matrix form
    avgTimeOpt = cumulativeTimePerBufferOpt / (curBuffer * buffer.getNumSamples());

    now = Time::getMillisecondCounterHiRes();
    
    // Calculate one buffer of the optimised vector form
    for (int i = 0; i < buffer.getNumSamples(); ++i)
        bowed1DWaveFirstOrder->calculateFirstOrderOptVec();
    cumulativeTimePerBufferOptVec += (Time::getMillisecondCounterHiRes() - now);

    // Calculate average time per sample for the optimised vector form
    avgTimeOptVec = cumulativeTimePerBufferOptVec / (curBuffer * buffer.getNumSamples());

    Logger::getCurrentLogger()->outputDebugString("1: Reference matrix: " + String(avgTimeRef));
    Logger::getCurrentLogger()->outputDebugString("2: Optimized matrix: " + String(avgTimeOpt));
    Logger::getCurrentLogger()->outputDebugString("3: Optimized vector: " + String(avgTimeOptVec));
#else
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        bowed1DWaveFirstOrder->calculateFirstOrderOptVec();
        vOutput = bowed1DWaveFirstOrder->getOutput (0.8); // get output at 0.8L of the string
//        DBG(vOutput);
        for (int channel = 0; channel < totalNumOutputChannels; ++channel)
            curChannel[channel][0][i] = Global::limitOutput (vOutput);

        ++curSample;
    }
#endif
    diffsum = bowed1DWaveFirstOrder->getDiffSum();
#else

    // Get the current time
    //double vNow = Time::getMillisecondCounterHiRes();

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        mpModalStiffStringProcessor->ComputeState();
        vOutput = mpModalStiffStringProcessor->ReadOutput();
        //DBG(Global::limitOutput(vOutput));
        //jassert(vOutput <= 1 && vOutput >= -1);
        for (int channel = 0; channel < totalNumOutputChannels; ++channel)
        {
            auto vpBuffer = buffer.getWritePointer(channel);
            vpBuffer[i] = Global::limitOutput(vOutput);
        }
    }

    //mCumulativeTimePerBufferMod = (Time::getMillisecondCounterHiRes() - vNow);
    //float vRealTime = (1000 / mSampleRate) * mBlockSize;

    //Logger::getCurrentLogger()->outputDebugString("3: Modal: " + String(mCumulativeTimePerBufferMod/vRealTime));

#endif

}

//==============================================================================
bool FastBowedStringAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FastBowedStringAudioProcessor::createEditor()
{
    return new FastBowedStringAudioProcessorEditor (*this);
}

//==============================================================================
void FastBowedStringAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void FastBowedStringAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

std::shared_ptr<ModalStiffStringProcessor> FastBowedStringAudioProcessor::GetModalStringProcessor()
{
    return mpModalStiffStringProcessor;
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FastBowedStringAudioProcessor();
}

String FastBowedStringAudioProcessor::getDebugString()
{
#ifdef RUN_ALL
   return "Diffsum: " + String(diffsum) + " Cursample: " + String(curSample);
#else
   return "Cursample: " + String(curSample);
#endif
}

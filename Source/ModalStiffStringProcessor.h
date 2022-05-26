/*
  ==============================================================================

    ModalStiffStringView.h
    Created: 04/05/2022
    Author:  Riccardo Russo

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Global.h"

class ModalStiffStringProcessor
{
public:
    //==========================================================================
    ModalStiffStringProcessor(double aK);
    ~ModalStiffStringProcessor();

    //==========================================================================
    void SetTimeStep(double aTimeStep);
    void SetPlayState(bool aPlayState);
    void SetGain(float aGain);
    void ResetStringStates();
    void ComputeState();
    float ReadOutput();
    float* GetModalState();
    std::vector<float> GetOutputModes();

private:
    //==========================================================================
    //PlayState
    std::atomic<bool> mPlayState{ false };
    std::atomic<float> mGain{ 0.f };

    //String params
    float mRadius{ 0.f };
    float mDensity{ 0.f };
    float mTension{ 0.f };
    float mArea{ 0.f };
    float mLinDensity{ 0.f };
    float mC{ 0.f };
    float mYoungMod{ 0.f };
    float mInertia{ 0.f };
    float mK{ 0.f };
    float mLength{ 0.f };
    float mExcitPos{ 0.f };
    float mReadPos{ 0.f };
    std::vector<float> mDampCoeffs;

    //==========================================================================
    //String states
    std::vector<std::vector<float>> mStates;
    std::vector<float*> mpStatesPointers;

    //==========================================================================
    //Bow params
    float mFb{ 0.f };
    float mA{ 0.f };
    float mVb{ 0.f };

    //==========================================================================
    //FDS & Modal params
    double mTimeStep{ 0.0 };
    int mModesNumber{ 0 };
    std::vector<float> mEigenFreqs;
    std::vector<float> mModesIn;
    std::vector<float> mModesOut;

    std::vector<int> mT11;
    std::vector<float> mT12;
    std::vector<float> mT21;
    std::vector<int> mT22;

    std::vector<float> mSchurComp;

    std::vector<int> mB11;
    std::vector<float> mB12;
    std::vector<float> mB21;
    std::vector<float> mB22;

    std::vector<float> mZeta2;
    std::vector<float> mB1;
    std::vector<float> mB2;

    std::vector<float> mZ1;
    std::vector<float> mInvAv2;
    std::vector<float> mInvAv1;

    std::vector<float> mY2;
    std::vector<float> mZ2;
    std::vector<float> mInvAb2;
    std::vector<float> mInvAb1;

    //==========================================================================
    //Utility Functions
    float ComputeEigenFreq(int aModeNumber);
    float ComputeMode(float aPos, int aModeNumber);
    float ComputeDampCoeff(float aFreq);

    void RecomputeModesNumber();
    void RecomputeEigenFreqs();
    void RecomputeInModes();
    void RecomputeOutModes();
    void RecomputeDampProfile();

    void InitializeStates();
    void ResetMatrices();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModalStiffStringProcessor)
};

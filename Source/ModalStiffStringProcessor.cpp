#include "ModalStiffStringProcessor.h"

ModalStiffStringProcessor::ModalStiffStringProcessor (double aK) : mTimeStep(aK)
{
    auto vPi = juce::MathConstants<float>::pi;

    mLength = 0.69f;
    mRadius = (float)6.05e-04;
    mDensity = (float)5.3570e3;
    mTension = 112.67f;
    mArea = vPi * mRadius * mRadius;
    mLinDensity = mDensity * mArea;
    mYoungMod = (float)8.6e9;
    mInertia = (vPi * mRadius * mRadius * mRadius * mRadius) / 4;
    mK = sqrt(mYoungMod * mInertia / (mLinDensity * mLength * mLength * mLength * mLength));
    mC = sqrt(mTension / mLinDensity);

    mFb = 10.f;
    mA = 100.f;
    mVb = 0.2f;
    mExcitPos = 0.733 * mLength;
    mReadPos = 0.53 * mLength;

    RecomputeModesNumber();
    RecomputeEigenFreqs();
    RecomputeInModes();
    RecomputeOutModes();
    RecomputeDampProfile();

    InitializeStates();
    ResetMatrices();
}

ModalStiffStringProcessor::~ModalStiffStringProcessor()
{
}

void ModalStiffStringProcessor::SetTimeStep(double aTimeStep)
{
    bool vCurrPlayState = mPlayState;
    if (vCurrPlayState)
    {
        mPlayState.store(false);
    }
    mTimeStep = aTimeStep;
    if (vCurrPlayState)
    {
        mPlayState.store(true);
    }
}

void ModalStiffStringProcessor::SetPlayState(bool aPlayState)
{
    mPlayState.store(aPlayState);
}

void ModalStiffStringProcessor::SetGain(float aGain)
{
    mGain.store(aGain);
}

void ModalStiffStringProcessor::ComputeState()
{
    if (mPlayState.load()){
        //Computing input projection
        float vZeta1 = 0.f;
        for (int i = 0; i < mModesNumber; ++i)
        {
            vZeta1 += mModesIn[i] * mpStatesPointers[0][i + mModesNumber];
        }

        //Computing bow input
        float vEta = vZeta1 - mVb;
        float vD = sqrt(2 * mA) * exp(-mA * vEta * vEta + 0.5);
        float vLambda = vD * (1 - 2 * mA * vEta * vEta);

        float vVt1 = 0.f;
        float vVt2 = 0.f;

        //Computing known terms
        for (int i = 0; i < mModesNumber; ++i)
        {
            float vZeta2 = mModesIn[i] * vZeta1;

            //Notice that the first half of zeta in the matlab code is made of zeroes, 
            //so there is no point of computing multiplications by it
            float vB1 = mB11[i] * mpStatesPointers[0][i] + mB12[i] * mpStatesPointers[0][i + mModesNumber];
            float vB2 = mB21[i] * mpStatesPointers[0][i] + mB22[i] * mpStatesPointers[0][i + mModesNumber] +
                vZeta2 * 0.5f * mTimeStep * mFb * (vLambda - 2 * vD) +
                mTimeStep * mFb * vD * mModesIn[i] * mVb;

            //Computing T^-1*a (see overleaf notes)
            float vZ1 = 0.5f * mTimeStep * mFb * vLambda * mModesIn[i];
            mInvAv2[i] = (1 / mSchurComp[i]) * vZ1;
            mInvAv1[i] = -mT11[i] * mT12[i] * mInvAv2[i];

            //Computing T^-1*[j1;j1] (see overleaf notes)
            float vY2 = mT11[i] * vB1;
            float vZ2 = vB2 - mT21[i] * vY2;
            mInvAb2[i] = (1 / mSchurComp[i]) * vZ2;
            mInvAb1[i] = vY2 - mT11[i] * mT12[i] * mInvAb2[i];

            vVt1 += mModesIn[i] * mInvAv2[i];
            vVt2 += mModesIn[i] * mInvAb2[i];
        }

        float vCoeff = 1 / (1 + vVt1);

        for (int i = 0; i < mModesNumber; ++i)
        {
            mpStatesPointers[1][i] = mInvAb1[i] - vCoeff * mInvAv1[i] * vVt2;
            mpStatesPointers[1][i + mModesNumber] = mInvAb2[i] - vCoeff * mInvAv2[i] * vVt2;
        }

        //Pointers switch
        auto vpStatePointer = mpStatesPointers[0];
        mpStatesPointers[0] = mpStatesPointers[1];
        mpStatesPointers[1] = vpStatePointer;
    }
}

float ModalStiffStringProcessor::ReadOutput()
{
    float vOutputValue = 0.f;
    if (mPlayState.load())
    {
        for (int i = 0; i < mModesNumber; ++i)
        {
            vOutputValue += mModesOut[i] * mpStatesPointers[0][i];
        }
    }
   
    return (mGain.load() * vOutputValue);
}

float* ModalStiffStringProcessor::GetModalState()
{
    return mpStatesPointers[0];
}

std::vector<float> ModalStiffStringProcessor::GetOutputModes()
{
    return mModesOut;
}

float ModalStiffStringProcessor::ComputeEigenFreq(int aModeNumber)
{
    auto vN = aModeNumber * juce::MathConstants<float>::pi / mLength;
    return sqrt((mTension / mLinDensity) * vN * vN + (mYoungMod * mInertia / mLinDensity) * vN * vN * vN * vN);
}

float ModalStiffStringProcessor::ComputeMode(float aPos, int aModeNumber)
{
    return sqrt(2 / mLength) * sin(aModeNumber * juce::MathConstants<float>::pi * aPos / mLength);
}

float ModalStiffStringProcessor::ComputeDampCoeff(float aFreq)
{
    auto vPi = juce::MathConstants<float>::pi;
    float vRhoAir = 1.225f;
    float vMuAir = (float)1.619e-5;
    auto vD0 = -2 * vRhoAir * vMuAir / (mDensity * mRadius * mRadius);
    auto vD1 = -2 * vRhoAir * sqrt(2 * vMuAir) / (mDensity * mRadius);
    auto vD2 = static_cast<float>(-1 / 18000);
    auto vD3 = -0.003f * mYoungMod * mDensity * vPi * vPi * mRadius * mRadius * mRadius * mRadius * mRadius * mRadius / (4 * mTension * mTension);
    return vD0 + vD1 * sqrt(aFreq) + vD2 * aFreq + vD3 * aFreq * aFreq * aFreq;
}

void ModalStiffStringProcessor::RecomputeModesNumber()
{
    int vModesNumber = 1;
    float vLimitFreq = 20e3 * 2 * juce::MathConstants<float>::pi;
    while (true)
    {
        auto vFreq = ComputeEigenFreq(vModesNumber);
        if (vFreq > vLimitFreq) 
        {
            --vModesNumber;
            break;
        }
        ++vModesNumber;
    }
    mModesNumber = vModesNumber;    
}

void ModalStiffStringProcessor::RecomputeEigenFreqs()
{
    mEigenFreqs.resize(mModesNumber);
    for (int i = 0; i < mModesNumber; ++i)
    {
        mEigenFreqs[i] = ComputeEigenFreq(i + 1);
    }
}

void ModalStiffStringProcessor::RecomputeInModes()
{
    mModesIn.resize(mModesNumber);
    for (int i = 0; i < mModesNumber; ++i)
    {
        mModesIn[i] = ComputeMode(mExcitPos, i + 1);
    }
}

void ModalStiffStringProcessor::RecomputeOutModes()
{
    mModesOut.resize(mModesNumber);
    for (int i = 0; i < mModesNumber; ++i)
    {
        mModesOut[i] = ComputeMode(mReadPos, i + 1);
    }
}

void ModalStiffStringProcessor::RecomputeDampProfile()
{
    mDampCoeffs.resize(mModesNumber);
    for (int i = 0; i < mModesNumber; ++i)
    {
        auto vFreq = mEigenFreqs[i];
        mDampCoeffs[i] = - ComputeDampCoeff(vFreq);
    }
}

void ModalStiffStringProcessor::InitializeStates()
{
    // Initialise xVectors
    mStates.resize(2);
    mpStatesPointers.resize(2);
    // initialise states container with two vectors of 0s
    mStates = std::vector<std::vector<float>>(2, std::vector<float>(mModesNumber * 2, 0));
    mpStatesPointers = std::vector<float*>(2, nullptr);
    // initialise pointers to state vectors
    for (int i = 0; i < 2; ++i)
    {
        mpStatesPointers[i] = &mStates[i][0];
    }
}

void ModalStiffStringProcessor::ResetStringStates()
{
    if (mPlayState.load())
    {
        mPlayState.store(false);
    }
    std::fill(mStates[0].begin(), mStates[0].end(), 0);
    std::fill(mStates[1].begin(), mStates[1].end(), 0);
}

void ModalStiffStringProcessor::ResetMatrices()
{
    mT11.resize(mModesNumber);
    mT12.resize(mModesNumber);
    mT21.resize(mModesNumber);
    mT22.resize(mModesNumber);
    mSchurComp.resize(mModesNumber);

    mB11.resize(mModesNumber);
    mB12.resize(mModesNumber);
    mB21.resize(mModesNumber);
    mB22.resize(mModesNumber);

    mZeta2.resize(mModesNumber);
    mB1.resize(mModesNumber);
    mB2.resize(mModesNumber);

    mZ1.resize(mModesNumber);
    mInvAv2.resize(mModesNumber);
    mInvAv1.resize(mModesNumber);

    mY2.resize(mModesNumber);
    mZ2.resize(mModesNumber);
    mInvAb2.resize(mModesNumber);
    mInvAb1.resize(mModesNumber);

    for (int i = 0; i < mModesNumber; ++i)
    {
        mT11[i] = 1;
        mT12[i] = - 0.5f * mTimeStep;
        mT21[i] = - 0.5f * mTimeStep * (-mEigenFreqs[i] * mEigenFreqs[i]);
        mT22[i] = 1;

        mSchurComp[i] = mT22[i] - mT21[i] * (mT11[i] * mT12[i]);

        mB11[i] = 1;
        mB12[i] = 0.5f * mTimeStep;
        mB21[i] = 0.5 * mTimeStep * (-mEigenFreqs[i] * mEigenFreqs[i]);
        mB22[i] = 1 - mTimeStep*mDampCoeffs[i];
    }
}

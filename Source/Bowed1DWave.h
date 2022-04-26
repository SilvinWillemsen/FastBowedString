/*
  ==============================================================================

    Bowed1DWave.h
    Created: 25 Apr 2022 12:20:12pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Global.h"
//#define MODAL // comment out if you want to calculate First-Order System

//==============================================================================
/*
*/
class Bowed1DWave  : public juce::Component
{
public:
    Bowed1DWave (double k);
    ~Bowed1DWave() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    // function to draw the state of the string
    Path visualiseState (Graphics& g, double visualScaling);

    void calculate();
    void updateStates();
    
    float getOutput (float outRatio) { return x.coeffRef ((int)floor(outRatio * N));
//    [(int)floor(outRatio * N)];
        
    };
    
private:
    
    // Time step
    double k;
    // Scheme parameters
    double L, c, h;
    
    // Number of grid points / modes
    int N, NN;
    
    // Bowing variables
    double a;   // free parameter
    double xB;  // Bowing location (as a ratio of the length) (can be made mouse-controlled)
    double vB;  // Bowing velocity (in m/s)
    double Fb;  // Bowing force (in m^2/s^2) (?)
    double eta; // relative velocity between the string and bow (in m/s)
    Eigen::SparseVector<double> zeta, b;
    
    double lambda, d; // noniterative factors
    double outPos; // output location (as a ratio of the length)
    
//    std::vector<std::vector<double>> xStates; // container saving the states of the system
//    std::vector<double*> x; // pointers to the state vectors
    
    Eigen::SparseVector<double> xNext, x;
    Eigen::SparseMatrix<double> I, J, Apre, Bpre, Bmat, zetaZetaT;
    Eigen::SparseMatrix<double> Amat;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Bowed1DWave)
};

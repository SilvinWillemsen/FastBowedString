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
    Path visualiseState (Graphics& g, double visualScaling, double* x, double offset);

    void calculateFirstOrder();
    void updateStates();
    
    float getOutput (float outRatio) { return xRef.data()[N + (int)floor(outRatio * N)]; };
    
    double getDiffSum() { return diffsum; };
private:
    
    // These functions are private such that they can only be called from member functions of this class (in this case, the public "calculateFirstOrder()" function)
    void calculateFirstOrderOptVec(); // Optimised first order system calculation with vectors
    void calculateFirstOrderOpt(); // Optimised first order system calculation
    void calculateFirstOrderRef(); // Reference first order system calculation

    void recalculateZeta(); // recalculate the zeta vector
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
    
    double lambda, d; // noniterative factors
    double outPos; // output location (as a ratio of the length)
    
//    std::vector<std::vector<double>> xStates; // container saving the states of the system
//    std::vector<double*> x; // pointers to the state vectors
    
    
    Eigen::VectorXd xNext, x, xNextRef, xRef, xPaint, xRefPaint;
    
    Eigen::MatrixXd T;
    Eigen::SparseMatrix<double, Eigen::RowMajor> I, J, Tinv, zetaZetaT, TzT;
    Eigen::SparseMatrix<double, Eigen::RowMajor> Amat, Bmat, Apre, Bpre, Ainv;
    Eigen::VectorXd zetaTinv, TinvZeta, b, bx;
    Eigen::SparseVector<double> zeta;
    
    // vector stuff
    std::vector<std::vector<double>> xStates;
    std::vector<double*> xVec;
    std::vector<std::vector<double>> BpreVec, BmatVec, zetaZetaTVec, TinvVec, TzTVec, AinvVec;
    std::vector<double> bxVec, zetaVec, TinvZetaVec;
    int zetaStartIdx, zetaEndIdx;
    bool zetaFlag;
    double zTz;
    
    double diffsum; // sum of the difference between the normal and the test vector
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Bowed1DWave)
};

/*
  ==============================================================================

    Bowed1DWave.cpp
    Created: 25 Apr 2022 12:20:12pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Bowed1DWave.h"

//==============================================================================
Bowed1DWave::Bowed1DWave (double k) : k (k)
{
    L = 0.7;
    c = 300;
    h = c * k; // using h to calculate number of modes
    N = floor (L / h);
    
    h = L / N; // recalculation of h for the non-modal scheme
    
    // calculate number of states in first order system
#ifdef MODAL
    NN = 2 * (N - 1);
#else
    NN = 2 * N - 1;
#endif
    
    a = 100;
    xB = 0.633 * L;
    vB = 0.2;
    Fb = 1;

    outPos = 0.33 * L;

//    xStates.resize (2);
//    x.resize (2);
//    // initialise states container with two vectors of 0s
//    xStates = std::vector<std::vector<double>> (2,
//                                               std::vector<double> (NN, 0));
//    // initialise pointers to state vectors
//    for (int i = 0; i < 2; ++i)
//        x[i] = &xStates[i][0];
    
    xNext = Eigen::VectorXd (NN);
    xNext.setZero();

    x = Eigen::VectorXd (NN);
    x.setZero();
    
    I = Eigen::MatrixXd (NN, NN);
    I.setIdentity();
    J = Eigen::MatrixXd (NN, NN);
    J.setZero();
    for (int i = 0; i < N; ++i)
    {
        J.coeffRef(i,N-1+i) += 1.0 / h;
        J.coeffRef(i+1,N-1+i) += -1.0 / h;
        
        J.coeffRef(N-1+i, i) += -1.0 / h;
        J.coeffRef(N-1+i, i+1) += 1.0 / h;

    }
            
    using namespace Eigen;
    Apre = MatrixXd (NN, NN);
    Bpre = MatrixXd (NN, NN);

    Apre = I / k - J / 2.0;
    Bpre = I / k + J / 2.0;

    Amat = MatrixXd (NN, NN);
    Bmat = MatrixXd (NN, NN);
    Amat.setZero();
    Bmat.setZero();
    zeta = VectorXd (NN);
    zeta.setZero();
#ifdef MODAL
    
#else
    zeta.coeffRef (N + (int)floor(xB * N / L)) = 1.0 / h;
#endif
    
}

Bowed1DWave::~Bowed1DWave()
{
}

void Bowed1DWave::paint (juce::Graphics& g)
{
    // clear the background
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    // choose your favourite colour
    g.setColour(Colours::cyan);
    
    // draw the state
    g.strokePath(visualiseState (g, 500), PathStrokeType(2.0f));
    
}


Path Bowed1DWave::visualiseState (Graphics& g, double visualScaling)
{
    // String-boundaries are in the vertical middle of the component
    double stringBoundaries = getHeight() / 2.0;
    
    // initialise path
    Path stringPath;
    
    // start path
    stringPath.startNewSubPath (0, 0 * visualScaling + stringBoundaries);
    
    double spacing = getWidth() / static_cast<double>(N);
    double xLoc = spacing;
    
    for (int l = 1; l <= N; l++) // if you don't save the boundaries use l < N
    {
        // Needs to be -u, because a positive u would visually go down
        float newY = -x[l+N-1] * visualScaling + stringBoundaries;
        
        // if we get NAN values, make sure that we don't get an exception
        if (isnan(newY))
            newY = 0;
        
        stringPath.lineTo (xLoc, newY);
        xLoc += spacing;
    }
    // if you don't save the boundaries, and add a stringPath.lineTo (x, getWidth()) here to end the statedrawing

    return stringPath;
}


void Bowed1DWave::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}

void Bowed1DWave::calculate()
{
    
    zetaZetaT = zeta * zeta.transpose();
#ifdef MODAL
    
#else
    double bowLoc = xB * N;
    eta = h * 1.0 / h * Global::cubicInterpolation((double*)&x[1], floor(bowLoc), bowLoc - floor(bowLoc)) - vB;
#endif
    
    lambda = sqrt(2.0*a) * (1.0 - 2.0 * a * eta * eta) * exp(-a * eta * eta + 0.5);
    d = sqrt(2.0 * a) * exp(-a * eta * eta + 0.5);
    
#ifdef MODAL
#else
    
    Amat = Apre + (Fb * h * 0.5 * lambda * zetaZetaT);
    Bmat = Bpre + (Fb * h * (0.5 * lambda - d) * zetaZetaT);
    
    // Attempt cholesky decomposition
    using namespace Eigen;
    PartialPivLU<MatrixXd> dec(Amat);
    VectorXd b = Bmat * x + Fb * zeta * d * vB;
    xNext = dec.solve (b);

#endif
    
}

void Bowed1DWave::updateStates()
{
    // pointer switch (much faster than copying the states in MATLAB);
//    double* xTmp = x[1];
//    x[1] = x[0];
//    x[0] = xTmp;
    
    x = xNext;
}

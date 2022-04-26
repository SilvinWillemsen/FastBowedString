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
    using namespace Eigen;
    
    xNext = SparseVector<double> (NN);
    xNext.setZero();

    x = SparseVector<double> (NN);
    x.setZero();
    
    I = SparseMatrix<double> (NN, NN);
    I.setIdentity();
    J = SparseMatrix<double> (NN, NN);
    J.setZero();
    for (int i = 0; i < N-1; ++i)
    {
        // top right quadrant
        J.coeffRef(i,N+i) += c / h;
        J.coeffRef(i+1,N+i) += -c / h;
        
        // bottom left quadrant
        J.coeffRef(N+i, i) += -c / h;
        J.coeffRef(N+i, i+1) += c / h;

    }
    std::cout << J << std::endl;
    using namespace Eigen;
    Apre = SparseMatrix<double> (NN, NN);
    Bpre = SparseMatrix<double> (NN, NN);

    Apre = I / k - J / 2.0;
    Bpre = I / k + J / 2.0;

    Amat = SparseMatrix<double> (NN, NN);
    Bmat = SparseMatrix<double> (NN, NN);
    Amat.setZero();
    Bmat.setZero();
    zeta = SparseVector<double> (NN);
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
    g.strokePath(visualiseState (g, 100000), PathStrokeType(2.0f));
    
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
    
    for (int l = 1; l < N; l++)
    {
        // Needs to be -x, because a positive x would visually go down
        float newY = -x.coeffRef (l+N-1) * visualScaling + stringBoundaries;
        
        // if we get NAN values, make sure that we don't get an exception
        if (isnan(newY))
            newY = 0;
        
        stringPath.lineTo (xLoc, newY);
        xLoc += spacing;
    }
    stringPath.lineTo (xLoc, stringBoundaries);
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
    double bowLoc = xB * N / L;// could be made user-controlled
//    eta = h * 1.0 / h * Global::cubicInterpolation((double*)&x[1], floor(bowLoc), bowLoc - floor(bowLoc)) - vB;
    eta = h * 1.0 / h * x.coeffRef(N + floor(bowLoc)) - vB;
#endif
    
    lambda = sqrt(2.0*a) * (1.0 - 2.0 * a * eta * eta) * exp(-a * eta * eta + 0.5);
    d = sqrt(2.0 * a) * exp(-a * eta * eta + 0.5);
    
#ifdef MODAL
#else
    
    Amat = Apre + (Fb * h * 0.5 * lambda * zetaZetaT);
    Bmat = Bpre + (Fb * h * (0.5 * lambda - d) * zetaZetaT);
    
    using namespace Eigen;
//    PartialPivLU<MatrixXd> dec(Amat);
//    VectorXd b = Bmat * x + Fb * zeta * d * vB;
//    xNext = dec.solve (b);

    SparseLU<SparseMatrix<double>, COLAMDOrdering<int>> solver;

    b = Bmat * x + Fb * zeta * d * vB;
    
    // Make sure the matrix is compressed (see SparseLU documentation)
    Amat.makeCompressed();
    
    // Compute the ordering permutation vector from the structural pattern of A
    solver.analyzePattern(Amat);
    
    // Compute the numerical factorization
    solver.factorize(Amat);

    //Use the factors to solve the linear system
    xNext = solver.solve(b);
    
    int idx = N + floor(bowLoc);
//    std::cout << xNext.coeff (idx) << std::endl;
//    for (int i = N; i < xNext.size(); ++i)
//        std::cout << i << " = " <<  xNext.coeff (i) << std::endl;
//    std::cout << std::endl;
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

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
    Fb = 5;

    outPos = 0.33 * L;

    // Initialise xVectors
    xStates.resize (2);
    xVec.resize (2);
    // initialise states container with two vectors of 0s
    xStates = std::vector<std::vector<double>> (2,
                                               std::vector<double> (NN, 0));
    // initialise pointers to state vectors
    for (int i = 0; i < 2; ++i)
        xVec[i] = &xStates[i][0];
    
    
    // Initialise x for optimised algorithm
    using namespace Eigen;
    
    xNext = VectorXd (NN);
    xNext.setZero();

    x = VectorXd (NN);
    x.setZero();
      
    // Initialise x for reference algorithm
    xNextRef = VectorXd (NN);
    xNextRef.setZero();

    xRef = VectorXd (NN);
    xRef.setZero();

    I = SparseMatrix<double, RowMajor> (NN, NN);
    I.setIdentity();
    J = SparseMatrix<double, RowMajor> (NN, NN);
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

    using namespace Eigen;
    
    T = I / k - J / 2.0;
    
    Tinv = T.inverse().sparseView();
    
    Tinv.prune (1e-8); // test the prune value and whether it makes it faster..
    
    TzT = SparseMatrix<double> (NN, NN);

    // Vector forms
    TinvVec = std::vector<std::vector<double>> (NN,
                                               std::vector<double> (NN, 0));
    for (int i = 0; i < NN; ++i)
        for (int j = 0; j < NN; ++j)
            TinvVec[i][j] = Tinv.coeff(i, j);
    TinvZetaVec = std::vector<double> (NN, 0);
    
    Apre = I / k - J / 2;
    Bpre = I / k + J / 2;
    
    Amat = SparseMatrix<double, RowMajor> (NN, NN);
    Bmat = SparseMatrix<double, RowMajor> (NN, NN);
    Amat.setZero();
    Bmat.setZero();

    zeta = SparseVector<double> (NN);
    zeta.setZero();
    
    bx = SparseVector<double> (NN);
    bx.setZero();

    bxVec = std::vector<double> (NN, 0);
    zetaVec = std::vector<double> (NN, 0);
    
    TzTVec  = std::vector<std::vector<double>> (NN,
                                                std::vector<double> (NN, 0));
    AinvVec  = std::vector<std::vector<double>> (NN,
                                                std::vector<double> (NN, 0));

#ifdef MODAL
    
#else
    zeta.coeffRef(N + (int)floor(xB * N / L)) = 1.0 / h;
    zetaVec[N + (int)floor(xB * N / L)]= 1.0 / h;
    recalculateZeta(); // if done in the loop this can be excluded here
#endif
    
    BpreVec = std::vector<std::vector<double>> (NN,
                                               std::vector<double> (NN, 0));
    BmatVec = std::vector<std::vector<double>> (NN,
                                               std::vector<double> (NN, 0));
    
    for (int i = 0; i < NN; ++i)
        for (int j = 0; j < NN; ++j)
            BpreVec[i][j] = Bpre.coeff(i, j);

}

Bowed1DWave::~Bowed1DWave()
{
}

void Bowed1DWave::paint (juce::Graphics& g)
{
    // clear the background
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    // draw the state of the optimised algorithm
    g.setColour (Colours::cyan);
    g.strokePath (visualiseState (g, 50000, &x.coeffRef (0), -0.2 * getHeight()), PathStrokeType(2.0f));
    
    // draw the state of the vector
    g.setColour (Colours::yellow);
    g.strokePath (visualiseState (g, 50000, xVec[1], 0.2 * getHeight()), PathStrokeType(2.0f));

    // draw the state of the reference
    g.setColour (Colours::green);
    g.strokePath (visualiseState (g, 50000, &xRef.coeffRef (0), 0.2 * getHeight()), PathStrokeType(2.0f));

}


Path Bowed1DWave::visualiseState (Graphics& g, double visualScaling, double* x, double offset)
{
    // String-boundaries are in the vertical middle of the component
//    double offset = getHeight() * 0.02;
    double stringBoundaries = getHeight() / 2.0 + offset;
    
    // initialise path
    Path stringPath;
    
    // start path
    stringPath.startNewSubPath (0, 0 * visualScaling + stringBoundaries);
    
    double spacing = getWidth() / static_cast<double>(N);
    double xLoc = spacing;
    
    for (int l = 1; l < N; l++)
    {
        // Needs to be -x, because a positive x would visually go down
        float newY = -x[l+N-1] * visualScaling + stringBoundaries;
        
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

void Bowed1DWave::recalculateZeta()
{
    for (int i = 0; i < NN; ++i)
    {
        if (zetaVec[i] != 0)
        {
            zetaFlag = true;
            zetaStartIdx = i;
        }
        if (zetaFlag && zetaVec[i] == 0)
        {
            zetaFlag = false;
            zetaEndIdx = i;
        }
    }
    
    zetaZetaT = zeta * zeta.transpose();
    
    zetaZetaTVec = std::vector<std::vector<double>> (NN, std::vector<double> (NN, 0));
    for (int i = 0; i < NN; ++i)
        for (int j = 0; j < NN; ++j)
            zetaZetaTVec[i][j] = zetaZetaT.coeff (i, j);
    
    // T^{-1}z
    for (int i = 0; i < NN; ++i)
    {
        TinvZetaVec[i] = 0;
        for (int j = 0; j < NN; ++j)
            TinvZetaVec[i] += TinvVec[i][j] * zetaVec[j];
    }
    
    // Sherman-Morrison
    zTz = 0;
    for (int i = 0; i < NN; ++i)
        zTz += zetaVec[i] * TinvZetaVec[i];
    
    // T^{-1}z
    TinvZeta = Tinv * zeta;
    
    TzT = (TinvZeta * zeta.transpose() * Tinv);
    for (int i = 0; i < NN; ++i)
        for (int j = 0; j < NN; ++j)
            TzTVec[i][j] = TzT.coeff (i, j);

}


void Bowed1DWave::calculateFirstOrder()
{
//    recalculateZeta();
    
    double now = Time::getMillisecondCounterHiRes();
    calculateFirstOrderRef(); // reference
    std::cout << "1: Reference matrix: " << String (Time::getMillisecondCounterHiRes() - now) << std::endl;

    now = Time::getMillisecondCounterHiRes();
    calculateFirstOrderOpt(); // optimised
    std::cout << "2: Optimized matrix: " << String (Time::getMillisecondCounterHiRes() - now) << std::endl;
        
    now = Time::getMillisecondCounterHiRes();
    calculateFirstOrderOptVec(); // optimised with vector
    std::cout << "3: Optimized vector: " << String (Time::getMillisecondCounterHiRes() - now) << std::endl;


    diffsum = 0;
    // using xVec[1] here for the vector version because the pointer switch has been done already
    for (int i = 0; i < NN; ++i)
        diffsum += (xNext.coeff(i) - xNextRef.coeff (i));
    
}

void Bowed1DWave::calculateFirstOrderOptVec()
{
    double bowLoc = xB * N / L; // xB can be made user-controlled
//    eta = h * 1.0 / h * Global::cubicInterpolation((double*)&x[1], floor(bowLoc), bowLoc - floor(bowLoc)) - vB;
    eta = h * 1.0 / h * xVec[1][N + (int)floor(bowLoc)] - vB; // should include zeta here as well
    
    // Non-iterative coefficients
    lambda = sqrt(2.0*a) * (1.0 - 2.0 * a * eta * eta) * exp(-a * eta * eta + 0.5);
    d = sqrt(2.0 * a) * exp(-a * eta * eta + 0.5);
    
    double invDiv = 1.0 + Fb * h * lambda * 0.5 * zTz;
    double divTerm = (Fb * h * lambda * 0.5) / invDiv;
    for (int i = 0; i < NN; ++i)
        for (int j = 0; j < NN; ++j)
            AinvVec[i][j] = TinvVec[i][j] - TzTVec[i][j] * divTerm;
    
    // B matrix
    BmatVec = BpreVec;
//    for (int i = 0; i < NN; ++i)
//        for (int j = 0; j < NN; ++j)
    for (int i = zetaStartIdx; i < zetaEndIdx; ++i)
        for (int j = zetaStartIdx; j < zetaEndIdx; ++j)
            BmatVec[i][j] += Fb * h * (0.5 * lambda - d) * zetaZetaTVec[i][j];
    
    // Calculate x^{n+1}
    for (int i = 0; i < NN; ++i)
    {
        bxVec[i] = 0;
        for (int j = 0; j < NN; ++j)
            bxVec[i] += (BmatVec[i][j] * xVec[1][j]);
    }
    for (int i = zetaStartIdx; i < zetaEndIdx; ++i)
        bxVec[i] += Fb * zetaVec[i] * d * vB;
    
    for (int i = 0; i < NN; ++i)
    {
        xVec[0][i] = 0;
        for (int j = 0; j < NN; ++j)
            xVec[0][i] += AinvVec[i][j] * bxVec[j];
    }
    
    // Update states here
    double* xTmp = xVec[1];
    xVec[1] = xVec[0];
    xVec[0] = xTmp;
    
}

void Bowed1DWave::calculateFirstOrderOpt()
{
    double bowLoc = xB * N / L; // xB can be made user-controlled
//    eta = h * 1.0 / h * Global::cubicInterpolation((double*)&x[1], floor(bowLoc), bowLoc - floor(bowLoc)) - vB;
    eta = h * 1.0 / h * x.data()[N + (int)floor(bowLoc)] - vB;
    
    // Non-iterative coefficients
    lambda = sqrt(2.0*a) * (1.0 - 2.0 * a * eta * eta) * exp(-a * eta * eta + 0.5);
    d = sqrt(2.0 * a) * exp(-a * eta * eta + 0.5);

    // Sherman-Morrison
    double invDiv = 1.0 + Fb * h * lambda * 0.5 * zTz;
    double divTerm = (Fb * h * lambda * 0.5) / invDiv;

    Ainv = Tinv - TzT * divTerm;
    
    // B matrix
    Bmat = Bpre + (Fb * h * (0.5 * lambda - d) * zetaZetaT);
    
    // Calculate x^{n+1}
    xNext = Ainv * (Bmat * x + Fb * zeta * d * vB);

    // Update states here
    x = xNext;
    
}

void Bowed1DWave::calculateFirstOrderRef()
{
    
    double bowLoc = xB * N / L; // xB can be made user-controlled
    eta = h * 1.0 / h * xRef.data()[N + (int)floor(bowLoc)] - vB;

    lambda = sqrt(2.0*a) * (1.0 - 2.0 * a * eta * eta) * exp(-a * eta * eta + 0.5);
    d = sqrt(2.0 * a) * exp(-a * eta * eta + 0.5);

    Bmat = Bpre + (Fb * h * (0.5 * lambda - d) * zetaZetaT);
    
    /// Linear system solve ///
    using namespace Eigen;
    SparseLU<SparseMatrix<double>, COLAMDOrdering<int>> solver;

    // Matrix to invert
    Amat = Apre + (Fb * h * 0.5 * lambda * zetaZetaT);
    
    // Right hand side
    b = Bmat * xRef + Fb * zeta * d * vB;

    // For the below, see SparseLU documentation on eigen.com
    Amat.makeCompressed();
    solver.analyzePattern(Amat);
    solver.factorize(Amat);
    
    // Solve the system for x^{n+1}
    xNextRef = solver.solve(b);
       
    // Update states here
    xRef = xNextRef;
}

void Bowed1DWave::updateStates()
{
    // For comparison between the various implementations this function is unused now..
    
    // pointer switch (much faster than copying the states in MATLAB);
//    double* xTmp = x[1];
//    x[1] = x[0];
//    x[0] = xTmp;
    
//    x = xNext;
//    xRef = xNextRef;
}

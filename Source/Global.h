/*
  ==============================================================================

    Global.h
    Created: 25 Apr 2022 12:37:28pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once
#include "../eigen/Eigen/Eigen"
//#define RUN_ALL  // define this macro if you want to run all methods (reference, optimised matrix and optimised vector)

namespace Global
{
    
    static double cubicInterpolation (double* xVec, int l, double alpha)
    {
        return xVec[l - 1] * (alpha * (alpha - 1) * (alpha - 2)) / -6.0
        + xVec[l] * ((alpha - 1) * (alpha + 1) * (alpha - 2)) / 2.0
        + xVec[l + 1] * (alpha * (alpha + 1) * (alpha - 2)) / -2.0
        + xVec[l + 2] * (alpha * (alpha + 1) * (alpha - 1)) / 6.0;
    }

    static void cubicExtrapolation (double* xVec, int l, double alpha, double val)
    {
        xVec[l - 1] = xVec[l - 1] + val * (alpha * (alpha - 1) * (alpha - 2)) / -6.0;
        xVec[l] = xVec[l] + val * ((alpha - 1) * (alpha + 1) * (alpha - 2)) / 2.0;
        xVec[l + 1] = xVec[l + 1] + val * (alpha * (alpha + 1) * (alpha - 2)) / -2.0;
        xVec[l + 2] = xVec[l + 2] + val * (alpha * (alpha + 1) * (alpha - 1)) / 6.0;

    }

    static float limitOutput (float x)
    {
        if (x > 1.0)
            return 1.0;
        else if (x < -1.0)
            return -1.0;
        else
            return x;
    }
};

#pragma once

#include <math.h>

/*
 * inverse cdf by Peter J. Acklam (Could not find anything on the matter but everybody seems to agree about him designing it)
*/
static double inverse_normal_cdf(double p);
static double inverse_normal_cdf(double p) {
    // Coefficients in rational approximations
    const double a1 = -39.6968302866538, a2 = 220.946098424521, a3 = -275.928510446969;
    const double a4 = 138.357751867269, a5 = -30.6647980661472, a6 = 2.50662827745924;
    const double b1 = -54.4760987982241, b2 = 161.585836858041, b3 = -155.698979859887;
    const double b4 = 66.8013118877197, b5 = -13.2806815528857;
    const double c1 = -0.00778489400243029, c2 = -0.322396458041136;
    const double c3 = -2.40075827716184, c4 = -2.54973253934373;
    const double c5 = 4.37466414146497, c6 = 2.93816398269878;
    const double d1 = 0.00778469570904146, d2 = 0.32246712907004;
    const double d3 = 2.44513413714299, d4 = 3.75440866190742;

    // Define break-points.
    const double p_low = 0.02425;
    const double p_high = 1.0 - p_low;

    // Rational approximation for lower region:
    double q, r;
    if (p < p_low) {
        q = sqrt(-2.0 * log(p));
        return (((((c1 * q + c2) * q + c3) * q + c4) * q + c5) * q + c6) /
               ((((d1 * q + d2) * q + d3) * q + d4) * q + 1.0);
    }

    // Rational approximation for upper region:
    if (p > p_high) {
        q = sqrt(-2.0 * log(1.0 - p));
        return -(((((c1 * q + c2) * q + c3) * q + c4) * q + c5) * q + c6) /
               ((((d1 * q + d2) * q + d3) * q + d4) * q + 1.0);
    }

    // Rational approximation for central region:
    q = p - 0.5;
    r = q * q;
    return (((((a1 * r + a2) * r + a3) * r + a4) * r + a5) * r + a6) * q /
           (((((b1 * r + b2) * r + b3) * r + b4) * r + b5) * r + 1.0);
}
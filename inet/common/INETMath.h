//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INETMATH_H
#define __INET_INETMATH_H

//
// Support functions for mathematical operations
//

#include <cmath>
#include <limits>

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * @brief Support functions for mathematical operations.
 *
 * This namespace contains all kind of mathematical support functions
 */
namespace math {

// file: INETMath.h , namespace math a class helyett
/* Windows math.h doesn't define the the following variables: */
#ifndef M_E
#define M_E           2.7182818284590452354
#endif // ifndef M_E

#ifndef M_LOG2E
#define M_LOG2E       1.4426950408889634074
#endif // ifndef M_LOG2E

#ifndef M_LOG10E
#define M_LOG10E      0.43429448190325182765
#endif // ifndef M_LOG10E

#ifndef M_LN2
#define M_LN2         0.69314718055994530942
#endif // ifndef M_LN2

#ifndef M_LN10
#define M_LN10        2.30258509299404568402
#endif // ifndef M_LN10

#ifndef M_PI
#define M_PI          3.14159265358979323846
#endif // ifndef M_PI

#ifndef M_PI_2
#define M_PI_2        1.57079632679489661923
#endif // ifndef M_PI_2

#ifndef M_PI_4
#define M_PI_4        0.78539816339744830962
#endif // ifndef M_PI_4

#ifndef M_1_PI
#define M_1_PI        0.31830988618379067154
#endif // ifndef M_1_PI

#ifndef M_2_PI
#define M_2_PI        0.63661977236758134308
#endif // ifndef M_2_PI

#ifndef M_2_SQRTPI
#define M_2_SQRTPI    1.12837916709551257390
#endif // ifndef M_2_SQRTPI

#ifndef M_SQRT2
#define M_SQRT2       1.41421356237309504880
#endif // ifndef M_SQRT2

#ifndef M_SQRT1_2
#define M_SQRT1_2     0.70710678118654752440
#endif // ifndef M_SQRT1_2

/* Constant for comparing doubles. Two doubles at most epsilon apart
   are declared equal.*/
#ifndef EPSILON
#define EPSILON    0.001
#endif // ifndef EPSILON

#define qNaN       std::numeric_limits<double>::quiet_NaN()
#define sNaN       std::numeric_limits<double>::signaling_NaN()
#define NaN        qNaN

/**
 * Returns the rest of a whole-numbered division.
 */
inline double mod(double dividend, double divisor)
{
    double i;
    return modf(dividend / divisor, &i) * divisor;
}

/**
 * Returns the result of a whole-numbered division.
 */
inline double div(double dividend, double divisor)
{
    double i;
    modf(dividend / divisor, &i);
    return i;
}

/**
 * Returns the remainder r on division of dividend a by divisor n,
 * using floored division. The remainder r has the same sign as the divisor n.
 */
inline double modulo(double a, double n) { return a - n * floor(a / n); }

/**
 * Tests whether two doubles are close enough to be declared equal.
 * Returns true if parameters are at most epsilon apart, false
 * otherwise
 */
inline bool close(double one, double two) { return fabs(one - two) < EPSILON; }

/**
 * Returns 0 if i is close to 0, 1 if i is positive and greater than epsilon,
 * or -1 if it is negative and less than epsilon.
 */
inline int stepfunction(double i) { return (i > EPSILON) ? 1 : close(i, 0) ? 0 : -1; };

inline double step(double a, double b) { return b < a ? 0.0 : 1.0; };

inline double clamp(double v, double l, double u) { return v < l ? l : v > u ? u : v; }

/**
 * Returns 1 if the parameter is greater than zero, -1 if less than zero, 0 otherwise.
 */
inline int sign(double v) { return (0 < v) - (v < 0); };

/**
 * Returns an integer that corresponds to rounded double parameter
 */
inline int round(double d) { return static_cast<int>(ceil(d - 0.5)); }

/**
 * Discards the fractional part of the parameter, e.g. -3.8 becomes -3
 */
inline double floorToZero(double d) { return (d >= 0.0) ? floor(d) : ceil(d); }

/**
 * Converts a dB value to fraction.
 */
inline double dB2fraction(double dB) { return pow(10.0, dB / 10.0); }

/**
 * Convert a fraction value to dB.
 */
inline double fraction2dB(double fraction) { return 10 * log10(fraction); }

/**
 * Converts a dBmW value to mW.
 */
inline double dBmW2mW(double dBm) { return pow(10.0, dBm / 10.0); }

/**
 * Convert a mW value to dBmW.
 */
inline double mW2dBmW(double mW) { return 10.0 * log10(mW); }

/**
 * Converts a dBmW/MHz value into W/Hz.
 */
inline double dBmWpMHz2WpHz(double dBmWpMHz) { return pow(10.0, dBmWpMHz / 10.0) / 1000000.0 / 1000.0; }

/**
 * Convert a W/Hz value to dBmW/MHz.
 */
inline double wpHz2dBmWpMHz(double wpHz) { return 10.0 * log10(wpHz * 1000000.0 * 1000.0); }

/**
 * Convert a degree value to radian.
 */
inline double deg2rad(double deg) { return deg * M_PI / 180; }

/**
 * Convert a radian value to degree.
 */
inline double rad2deg(double rad) { return rad * 180 / M_PI; }

/**
 * Calcutes inverse cosine (output in rad)
  */
inline double arcos(double theta) { return acos(theta); }

/**
 * Implementation of the n choose k (binomial coefficient) function, from the MiXiM Framework
 * Author Karl Wessel
 */
inline double n_choose_k(int n, int k) {
    if (n < k)
        return 0.0;

    const int iK = (k << 1) > n ? n - k : k;
    const double dNSubK = (n - iK);
    int i = 1;
    double dRes = i > iK ? 1.0 : (dNSubK + i);

    for (++i; i <= iK; ++i) {
        dRes *= dNSubK + i;
        dRes /= i;
    }
    return dRes;
}

/**
 * This function properly and symmetrically handles NaNs in contrast with std::min and std::fmin.
 * For example, the minimum of NaN and 1 must be NaN independently of the argument order.
 * See 'Not a number' section at https://2pi.dk/2016/05/ieee-min-max
 */
template<typename T>
inline const T minnan(const T& a, const T& b) {
static_assert(!std::is_integral<T>::value, "minnan() is only meant for doubles and double based units, use std::min() for integers");
    if (a < b)
        return a;
    else if (b < a)
        return b;
    else if (a == b)
        return a;
    else
        return T(NaN);
}

/**
 * This function properly and symmetrically handles NaNs in contrast with std::max and std::fmax.
 * For example, the maximum of NaN and 1 must be NaN independently of the argument order.
 * See 'Not a number' section at https://2pi.dk/2016/05/ieee-min-max
 */
template<typename T>
inline const T maxnan(const T& a, const T& b) {
static_assert(!std::is_integral<T>::value, "maxnan() is only meant for doubles and double based units, use std::max() for integers");
    if (a > b)
        return a;
    else if (b > a)
        return b;
    else if (a == b)
        return a;
    else
        return T(NaN);
}

} // namespace math

} // namespace inet

#endif


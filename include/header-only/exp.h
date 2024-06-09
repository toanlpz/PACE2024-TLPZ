#pragma once

#include <stdint.h>

/*
    found on https://stackoverflow.com/questions/47025373/fastest-implementation-of-the-natural-exponential-function-using-sse
*/ 
static inline float fastExp4(register float x);
static inline float fastExp4(register float x)  // quartic spline approximation
{
    union { float f; int32_t i; } reinterpreter;
    reinterpreter.i = (int32_t)(12102203.0f*x) + 127*(1 << 23);
    int32_t m = (reinterpreter.i >> 7) & 0xFFFF;  // copy mantissa
    // empirical values for small maximum relative error (1.21e-5):
    reinterpreter.i += (((((((((((3537*m) >> 16) + 13668)*m) >> 18) + 15817)*m) >> 14) - 80470)*m) >> 11);
    return reinterpreter.f;
}

/*
* C - version of fast math by ncruces
* https://github.com/ncruces/fastmath/blob/main/fast.go
*/
#ifndef __FAST_MATH_H
#define __FAST_MATH_H
#include <stdint.h>

/**
 * FAST INVERSE SQUARE ROOT (RSqrt)
 * Mathematical name: Reciprocal Square Root Approximation
 * 
 * The famous Quake III Arena fast inverse square root algorithm variant.
 * Approximates 1/sqrt(x) using bit manipulation.
 */
float rcpr_sqrt(float x) {
    uint32_t i = *(uint32_t*)&x;
    i = 0x5f37642f - (i >> 1);
    return *(float*)&i;
}

/**
 * FAST SQUARE ROOT
 * Mathematical name: Square Root Approximation
 * 
 * Approximates sqrt(x) using bit manipulation.
 */
float fast_sqrt(float x) {
    uint32_t i = *(uint32_t*)&x;
    i = 0x1fbb4f2e + (i >> 1);
    return *(float*)&i;
}

/**
 * FAST CUBE ROOT
 * Mathematical name: Cube Root Approximation
 * 
 * Approximates cbrt(x) using bit manipulation.
 */
float fast_cbrt(float x) {
    uint32_t i = *(uint32_t*)&x;
    i = 0x2a51067f + i/3;
    return *(float*)&i;
}

/**
 * FAST RECIPROCAL (1/x)
 * Mathematical name: Reciprocal Approximation
 * 
 * Approximates 1/x using bit manipulation.
 */
float fast_rcpr(float x) {
    uint32_t i = *(uint32_t*)&x;
    i = 0x7ef311c2 - i;
    return *(float*)&i;
}

/**
 * FAST RECIPROCAL CUBE ROOT
 * Mathematical name: Reciprocal Cube Root Approximation
 * 
 * Approximates 1/cbrt(x) using bit manipulation.
 */
float rcpr_cbrt(float x) {
    uint32_t i = *(uint32_t*)&x;
    i = 0x54a232a3 - i/3;
    return *(float*)&i;
}

/**
 * FAST LOGARITHM (base arbitrary)
 * Mathematical name: Logarithm Approximation (Base B)
 * 
 * Approximates log_b(x) using exponent manipulation.
 * Note: This is a very rough approximation.
 */
float fast_log(float x, float b) {
    int32_t i = *(int32_t*)&x;
    i = i - 0x3f800000;  // Remove bias from exponent
    int32_t j = *(int32_t*)&b;
    j = j - 0x3f800000;
    return (float)i / (float)j;
}

/**
 * FAST BINARY LOGARITHM
 * Mathematical name: Binary Logarithm Approximation
 * 
 * Approximates log2(x) using exponent manipulation.
 */
float fast_log2(float x) {
    int32_t i = *(int32_t*)&x;
    x = (float)(i - 0x3f800000);
    i = *(int32_t*)&x;
    i = i - 0xb800000;
    return *(float*)&i;
}

/**
 * FAST NATURAL LOGARITHM
 * Mathematical name: Natural Logarithm Approximation
 * 
 * Approximates ln(x) using exponent manipulation.
 * Multiplier ≈ 1/2^23 * ln(2) = 8.262958e-8
 */
float fast_loge(float x) {
    int32_t i = *(int32_t*)&x;
    i = i - 0x3f800000;  // Extract exponent bias
    return (float)i * 8.262958e-8f;  // Scale to natural log
}

/**
 * FAST COMMON LOGARITHM
 * Mathematical name: Common Logarithm (Base 10) Approximation
 * 
 * Approximates log10(x) using exponent manipulation.
 * Multiplier ≈ 1/2^23 * log10(2) = 3.5885572e-8
 */
float fast_log10(float x) {
    int32_t i = *(int32_t*)&x;
    i = i - 0x3f800000;
    return (float)i * 3.5885572e-8f;
}

/**
 * FAST POWER FUNCTION
 * Mathematical name: Exponentiation Approximation (x^p)
 * 
 * Approximates x^p using exponent manipulation.
 */
float fast_pow(float x, float p) {
    int32_t i = *(int32_t*)&x;
    i = i - 0x3f800000;
    i = (int32_t)((float)i * p);
    i = i + 0x3f800000;
    return *(float*)&i;
}

/**
 * FAST POWER OF TWO
 * Mathematical name: Binary Exponentiation (2^x)
 * 
 * Approximates 2^x using exponent manipulation.
 */
float fast_pow2(float x) {
    int32_t i = *(int32_t*)&x;
    i = i + 0xb800000;
    i = *(int32_t*)&(*(float*)&i);
    i = i + 0x3f800000;
    return *(float*)&i;
}

/**
 * FAST EXPONENTIAL (e^x)
 * Mathematical name: Natural Exponential Approximation
 * 
 * Approximates e^x using exponent manipulation.
 * Scaled by 0xb8aa3b (≈ 2^23 / ln(2))
 */
float fast_powe(float x) {
    int32_t i = (int32_t)(0xb8aa3b * x);
    i = i + 0x3f800000;
    return *(float*)&i;
}

/**
 * FAST POWER OF TEN
 * Mathematical name: Decimal Exponentiation (10^x)
 * 
 * Approximates 10^x using exponent manipulation.
 * Scaled by 0x1a934f0 (≈ 2^23 * log2(10))
 */
float fast_pow10(float x) {
    int32_t i = (int32_t)(0x1a934f0 * x);
    i = i + 0x3f800000;
    return *(float*)&i;
}
#endif // __FAST_MATH_H

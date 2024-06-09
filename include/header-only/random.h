#pragma once

#include <stdint.h>
#include <random.h>

#define RANDINT(n) (xorshift() % (n))
#define RANDOM() ((float)(xorshift() % 16777217) / 16777217)

static inline uint32_t xorshift(void);
static inline uint32_t xorshift(void)
{
    static uint32_t x = 123456789;
    static uint32_t y = 362436069;
    static uint32_t z = 521288629;
    static uint32_t w = 88675123;

    uint32_t t;

    t = x ^ (x << 11);
    x = y; y = z; z = w;
    w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));

    return w;
}



/*
    Returns a shuffled array of the first n integer.
*/
static inline void shuffled_indices(uint32_t* array, uint32_t n);
static inline void shuffled_indices(uint32_t* array, uint32_t n)
{
    for(uint32_t i = n - 1; i > 1; i--) {
        uint32_t j = RANDINT(i + 1);
        uint32_t t = array[i];
        array[i] = array[j];
        array[j] = t;
    }
}



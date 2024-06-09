#pragma once

#include <stdint.h>

/*
 * Takes order into account.
*/
static inline int check_array_equality(uint32_t* arr1, uint32_t size1, uint32_t* arr2, uint32_t size2);
static inline int check_array_equality(uint32_t* arr1, uint32_t size1, uint32_t* arr2, uint32_t size2)
{
    return (size1 == size2 && memcmp(arr1, arr2, size1 * sizeof(uint32_t)) == 0);
}


/*
 * Initialize array with numbers from offset to offset + size.
*/
static inline void initialize_order(uint32_t* arr, uint32_t size, uint32_t offset);
static inline void initialize_order(uint32_t* arr, uint32_t size, uint32_t offset)
{
    for(uint32_t i = 0, v = offset; i < size; i++, v++) arr[i] = v;
}

/*
 * Computes the cardinality of the intersection of two sorted arrays.
*/
static inline int len_intersection(uint32_t* arr1, uint32_t size1, uint32_t* arr2, uint32_t size2);
static inline int len_intersection(uint32_t* arr1, uint32_t size1, uint32_t* arr2, uint32_t size2)
{
    uint32_t count = 0;
    while(size1 > 0 && size2 > 0) {
        if(arr1[size1 - 1] > arr2[size2 - 1]) size1--;
        else if(arr1[size1 - 1] < arr2[size2 - 1]) size2--;
        else {
            count++;
            size1--;
            size2--;
        }
    }
    return count;
}


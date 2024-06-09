#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>


// Structure to track memory allocation information
typedef struct {
    void* ptr;
    uint64_t size;
    const char* file;
    int line;
} MemoryInfo;


// Function to track malloc calls
static void* tracked_malloc(uint64_t size, const char* file, int line);

// Function to track calloc calls
static void* tracked_calloc(uint64_t num, uint64_t size, const char* file, int line);

// Function to track free calls
static void tracked_free(void* ptr, const char* file, int line);



// Function to track malloc calls
static void* tracked_malloc(uint64_t size, const char* file, int line) {
    void* ptr = malloc(size);
    if (ptr != NULL) {
        MemoryInfo info = {ptr, size, file, line};
        printf("Allocated memory: %p (%zu bytes) at %s:%d\n", ptr, size, file, line);
        // Optionally, you can store the MemoryInfo for later analysis
    }
    return ptr;
}

// Function to track calloc calls
static void* tracked_calloc(uint64_t num, uint64_t size, const char* file, int line) {
    void* ptr = calloc(num, size);
    if (ptr != NULL) {
        MemoryInfo info = {ptr, num * size, file, line};
        printf("Allocated memory: %p (%zu bytes) at %s:%d\n", ptr, num * size, file, line);
        // Optionally, you can store the MemoryInfo for later analysis
    }
    return ptr;
}

// Function to track free calls
static void tracked_free(void* ptr, const char* file, int line) {
    if (ptr != NULL) {
        printf("Freeing memory: %p at %s:%d\n", ptr, file, line);
        free(ptr);
    }
}







// Wrapper function for malloc
#define malloc(size) tracked_malloc(size, __FILE__, __LINE__)

// Wrapper function for calloc
#define calloc(num, size) tracked_calloc(num, size, __FILE__, __LINE__)

// Wrapper function for free
#define free(ptr) tracked_free(ptr, __FILE__, __LINE__)


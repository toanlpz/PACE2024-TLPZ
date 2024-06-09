#include "segtree.h"

// #include "mem_track.h"

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>



static inline uint64_t height(const uint64_t x) {
    uint64_t y;
    asm ( "\tbsr %1, %0\n"
        : "=r"(y)
        : "r" (x - 1)
    );
    return y + 2;
}

static inline uint64_t mean(const uint64_t a, const uint64_t b) {
    return ((a & b) + ((a ^ b) >> 1));
}

static inline uint64_t min(const uint64_t a, const uint64_t b) {
    return a < b ? a : b;
}

static inline uint64_t max(const uint64_t a, const uint64_t b) {
    return a > b ? a : b;
}


segtree* build_empty_segtree(uint64_t n)
{
    segtree* segt = (segtree*)malloc(sizeof(segtree));
    segt->tree = (uint64_t*)calloc(4 * n, sizeof(uint64_t));

    if(NULL == segt || NULL == segt->tree) {
        fprintf(stderr, "Failed to allocate memory for the segtree\n");
        free(segt->tree);
        free(segt);
        return NULL;
    }

    segt->size = n;
    segt->height = height(n);
    segt->size_before_base_array = ((unsigned long long int)1 << (segt->height - 1)) - 1;

    segt->get_sum_stack = (stack_args*)malloc(4 * segt->height * sizeof(stack_args));
    if(NULL == segt->get_sum_stack) {
        fprintf(stderr, "Failed to allocate memory for the segtree\n");
        free(segt->tree);
        free(segt);
        return NULL;
    }

    return segt;
}

void free_segtree(segtree* segt)
{
    free(segt->get_sum_stack);
    free(segt->tree);
    free(segt);
}

int update_segtree(segtree* segt, uint64_t index, uint64_t new_value)
{
    if(index >= segt->size) {
        fprintf(stderr, "Index (%" PRIu64 ") out of range (%" PRIu64 ")\n", index, segt->size);
        return 1;
    }

    uint64_t v = segt->size_before_base_array + index;

    int64_t delta = new_value - segt->tree[v];
    segt->tree[v] = new_value;

    for(uint64_t level = segt->height - 1; level > 0; level--) {
        v = (v + 1) / 2 - 1;
        segt->tree[v] += delta;
    }

    return 0;
}

uint64_t get_sum(segtree* segt, uint64_t ql, uint64_t qr)
{
    stack_args* stack = segt->get_sum_stack;

    stack[0] = (stack_args){1, 0, segt->size_before_base_array - 1, ql, qr};

    uint64_t stack_size = 1;

    uint64_t total = 0;

    while(stack_size) {
        stack_size--;
        stack_args c = stack[stack_size];   // current iteration parameters
        
        if(c.l > c.r) continue;

        if(c.l == c.tl && c.r == c.tr) {
            total += segt->tree[c.v - 1];
            continue;
        }

        uint64_t tm = mean(c.tl, c.tr);

        stack[stack_size+1] = (stack_args){c.v * 2,     c.tl,   tm,     c.l,            min(c.r,tm)};
        stack[stack_size]  =  (stack_args){c.v * 2+1,   tm+1,   c.tr,   max(c.l,tm+1),  c.r};
        stack_size += 2;

    }

    return total;
}
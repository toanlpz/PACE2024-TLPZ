#pragma once

#include <stdint.h>
#include <stdlib.h>

typedef struct {uint64_t v, tl, tr, l, r;} stack_args;

typedef struct{
    uint64_t size;
    uint64_t* tree;

    uint64_t height;  
    uint64_t size_before_base_array;  

    stack_args* get_sum_stack;
} segtree;


segtree* build_empty_segtree(uint64_t n);

void free_segtree(segtree* segt);

int update_segtree(segtree* segt, uint64_t index, uint64_t new_value);

uint64_t get_sum(segtree* segt, uint64_t ql, uint64_t qr);



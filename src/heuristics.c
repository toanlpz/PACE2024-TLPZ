#include "heuristics.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function prototypes
static void merge_sort(void *base, uint64_t num, uint64_t size, int (*compar)(const void *, const void *));
static void merge_sort_recursive(void *base, uint64_t size, int (*compar)(const void *, const void *), void *temp, uint64_t left, uint64_t right);
static void merge(void *base, uint64_t size, int (*compar)(const void *, const void *), void *temp, uint64_t left, uint64_t mid, uint64_t right);

static void merge_sort(void *base, uint64_t num, uint64_t size, int (*compar)(const void *, const void *)) {
    if (num < 2) return; // An array with fewer than 2 elements is already sorted
    
    // Allocate memory for temporary array
    void *temp = malloc(num * size);
    if (temp == NULL) {
        // Handle memory allocation failure
        return;
    }
    
    merge_sort_recursive(base, size, compar, temp, 0, num - 1);
    
    free(temp); // Free the temporary array
}

static void merge_sort_recursive(void *base, uint64_t size, int (*compar)(const void *, const void *), void *temp, uint64_t left, uint64_t right) {
    if (left >= right) return; // Base case: a single element is already sorted
    
    uint64_t mid = left + (right - left) / 2;
    merge_sort_recursive(base, size, compar, temp, left, mid);
    merge_sort_recursive(base, size, compar, temp, mid + 1, right);
    merge(base, size, compar, temp, left, mid, right);
}

static void merge(void *base, uint64_t size, int (*compar)(const void *, const void *), void *temp, uint64_t left, uint64_t mid, uint64_t right) {
    uint64_t left_start = left;
    uint64_t left_end = mid;
    uint64_t right_start = mid + 1;
    uint64_t right_end = right;
    uint64_t index = left;
    
    // Copy both halves into a temporary array
    memcpy((char *)temp + left * size, (char *)base + left * size, (right - left + 1) * size);
    
    // Merge the two halves back into the original array
    while (left_start <= left_end && right_start <= right_end) {
        if (compar((char *)temp + left_start * size, (char *)temp + right_start * size) <= 0) {
            memcpy((char *)base + index * size, (char *)temp + left_start * size, size);
            left_start++;
        } else {
            memcpy((char *)base + index * size, (char *)temp + right_start * size, size);
            right_start++;
        }
        index++;
    }
    
    // Copy any remaining elements from the left half
    while (left_start <= left_end) {
        memcpy((char *)base + index * size, (char *)temp + left_start * size, size);
        left_start++;
        index++;
    }
    
    // Copy any remaining elements from the right half
    while (right_start <= right_end) {
        memcpy((char *)base + index * size, (char *)temp + right_start * size, size);
        right_start++;
        index++;
    }
}


typedef struct { uint32_t key, value; } pair_lu_lu;
typedef struct { float key; uint32_t value;} pair_f_lu;


static int compare_pair_lu_lu(const void* a, const void* b)
{
    pair_lu_lu *p1 = (pair_lu_lu*)a;
    pair_lu_lu *p2 = (pair_lu_lu*)b;

    if(p1->key > p2->key) return 1;
    if(p1->key < p2->key) return -1;
    return 0;
}

static int compare_pair_f_lu(const void* a, const void* b)
{
    pair_f_lu *p1 = (pair_f_lu*)a;
    pair_f_lu *p2 = (pair_f_lu*)b;

    if(p1->key > p2->key) return 1;
    if(p1->key < p2->key) return -1;
    return 0;
}


static int sort_by_median(uint32_t *median, uint32_t *vertices, uint64_t nbElem)
{
    pair_lu_lu * pairs = (pair_lu_lu*)malloc(nbElem * sizeof(pair_lu_lu));
    if(pairs == NULL) {
        fprintf(stderr, "Error while allocating ressources for sort_by_median\n");
        return 1;
    }

    for(uint32_t i = 0; i < nbElem; i++) {
        pairs[i].key = median[i];
        pairs[i].value = vertices[i];
    }

    merge_sort(pairs, nbElem, sizeof(pair_lu_lu), compare_pair_lu_lu);

    for(uint32_t i = 0; i < nbElem; i++) {
        median[i] = pairs[i].key;
        vertices[i] = pairs[i].value;
    }
    return 0;
}

static int sort_by_mean(float *mean, uint32_t *vertices, uint64_t nbElem)
{
    pair_f_lu * pairs = (pair_f_lu*)malloc(nbElem * sizeof(pair_f_lu));
    if(pairs == NULL) {
        fprintf(stderr, "Error while allocating ressources for sort_by_median\n");
        return 1;
    }

    for(uint32_t i = 0; i < nbElem; i++) {
        pairs[i].key = mean[i];
        pairs[i].value = vertices[i];
    }

    merge_sort(pairs, nbElem, sizeof(pair_f_lu), compare_pair_f_lu);

    for(uint32_t i = 0; i < nbElem; i++) {
        mean[i] = pairs[i].key;
        vertices[i] = pairs[i].value;
    }
    return 0;
}


int median_heuristic(graph* g, uint32_t* order)
{
    uint32_t *median = (uint32_t*)malloc(g->nB * sizeof(uint32_t));
    if(median == NULL) {
        fprintf(stderr, "Failed memory allocation in median_heuristic.\n");
        return 1;
    }

    for(uint32_t v_id = 0, v = g->nA; v_id < g->nB; v_id++, v++) {
        
        uint32_t degv = g->degree[v];
        if(degv == 0) {
            median[v_id] = 0;
            continue;
        }
        uint32_t median_index = degv / 2;
        uint32_t parity_based_offset = degv % 2;
        median[v_id] = 2 * g->adjacency_arr[v][median_index] - parity_based_offset;
        
    }
    
    sort_by_median(median, order, g->nB);

    free(median);
    return 0;
}

int mean_heuristic(graph* g, uint32_t* order)
{
    float *mean = (float*)calloc(g->nB, sizeof(float));
    if(mean == NULL) {
        fprintf(stderr, "Failed memory allocation in mean_heuristic.\n");
        return 1;
    }

    for(uint32_t v_id = 0, v = g->nA; v_id < g->nB; v_id++, v++) {
        for(uint32_t i = 0; i < g->degree[v]; i++) {
            mean[v_id] += g->adjacency_arr[v][i];
        }
        mean[v_id] /= g->degree[v];
    }

    sort_by_mean(mean, order, g->nB);

    free(mean);
    return 0;
}

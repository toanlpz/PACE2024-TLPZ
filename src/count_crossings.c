#include "count_crossings.h"

// #include "mem_track.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "segtree.h"

typedef struct {edge e; uint32_t order;} edge_with_order;

static int compare_ordered_edge(const void* a, const void* b)
{
    edge_with_order *e1 = (edge_with_order*)a;
    edge_with_order *e2 = (edge_with_order*)b;

    if(e1->e.u > e2->e.u) return 1;
    if(e2->e.u > e1->e.u) return -1;

    if(e1->order > e2->order) return 1;
    if(e2->order > e1->order) return -1;

    return 0;
}


uint64_t count_crossings(graph* g, uint32_t* ordering)
{
    bool free_order = false;
    if(g->isReduced) {
        uint32_t* act_order = (uint32_t*)malloc(g->eqgraph->nB * sizeof(uint32_t));
        if(NULL == act_order) { fprintf(stderr, "Failed allocation while counting crossings\n"); return -1; }

        get_actual_order(g->eqgraph, ordering, act_order);
        
        g = g->eqgraph;
        ordering = act_order;
        free_order = true;
    }


    uint32_t nA = g->nA;
    uint32_t nB = g->nB;
    uint32_t m = g->m;

    edge_with_order* edges_with_order = (edge_with_order*)malloc(m * sizeof(edge_with_order));
    uint32_t* right_order = (uint32_t*)malloc(nB * sizeof(uint32_t));
    if(NULL == edges_with_order || NULL == right_order) {
        fprintf(stderr, "Failed allocation while counting crossings\n");
        return -1;
    }

    for(uint32_t i = 0; i < nB; i++) {
        right_order[ordering[i] - nA] = i;
    }

    for(uint64_t i = 0; i < m; i++) {
        edges_with_order[i].e = g->edges[i];
        edges_with_order[i].order = right_order[g->edges[i].v - nA];
    }

    qsort(edges_with_order, m, sizeof(edge_with_order), compare_ordered_edge);
    segtree* t = build_empty_segtree(nB + 1);
    uint64_t crossings = 0;
    for(uint32_t i = 0; i < m; i++) {
        edge e = edges_with_order[i].e;

        uint64_t position = right_order[e.v - nA];
        uint64_t old = get_sum(t, position, position);
        update_segtree(t, position, old + 1);

        if(nB > position + 1) {
            crossings += get_sum(t, position+1, nB);
        }
    }


    free(edges_with_order);
    free(right_order);

    if(free_order) free(ordering);

    free_segtree(t);

    return crossings;
}
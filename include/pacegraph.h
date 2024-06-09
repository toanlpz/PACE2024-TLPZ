#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct { uint32_t u, v; } edge;
typedef struct graph {
    uint32_t nA, nB, n, m;
    bool hasReduced, isReduced;
    void* root_ptr;
    struct graph* eqgraph;      // equivalent graph
    
    edge* edges;                    // store all edges
                                    // size = m
    
    uint32_t* degree;               // count the number of neighbors of each vertex
                                    // size = n
    uint32_t** adjacency_arr;       // adjacency_arr[v] points to the first neighbor of the vertex v
                                    // size = n
    uint32_t* all_adj;              // all adjacency data
                                    // size = 2*m
    uint32_t* weights;              // weights[eq] is the number of vertices in the equivalence class eq
                                    // size = nB

    uint32_t* eq_of;                // eq_of[v] is the id of the equivalence class of v
                                    // size = nB
    uint32_t* all_eq_class;         // equivalence classes
                                    // size = nB
    uint32_t** eq_class;            // eq_class[eq] points to the first vertex of the equivalence class eq
                                    // size eq_count = eqgraph->nB <= nB
    
} graph;

int read_input_from_stdin(graph* g);
int read_input_from_file(char* input, graph* g);


void output_to_stdout(uint32_t* output, uint32_t size);
int output_to_file(char *output_file_name, uint32_t* output, uint32_t size);

int graph_malloc(graph* g, int is_initial);
void free_graph(graph* g);

void build_graph(graph* g, bool is_initial);

bool build_reduced_graph(graph* initial_g, graph* eqg, float threshold);
void get_actual_order(graph* initial_g, uint32_t* eq_order, uint32_t* order);

/*
 * Cost to go from uv to vu
*/
int64_t cost_uv_to_vu(graph* g, uint32_t u, uint32_t v, bool* already_zero);
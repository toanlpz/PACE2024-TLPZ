#include "pacegraph.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include "arrays.h"
#include "random.h"



int read_input_from_stdin(graph* g)
{
    char line[30];

    do {
        if(fgets(line, sizeof(line), stdin) == NULL) {
            fprintf(stderr, "Error reading input\n");
            return 1;
        }
    } while('c' == line[0]);

    if('p' != line[0]) {
        fprintf(stderr, "Error : Expected p line.\n");
        return 1;    
    }

    if(sscanf(line, "%*c %*s %" PRIu32 " %" PRIu32 " %" PRIu32 "", &g->nA, &g->nB, &g->m) != 3) {
        fprintf(stderr, "Wrong p line format.\n");
        return 1;
    }
    g->n = g->nA + g->nB;

    if(graph_malloc(g, true) != 0) {
        fprintf(stderr, "Error while allocating ressources for the initial graph.\n");
        return 1;
    }

    uint32_t index = 0;
    while(index < g->m) {
        do {
            if(fgets(line, sizeof(line), stdin) == NULL) {
                fprintf(stderr, "Error reading input\n");
                return 1;
            }
        } while('c' == line[0]);
        
        uint32_t u, v;
        if(2 != sscanf(line, "%" PRIu32 " %" PRIu32 "", &u, &v)) {
            fprintf(stderr, "Wrong format.\n");
            return 1;
        }

        g->edges[index++] = (edge) {u - 1, v - 1};
    }

    return 0;    

}



void output_to_stdout(uint32_t* output, uint32_t size)
{
    for(uint32_t i = 0; i < size; i++) {
        printf("%" PRIu32 "\n", output[i] + 1);
    }
}


#ifdef WIP
int read_input_from_file(char* input, graph* g)
{   
    FILE* file;
    file = fopen(input, "r");
    if(file == NULL) {
        fprintf(stderr, "Failed opening file at %s.\n", input);
        return 1;
    }

    char line[30];

    do {
        if(fgets(line, sizeof(line), file) == NULL) {
            fprintf(stderr, "Error reading input\n");
            return 1;
        }
    } while('c' == line[0]);

    if('p' != line[0] || sscanf(line, "%*c %*s %" PRIu32 " %" PRIu32 " %" PRIu32 "", &g->nA, &g->nB, &g->m) != 3) {
        fprintf(stderr, "Wrong p line format.\n");
        return 1;
    }

    g->n = g->nA + g->nB;


    if(graph_malloc(g, true) != 0) {
        fprintf(stderr, "Error while allocating ressources for the initial graph.\n");
        return 1;
    }

    uint32_t index = 0;
    while(fgets(line, sizeof(line), file) != NULL) {
        if(line[0] == 'c') continue;
        uint32_t u, v;
        if(2 != sscanf(line, "%" PRIu32 " %" PRIu32 "", &u, &v)) {
            fprintf(stderr, "Wrong format on edge line %" PRIu32 ".\n", index);
            return 1;
        }

        g->edges[index] = (edge) {u - 1, v - 1};
        index++;
    }

    return 0;
}
#endif

#ifdef WIP
int output_to_file(char *output_file_name, uint32_t* output, uint32_t size)
{
    FILE *file = fopen(output_file_name, "w");
    if(NULL == file)
    {
        fprintf(stderr, "Error opening the output file.\n");
        return 1;
    }

    for(uint32_t i = 0; i < size; i++)
    {
        fprintf(file, "%" PRIu32 "\n", output[i] + 1);
    }
    fflush(file);

    fclose(file);

    return 0;
}
#endif


void free_graph(graph* g)
{
    free(g->root_ptr);
}

/*
allocates memory to the graph based on its n, nB and m values.
*/
int graph_malloc(graph* g, int is_initial)
{
    free_graph(g);

    uint32_t total_size =
        g->m * sizeof(edge) +           // g.edges
        g->n * sizeof(uint32_t) +          // g.degree
        g->n * sizeof(uint32_t*) +         // g.adjacency_arr
        g->m * 2 * sizeof(uint32_t) +      // g.all_adj
        g->nB * sizeof(uint32_t);          // g.weights
    
    if(is_initial)
    total_size +=
        g->nB * sizeof(uint32_t) +         // g.eq_of
        g->nB * sizeof(uint32_t) +         // g.all_eq_class
        g->nB * sizeof(uint32_t*);         // g.eq_class

    char* raw_block = (char*)malloc(total_size);
    if(raw_block == NULL) {
        fprintf(stderr, "Allocation error.\n");
        return 1;
    }
    memset(raw_block, 0, total_size);

    g->root_ptr = (void*)raw_block;


    g->edges = (edge*)raw_block;
    raw_block += g->m * sizeof(edge);        

    g->degree = (uint32_t*)raw_block;
    raw_block += g->n * sizeof(uint32_t);

    g->adjacency_arr = (uint32_t**)raw_block;
    raw_block += g->n * sizeof(uint32_t*);
        
    g->all_adj = (uint32_t*)raw_block;
    raw_block += 2 * g->m * sizeof(uint32_t);

    g->weights = (uint32_t*)raw_block;
    raw_block += g->nB * sizeof(uint32_t); 

    if(is_initial) {
        g->eq_of = (uint32_t*)raw_block;
        raw_block += g->nB * sizeof(uint32_t);

        g->all_eq_class = (uint32_t*)raw_block;
        raw_block += g->nB * sizeof(uint32_t);

        g->eq_class = (uint32_t**)raw_block;
        raw_block += g->nB * sizeof(uint32_t*);
    }

    return 0;
}



static int compare_edge(const void* a, const void* b)
{
    edge e1 = *(edge*)a;
    edge e2 = *(edge*)b;

    if(e1.u > e2.u) return 1;
    if(e1.u < e2.u) return -1;

    if(e1.v > e2.v) return 1;
    if(e1.v < e2.v) return -1;

    return 0;
}

/*
 * builds the remainder of the graph from its edges array. The graph need to already have memory allocated.
*/
void build_graph(graph* g, bool is_initial)
{
    qsort(g->edges, g->m, sizeof(edge), compare_edge);

    uint32_t* deg = g->degree;
    uint32_t** adjarr = g->adjacency_arr;
    uint32_t* alladj = g->all_adj;


    for(uint32_t i = 0; i < g->m; i++) {               // computes the degree of all vertices.
        edge e = g->edges[i];
        deg[e.u]++;
        deg[e.v]++;
    }

    adjarr[0] = alladj;
    for(uint32_t v = 1; v < g->n; v++) {
        adjarr[v] = adjarr[v - 1] + deg[v - 1];     // computes where to put the neighbors of each vertex based on the placement and the size of the previous one
    }
    memset(deg, 0, g->n * sizeof(uint32_t));        // reset degrees to know where to put the adjacent vertices.



    for(uint32_t i = 0; i < g->m; i++) {
        edge e = g->edges[i];
        adjarr[e.u][deg[e.u]] = e.v;
        adjarr[e.v][deg[e.v]] = e.u;
        
        deg[e.u]++;
        deg[e.v]++;
    }
    
    
    if(is_initial)
        for(uint32_t v = 0; v < g->nB; v++)
            g->weights[v] = 1;


}



bool build_reduced_graph(graph* initial_g, graph* eqg, float threshold)
{
    eqg->nA = initial_g->nA;
    eqg->nB = 0;
    eqg->m = 0;

    uint32_t* w = (uint32_t*)calloc(initial_g->nB, sizeof(uint32_t));
    uint32_t* eq_rep = (uint32_t*)calloc(initial_g->nB, sizeof(uint32_t));
    edge* edges = (edge*)calloc(initial_g->m, sizeof(edge));

    if(NULL == w || NULL == eq_rep || NULL == edges) {
        fprintf(stderr, "Failed allocation for temporary arrays while building equivalent graph.\n");
        free(w); free(eq_rep); free(edges);
        return false;
    }


    // Testing if the reduction rate is high enough.
    #define SAMPLE_SIZE 10000
    if(initial_g->nB > SAMPLE_SIZE * 2) {
        
        uint32_t* shuffled = (uint32_t*)malloc(SAMPLE_SIZE * sizeof(uint32_t));
        if(NULL == shuffled) {
            fprintf(stderr, "Failed allocation for temporary arrays while building equivalent graph.\n");
            free(w); free(eq_rep); free(edges);
            return false;
        }

        uint32_t temp_nB = 0;
        initialize_order(shuffled, SAMPLE_SIZE, 0);
        shuffled_indices(shuffled, SAMPLE_SIZE);
        
        for(uint32_t i = 0; i < SAMPLE_SIZE; i++) {
            uint32_t v_ind = shuffled[i];
            uint32_t v = v_ind + initial_g->nA;
            for(uint32_t eq = 0; eq < temp_nB; eq++) {
                uint32_t rep = eq_rep[eq];

                if(check_array_equality(initial_g->adjacency_arr[v], initial_g->degree[v], initial_g->adjacency_arr[rep], initial_g->degree[rep])) {
                    initial_g->eq_of[v_ind] = eq + initial_g->nA;
                    goto continue_outer_loop;
                }
            }
            // if no equivalence found (then new equivalence) :
            initial_g->eq_of[v_ind] = eqg->nA + temp_nB;
            eq_rep[temp_nB] = v;

            temp_nB++;

            continue_outer_loop: continue;
        }

        free(shuffled);
        if(temp_nB / (float)SAMPLE_SIZE > threshold) {
            free(w); free(eq_rep); free(edges);
            return false;
        }
    }
    


    for(uint32_t v_ind = 0, v = initial_g->nA; v_ind < initial_g->nB; v_ind++, v++) {
        for(uint32_t eq = 0; eq < eqg->nB; eq++) {
            uint32_t rep = eq_rep[eq];

            if(check_array_equality(initial_g->adjacency_arr[v], initial_g->degree[v], initial_g->adjacency_arr[rep], initial_g->degree[rep])) {
                initial_g->eq_of[v_ind] = eq + initial_g->nA;
                w[eq]++;
                goto found_eq_continue;
            }
        }
        // if no equivalence found (then new equivalence) :
        initial_g->eq_of[v_ind] = eqg->nA + eqg->nB;
        eq_rep[eqg->nB] = v;
        w[eqg->nB] = 1;

        for(uint32_t i = 0; i < initial_g->degree[v]; i++) {
            uint32_t u = initial_g->adjacency_arr[v][i];
            edges[eqg->m] = (edge){u, eqg->nA + eqg->nB};
            eqg->m++;
        }
        eqg->nB++;

        found_eq_continue:
        continue;
    }

    eqg->n = eqg->nA + eqg->nB;

    if(graph_malloc(eqg, false) != 0) {
        fprintf(stderr, "Error while allocating ressources for the equivalent graph.\n");
        free(w); free(eq_rep); free(edges);
        return false;
    }

    memcpy(eqg->edges, edges, eqg->m * sizeof(edge));
    memcpy(eqg->weights, w, eqg->nB * sizeof(uint32_t));   // copying temporary weight array into newly allocated eqg's one

    build_graph(eqg, false);

    initial_g->eqgraph = eqg;
    initial_g->hasReduced = true;

    initial_g->eq_class[0] = initial_g->all_eq_class;
    for(uint32_t eq = 1; eq < eqg->nB; eq++) {
        initial_g->eq_class[eq] = initial_g->eq_class[eq - 1] + eqg->weights[eq - 1];
    }
    memset(eqg->weights, 0, eqg->nB * sizeof(uint32_t));

    for(uint32_t v = initial_g->nA, i = 0; v < initial_g->n; v++, i++) {
        uint32_t eq = initial_g->eq_of[i] - initial_g->nA;
        initial_g->eq_class[eq][eqg->weights[eq]] = v;
        eqg->weights[eq]++;
    }

    eqg->eqgraph = initial_g;
    eqg->isReduced = true;
    
    free(w); free(eq_rep); free(edges);

    return true;
}

void get_actual_order(graph* initial_g, uint32_t* eq_order, uint32_t* order)
{
    uint32_t i = 0;
    graph* eqg = initial_g->eqgraph;

    for(uint32_t eq_index = 0; eq_index < eqg->nB; eq_index++) {
        uint32_t eq = eq_order[eq_index];                                   // vertex of eqgraph
        uint32_t* eq_array = initial_g->eq_class[eq - initial_g->nA];       // all vertices eq is representing
        for(uint32_t j = 0; j < eqg->weights[eq - initial_g->nA]; j++) {
            
            order[i++] = eq_array[j];
        }

    }
}





int64_t cost_uv_to_vu(graph* g, uint32_t u, uint32_t v, bool* already_zero)
{
    int64_t c_uv = 0;
    int64_t c_vu = 0;

    uint32_t u_remaining_neighbor = g->degree[u];
    uint32_t v_remaining_neighbor = g->degree[v];

    uint32_t* u_neighbor = g->adjacency_arr[u];
    uint32_t* v_neighbor = g->adjacency_arr[v];

    uint32_t* end_u = &g->adjacency_arr[u][g->degree[u]];
    uint32_t* end_v = &g->adjacency_arr[v][g->degree[v]];

    while(u_neighbor < end_u && v_neighbor < end_v) {
        if(*v_neighbor < *u_neighbor) {
            v_neighbor++;
            v_remaining_neighbor--;
            c_uv += u_remaining_neighbor;
        }
        else {
            c_vu += v_remaining_neighbor - (*u_neighbor == *v_neighbor);
            u_neighbor++;
            u_remaining_neighbor--;            
        }
    }

    *already_zero = (c_uv == 0);

    return (c_vu - c_uv) * g->weights[u - g->nA] * g->weights[v - g->nA];
}









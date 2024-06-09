
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>
#include <inttypes.h>

#include "signal_handling.h"

#include "pacegraph.h"
#include "arrays.h"
#include "heuristics.h"
#include "simulated_annealing.h"
#include "count_crossings.h"

#define MIN(a,b) (((a) < (b)) ? (a) : (b))


int main(int argc, char *argv[])
{
    enable_signal();

    graph g_init = {0};
    graph eqg = {0};

    graph* g;


    #ifdef WIP                                                          //
        if(read_input_from_file(argv[1], &g_init) != 0) {                //
            free_graph(&g_init);                                         //
            fprintf(stderr, "Problem while reading input file.\n");      //
            return 1;                                                    //
        }                                                                //
    #else                                                                /// Read input
        if(read_input_from_stdin(&g_init) != 0) {                        //
            free_graph(&g_init);                                         //
            fprintf(stderr, "Problem while reading input.\n");           //
            return 1;                                                    //
        }                                                                //
    #endif                                                              //

    

    // Build graph and equivalent graph
    build_graph(&g_init, true);
    g = build_reduced_graph(&g_init, &eqg, 0.75) ? &eqg : &g_init;

    uint32_t* order = (uint32_t*)malloc(g->nB * sizeof(uint32_t));
    if(order == NULL) { free_graph(&g_init); fprintf(stderr, "Failed mem allocation for the order array.\n"); return 1; }


    // Perform calculations
    uint64_t cost;
    if(g->nB > 1) {
        initialize_order(order, g->nB, g->nA);                          //
        median_heuristic(g, order);                                      // Median Heuristic
        uint64_t median_crossings_count = count_crossings(g, order);    //

        initialize_order(order, g->nB, g->nA);                          //
        mean_heuristic(g, order);                                        // Mean Heuristic
        uint64_t mean_crossings_count = count_crossings(g, order);      //

        uint64_t starting_cost;                                         //
        initialize_order(order, g->nB, g->nA);                           //
        if(median_crossings_count > mean_crossings_count) {              //
            mean_heuristic(g, order);                                    //
            starting_cost = mean_crossings_count;                        //
        }                                                                /// Compare the two heuristics and select the best one
        else {                                                           //
            median_heuristic(g, order);                                  //
            starting_cost = median_crossings_count;                      //
        }                                                               //
     
        cost = homogeneous_SA(g, order, MIN(median_crossings_count, mean_crossings_count));

        #ifdef WIP
        printf("%llu\n%llu\n", MIN(median_crossings_count, mean_crossings_count), cost);
        #endif
    }
    #ifdef WIP
    else printf("0\n0");
    #endif


    // Expand the reduced permutation back to the original one.
    if(g_init.hasReduced) {
        uint32_t* order_on_initial = (uint32_t*)malloc(g_init.nB * sizeof(uint32_t));
        if(order_on_initial == NULL) {
            fprintf(stderr, "Failed mem allocation for the actual order array.\n");
            free(order); free_graph(&g_init); free_graph(&eqg);
            return 1;
        }

        if(g->nB > 1) 
            get_actual_order(&g_init, order, order_on_initial);
        else
            initialize_order(order_on_initial, g_init.nB, g_init.nA);

        uint32_t* t = order;
        order = order_on_initial;
        free(t);
    }



    #ifdef WIP
    output_to_file(argv[2], order, g_init.nB);
    #else
    output_to_stdout(order, g_init.nB);
    #endif


    fflush(stdout);
    fflush(stderr);

    free(order);
    free_graph(&g_init);
    free_graph(&eqg);
    return 0;
}
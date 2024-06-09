#include "simulated_annealing.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "signal_handling.h"
#include "random.h"
#include "exp.h"
#include "arrays.h"
#include "inverse_normal_cdf.h"
#include "count_crossings.h"


static inline double max_lf(const double a, const double b)
{
    return a > b ? a : b;
}

static inline double min_lf(const double a, const double b)
{
    return a < b ? a : b;
}

static inline uint32_t max_uint32(const uint32_t a, const uint32_t b) {
    return a > b ? a : b;
}


static inline bool attempt_transition(graph* g, uint32_t* order, float t, uint64_t* current_cost)
{
    uint32_t u_ind = RANDINT(g->nB - 1);
    uint32_t u = order[u_ind], v = order[u_ind + 1];

    bool already_zero;
    int64_t cost = cost_uv_to_vu(g, u, v, &already_zero);

    if(cost <= 0 || (!already_zero && RANDOM() < fastExp4(-cost / t))) {
        order[u_ind] = v;
        order[u_ind + 1] = u;

        *current_cost += cost;
        return true;
    }

    return false;
}

static int compute_mean_and_var_infinity(graph* g, double* mean, double* var)
{
    uint32_t sample_size = g->m < 10000 ? 50 : 16;
    uint64_t* scores = (uint64_t*)calloc(sample_size, sizeof(uint64_t));
    uint32_t* order = (uint32_t*)malloc(g->nB * sizeof(uint32_t));
    if(NULL == scores || NULL == order) {
        fprintf(stderr, "Failed to allocate memory in starting_temperature()\n");
        free(scores); free(order);
        return 1;
    }

    initialize_order(order, g->nB, g->nA);
    for(int i = 0; i < sample_size; i++) {
        shuffled_indices(order, g->nB);
        scores[i] = count_crossings(g, order);
        *mean += scores[i] / (double)sample_size;
    } 

    for(int i = 0; i < sample_size; i++) {
        double x = (double)scores[i] - *mean;
        *var += x * x / (double)sample_size;
    }

    free(scores);
    free(order);

    return 0;
}

/*
 * See Chapter 3 from "On the Acceleration of Simulated Annealing" by James M. Varanelli
*/
static double starting_temperature(graph* g, double mean, double var, uint64_t cost, int chain_length)
{    
    double gamma = inverse_normal_cdf(1 - 1 / ((double)2 * chain_length));
    return var / (mean - (double)cost - gamma * sqrt(var));
}






uint64_t homogeneous_SA(graph* g, uint32_t* best_order, uint64_t initial_cost)
{
    uint32_t* current_order = (uint32_t*)malloc(g->nB * sizeof(uint32_t));
    if(current_order == NULL) {
        fprintf(stderr, "Failed allocation in homogeneous_SA for current order\n");
        return 0;
    }
    uint32_t* best_local_search_order = (uint32_t*)malloc(g->nB * sizeof(uint32_t));
    if(current_order == NULL) {
        fprintf(stderr, "Failed allocation in homogeneous_SA for current order\n");
        return 0;
    }
    memcpy(best_local_search_order, best_order, g->nB * sizeof(uint32_t));
    memcpy(current_order, best_order, g->nB * sizeof(uint32_t));

    double mean, var;
    compute_mean_and_var_infinity(g, &mean, &var);

    const uint32_t chain_length = max_uint32(4, g->nB);
    double temperature;

    uint64_t current_cost, best_cost, last_chain_cost, best_local_search_cost;
    current_cost = best_cost = last_chain_cost = best_local_search_cost = (initial_cost);

    bool do_damp = false;
    uint64_t damping_mechanism = 1;

    while(running) {
        
        // -- local search -- //
        memcpy(current_order, best_local_search_order, g->nB * sizeof(uint32_t));
        current_cost = local_search(g, current_order, best_local_search_cost);
        if(current_cost < best_local_search_cost) {
            best_local_search_cost = current_cost;
            memcpy(best_local_search_order, current_order, g->nB * sizeof(uint32_t));
        }    
        // -- local search - end -- //

        memcpy(current_order, best_order, g->nB * sizeof(uint32_t));                 ///
        current_cost = best_cost;                                                     /// Setting up the next SA
        temperature = starting_temperature(g, mean, var, best_cost, chain_length);   ///

        temperature /= damping_mechanism;

        // ========= classical SA - START ========= //

        uint32_t chain_since_change = 0;                //
        #define RESET_CRITERION 100                     /// Reset mechanism

        while(running && chain_since_change < RESET_CRITERION) {
            
            // --- MARKOV CHAIN - START --- //
            uint32_t count;
            double mean_mc = 0, var_mc = 0; // Mean value and variance within this markov chain
            for(count = 1; count <= chain_length; count++) {
                bool did_transition = attempt_transition(g, current_order, temperature, &current_cost);

                if(did_transition && current_cost < best_cost){                         //
                    memcpy(best_order, current_order, g->nB * sizeof(uint32_t));         // Best Solution Found
                    best_cost = current_cost;                                           //
                }

                double delta = current_cost - mean_mc;          //
                mean_mc += delta / count;                        // Online Welford's Algorithm
                var_mc += delta * (current_cost - mean_mc);     //                                                              
            }
            var_mc /= count - 1;
            // --- MARKOV CHAIN - END --- //
            

            // --- COOLING SCHEDULE - START --- //
            const double delta = 0.085;
            double decrease_rate = 1 / (1 + temperature * log(1 + delta) / (3 * sqrt(var))); // decrement rule of Aarts and van Laarhoven
            
            temperature *= min_lf(0.999, decrease_rate);
            // --- COOLING SCHEDULE - END --- //

            if(current_cost == last_chain_cost)     ///
                chain_since_change++;                //
            else                                     /// Reset mechanism
                chain_since_change = 0;              //
            last_chain_cost = current_cost;         ///

            
        }
        // ========= classical SA - END ========= //

        if(current_cost > 2 * best_cost || do_damp) {
            damping_mechanism *= 2;
            if(damping_mechanism == 0)    // overflow
                damping_mechanism = UINT64_MAX;
            do_damp = true;
        }
        else if(current_cost < best_cost) {
            damping_mechanism = 1;
            do_damp = false;
        }

    }


    // -- local search -- //
    memcpy(current_order, best_local_search_order, g->nB * sizeof(uint32_t));
    current_cost = local_search(g, current_order, best_local_search_cost);
    if(current_cost < best_cost) {
        best_cost = current_cost;
        memcpy(best_order, current_order, g->nB * sizeof(uint32_t));
    }
    // -- local search - end -- //

    free(current_order);
    free(best_local_search_order);

    return best_cost;
}


uint64_t local_search(graph* g, uint32_t* order, uint64_t cost)
{
    const uint32_t stop_criterion = 2 * g->nB;
    uint32_t count = 0;

    uint64_t gain = 0;

    while(count < stop_criterion) {

        uint32_t u_ind = RANDINT(g->nB - 1);
        uint32_t u = order[u_ind], v = order[u_ind + 1];

        bool already_zero;

        int64_t cost = cost_uv_to_vu(g, u, v, &already_zero);

        if(cost <= 0 && !already_zero) {
            order[u_ind] = v;
            order[u_ind + 1] = u;
            gain -= cost;

            count++;
        }
        else {
            count-=5;
        }
    }

    return cost - gain;
}

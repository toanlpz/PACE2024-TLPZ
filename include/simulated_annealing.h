#pragma once

#include <stdint.h>
#include "pacegraph.h"

uint64_t homogeneous_SA(graph* g, uint32_t* best_order, uint64_t initial_cost);

uint64_t local_search(graph* g, uint32_t* order, uint64_t cost);
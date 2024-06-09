#pragma once

#include <stdint.h>
#include "pacegraph.h"

int median_heuristic(graph* g, uint32_t* order);
int mean_heuristic(graph* g, uint32_t* order);

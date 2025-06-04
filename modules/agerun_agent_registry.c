/**
 * @file agerun_agent_registry.c
 * @brief Implementation of the agent registry module
 */

#include "agerun_agent_registry.h"
#include "agerun_agent.h"
#include "agerun_data.h"
#include "agerun_heap.h"
#include <stdio.h>
#include <stdlib.h>

/* Module state */
static bool g_initialized = false;

/* Initialize the agent registry */
bool ar_agent_registry_initialize(void) {
    if (g_initialized) {
        return true;
    }
    
    g_initialized = true;
    return true;
}

/* Shutdown the agent registry */
void ar_agent_registry_shutdown(void) {
    g_initialized = false;
}

/* Check if the registry is initialized */
bool ar_agent_registry_is_initialized(void) {
    return g_initialized;
}

/* Get the number of active agents */
int ar_agent_registry_count(void) {
    if (!g_initialized) {
        return 0;
    }
    return ar_agent_count_active();
}

/* Get the first active agent ID */
int64_t ar_agent_registry_get_first(void) {
    if (!g_initialized) {
        return 0;
    }
    return ar_agent_get_first_active();
}

/* Get the next active agent ID */
int64_t ar_agent_registry_get_next(int64_t current_id) {
    if (!g_initialized) {
        return 0;
    }
    return ar_agent_get_next_active(current_id);
}


/* Reset all agents in the registry */
void ar_agent_registry_reset_all(void) {
    if (!g_initialized) {
        return;
    }
    ar_agent_reset_all();
}

/* Agerun Agency Implementation */
#include "agerun_agency.h"
#include "agerun_agent.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global State */
static agent_t agents[MAX_AGENTS];
static agent_id_t next_agent_id = 1;
static bool is_initialized = false;

/* Implementation */
void ar_agency_set_initialized(bool initialized) {
    is_initialized = initialized;
}

agent_t* ar_agency_get_agents(void) {
    return is_initialized ? agents : NULL;
}

agent_id_t ar_agency_get_next_id(void) {
    return next_agent_id;
}

void ar_agency_set_next_id(agent_id_t id) {
    next_agent_id = id;
}

void ar_agency_reset(void) {
    // Reset all agents to inactive
    for (int i = 0; i < MAX_AGENTS; i++) {
        agents[i].is_active = false;
    }
    
    // Reset next_agent_id
    next_agent_id = 1;
}

/* End of implementation */


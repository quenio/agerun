#ifndef AGERUN_AGENCY_H
#define AGERUN_AGENCY_H

#include <stdbool.h>
#include <stdint.h>
#include "agerun_agent.h"

/**
 * Set initialization state - called by system init
 * @param initialized Initialization state to set
 */
void ar_agency_set_initialized(bool initialized);

/**
 * Get agents array - used by system functions
 * @return Pointer to the agents array
 */
agent_t* ar_agency_get_agents(void);

/**
 * Get next agent ID - used by system functions
 * @return Next agent ID to be assigned
 */
agent_id_t ar_agency_get_next_id(void);

/**
 * Set next agent ID - used by system functions
 * @param id The ID to set as next agent ID
 */
void ar_agency_set_next_id(agent_id_t id);

/**
 * Reset agency state (used during shutdown)
 */
void ar_agency_reset(void);

#endif /* AGERUN_AGENCY_H */


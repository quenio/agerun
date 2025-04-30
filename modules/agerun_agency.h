#ifndef AGERUN_AGENCY_H
#define AGERUN_AGENCY_H

#include <stdbool.h>
#include <stdint.h>
#include "agerun_agent.h"

/* Constants */
#define AGENCY_FILE_NAME "agency.agerun"

/**
 * Set initialization state - called by system init
 * @param initialized Initialization state to set (value type, not a reference)
 */
void ar_agency_set_initialized(bool initialized);

/**
 * Get agents array - used by system functions
 * @return Pointer to the agents array (borrowed reference)
 * @note Ownership: Returns a borrowed reference that caller must not destroy.
 */
agent_t* ar_agency_get_agents(void);

/**
 * Get next agent ID - used by system functions
 * @return Next agent ID to be assigned (value type, not a reference)
 */
agent_id_t ar_agency_get_next_id(void);

/**
 * Set next agent ID - used by system functions
 * @param id The ID to set as next agent ID (value type, not a reference)
 */
void ar_agency_set_next_id(agent_id_t id);

/**
 * Reset agency state (used during shutdown)
 * @note Ownership: Destroys all agent resources and resets global state.
 */
void ar_agency_reset(void);

/**
 * Get the current number of active agents
 * @return Number of active agents (value type, not a reference)
 */
int ar_agency_count_agents(void);

/**
 * Save all persistent agents to disk
 * @return true if successful, false otherwise
 * @note Ownership: Does not affect ownership of any agent resources.
 */
bool ar_agency_save_agents(void);

/**
 * Load all persistent agents from disk
 * @return true if successful, false otherwise
 * @note Ownership: Creates new agents with their own resources.
 */
bool ar_agency_load_agents(void);

#endif /* AGERUN_AGENCY_H */


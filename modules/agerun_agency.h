#ifndef AGERUN_AGENCY_H
#define AGERUN_AGENCY_H

#include <stdbool.h>
#include <stdint.h>
#include "agerun_agent.h"

/* Forward declaration for method_t */
typedef struct method_s method_t;

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

/**
 * Update agents using a specific method to use a different method
 * @param ref_old_method The old method being used (borrowed reference)
 * @param ref_new_method The new method to use (borrowed reference)
 * @return Number of agents updated
 * @note Ownership: Does not take ownership of either method reference.
 *       The update process involves:
 *       1. Agent finishes processing current message
 *       2. Sleep message is sent to agent
 *       3. Method reference is updated
 *       4. Wake message is sent to agent
 */
int ar_agency_update_agent_methods(const method_t *ref_old_method, const method_t *ref_new_method);

/**
 * Count the number of agents using a specific method
 * @param ref_method The method to check (borrowed reference)
 * @return Number of active agents using the method
 * @note Ownership: Does not take ownership of the method reference.
 */
int ar_agency_count_agents_using_method(const method_t *ref_method);

#endif /* AGERUN_AGENCY_H */


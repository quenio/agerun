#ifndef AGERUN_AGENCY_H
#define AGERUN_AGENCY_H

#include <stdbool.h>
#include <stdint.h>
#include "agerun_data.h"

/* Forward declarations */
typedef struct method_s method_t;
typedef struct agent_s agent_t;

/* Constants */
#define AGENCY_FILE_NAME "agency.agerun"

/**
 * Set initialization state - called by system init
 * @param initialized Initialization state to set (value type, not a reference)
 */
void ar_agency_set_initialized(bool initialized);


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
 * @return Number of agents updated (each agent will have 2 messages queued)
 * @note Ownership: Does not take ownership of either method reference.
 *       The update process involves:
 *       1. Agent finishes processing current message
 *       2. Sleep message is sent to agent
 *       3. Method reference is updated
 *       4. Wake message is sent to agent
 *       IMPORTANT: The caller must process 2*update_count messages after this call
 *       to ensure all sleep and wake messages are processed.
 */
int ar_agency_update_agent_methods(const method_t *ref_old_method, const method_t *ref_new_method);

/**
 * Count the number of agents using a specific method
 * @param ref_method The method to check (borrowed reference)
 * @return Number of active agents using the method
 * @note Ownership: Does not take ownership of the method reference.
 */
int ar_agency_count_agents_using_method(const method_t *ref_method);

/**
 * Get the first active agent ID
 * @return First active agent ID, or 0 if no active agents
 */
int64_t ar_agency_get_first_agent(void);

/**
 * Get the next active agent ID after the given agent
 * @param current_id Current agent ID
 * @return Next active agent ID, or 0 if no more active agents
 */
int64_t ar_agency_get_next_agent(int64_t current_id);

/**
 * Check if an agent has messages in its queue
 * @param agent_id Agent ID to check
 * @return true if agent has messages, false otherwise
 */
bool ar_agency_agent_has_messages(int64_t agent_id);

/**
 * Get and remove the first message from an agent's queue
 * @param agent_id Agent ID
 * @return Message data (ownership transferred), or NULL if no messages
 * @note Ownership: Returns an owned value that caller must destroy
 */
data_t* ar_agency_get_agent_message(int64_t agent_id);

#endif /* AGERUN_AGENCY_H */


#ifndef AGERUN_SYSTEM_H
#define AGERUN_SYSTEM_H

#include <stdbool.h>
#include "agerun_agent.h"

/**
 * Initialize the Agerun runtime system
 * @param method_name Name of the initial method to run
 * @param version Version of the method (0 for latest)
 * @return ID of the created initial agent, or 0 on failure
 */
agent_id_t ar_init(const char *method_name, version_t version);

/**
 * Shut down the Agerun runtime system
 */
void ar_shutdown(void);



/**
 * Process the next pending message in the system
 * @return true if a message was processed, false if no messages
 */
bool ar_process_next_message(void);

/**
 * Process all pending messages in the system
 * @return Number of messages processed
 */
int ar_process_all_messages(void);

/**
 * Check if an agent exists
 * @param agent_id ID of the agent to check
 * @return true if the agent exists, false otherwise
 */
bool ar_agent_exists(agent_id_t agent_id);

/**
 * Get the current number of active agents
 * @return Number of active agents
 */
int ar_count_agents(void);

/**
 * Save all persistent agents to disk
 * @return true if successful, false otherwise
 */
bool ar_save_agents(void);

/**
 * Load all persistent agents from disk
 * @return true if successful, false otherwise
 */
bool ar_load_agents(void);

/**
 * Save all method definitions to disk
 * @return true if successful, false otherwise
 */
bool ar_save_methods(void);

/**
 * Load all method definitions from disk
 * @return true if successful, false otherwise
 */
bool ar_load_methods(void);

/**
 * Set a value in memory
 * @param memory Memory dictionary
 * @param key Key to set
 * @param value Value to set
 * @return true if successful, false otherwise
 */
bool ar_memory_set(void *memory, const char *key, void *value);

#endif /* AGERUN_SYSTEM_H */

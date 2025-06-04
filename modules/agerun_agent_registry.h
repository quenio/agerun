#ifndef AGERUN_AGENT_REGISTRY_H
#define AGERUN_AGENT_REGISTRY_H

#include <stdbool.h>
#include <stdint.h>

/* Forward declarations */
typedef struct data_s data_t;

/**
 * @file agerun_agent_registry.h
 * @brief Agent registry module for managing agent IDs and runtime registry
 * 
 * This module handles agent ID allocation, tracking active agents,
 * and providing iteration capabilities over the agent registry.
 * It serves as a central registry for all active agents in the system.
 */

/**
 * Initialize the agent registry
 * @return true if successful, false otherwise
 * @note Must be called before any other registry functions
 */
bool ar_agent_registry_initialize(void);

/**
 * Shutdown the agent registry and clean up resources
 * @note This does not destroy agents, only cleans up registry state
 */
void ar_agent_registry_shutdown(void);

/**
 * Check if the registry is initialized
 * @return true if initialized, false otherwise
 */
bool ar_agent_registry_is_initialized(void);

/**
 * Get the number of active agents in the registry
 * @return Number of active agents
 */
int ar_agent_registry_count(void);

/**
 * Get the first active agent ID
 * @return First active agent ID, or 0 if no active agents
 */
int64_t ar_agent_registry_get_first(void);

/**
 * Get the next active agent ID after the given agent
 * @param current_id Current agent ID
 * @return Next active agent ID, or 0 if no more active agents
 */
int64_t ar_agent_registry_get_next(int64_t current_id);


/**
 * Reset all agents in the registry
 * @note This destroys all agents and resets the registry state
 */
void ar_agent_registry_reset_all(void);

#endif /* AGERUN_AGENT_REGISTRY_H */

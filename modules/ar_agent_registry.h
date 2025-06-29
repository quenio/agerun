#ifndef AGERUN_AGENT_REGISTRY_H
#define AGERUN_AGENT_REGISTRY_H

#include <stdbool.h>
#include <stdint.h>

/* Forward declarations */
typedef struct data_s data_t;
typedef struct agent_registry_s agent_registry_t;

/**
 * @file agerun_agent_registry.h
 * @brief Agent registry module for managing agent IDs and runtime registry
 * 
 * This module handles agent ID allocation, tracking active agents,
 * and providing iteration capabilities over the agent registry.
 * It serves as a central registry for all active agents in the system.
 */

/**
 * Create a new agent registry
 * @return New agent registry instance, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy
 */
agent_registry_t* ar_agent_registry__create(void);

/**
 * Destroy an agent registry
 * @param own_registry The registry to destroy (ownership transferred)
 * @note This does not destroy agents, only the registry itself
 */
void ar_agent_registry__destroy(agent_registry_t *own_registry);

/**
 * Get the number of registered agents
 * @param ref_registry The registry to query (borrowed reference)
 * @return Number of registered agents
 */
int ar_agent_registry__count(const agent_registry_t *ref_registry);

/**
 * Get the first registered agent ID
 * @param ref_registry The registry to query (borrowed reference)
 * @return First registered agent ID, or 0 if no registered agents
 */
int64_t ar_agent_registry__get_first(const agent_registry_t *ref_registry);

/**
 * Get the next registered agent ID after the given agent
 * @param ref_registry The registry to query (borrowed reference)
 * @param current_id Current agent ID
 * @return Next registered agent ID, or 0 if no more registered agents
 */
int64_t ar_agent_registry__get_next(const agent_registry_t *ref_registry, int64_t current_id);

/**
 * Clear all agents from the registry
 * @param mut_registry The registry to clear (mutable reference)
 * @note This only clears the registry, it does not destroy agents
 */
void ar_agent_registry__clear(agent_registry_t *mut_registry);

/**
 * Get the next agent ID to be assigned
 * @param ref_registry The registry to query (borrowed reference)
 * @return Next agent ID
 */
int64_t ar_agent_registry__get_next_id(const agent_registry_t *ref_registry);

/**
 * Set the next agent ID
 * @param mut_registry The registry to modify (mutable reference)
 * @param id The next agent ID to set
 * @note This is used by agent_store module to prevent ID collisions
 */
void ar_agent_registry__set_next_id(agent_registry_t *mut_registry, int64_t id);

/**
 * Allocate a new agent ID
 * @param mut_registry The registry to modify (mutable reference)
 * @return Newly allocated agent ID
 */
int64_t ar_agent_registry__allocate_id(agent_registry_t *mut_registry);

/**
 * Register an agent ID
 * @param mut_registry The registry to modify (mutable reference)
 * @param agent_id The agent ID to register
 * @return true if successful, false otherwise
 */
bool ar_agent_registry__register_id(agent_registry_t *mut_registry, int64_t agent_id);

/**
 * Unregister an agent ID
 * @param mut_registry The registry to modify (mutable reference)
 * @param agent_id The agent ID to unregister
 * @return true if successful, false otherwise
 */
bool ar_agent_registry__unregister_id(agent_registry_t *mut_registry, int64_t agent_id);

/**
 * Check if an agent ID is registered
 * @param ref_registry The registry to query (borrowed reference)
 * @param agent_id The agent ID to check
 * @return true if registered, false otherwise
 */
bool ar_agent_registry__is_registered(const agent_registry_t *ref_registry, int64_t agent_id);

/**
 * Track an agent object in the registry
 * @param mut_registry The registry to modify (mutable reference)
 * @param agent_id The agent ID
 * @param mut_agent The agent object to track (mutable reference, not owned)
 * @return true if successful, false otherwise
 * @note The registry does not take ownership of the agent object
 */
bool ar_agent_registry__track_agent(agent_registry_t *mut_registry, int64_t agent_id, void *mut_agent);

/**
 * Untrack an agent object from the registry
 * @param mut_registry The registry to modify (mutable reference)
 * @param agent_id The agent ID
 * @return The tracked agent object, or NULL if not found
 * @note Returns a borrowed reference - caller should not free
 */
void* ar_agent_registry__untrack_agent(agent_registry_t *mut_registry, int64_t agent_id);

/**
 * Find a tracked agent by ID
 * @param ref_registry The registry to query (borrowed reference)
 * @param agent_id The agent ID to find
 * @return The agent object, or NULL if not found
 * @note Returns a borrowed reference - caller should not free
 */
void* ar_agent_registry__find_agent(const agent_registry_t *ref_registry, int64_t agent_id);

#endif /* AGERUN_AGENT_REGISTRY_H */

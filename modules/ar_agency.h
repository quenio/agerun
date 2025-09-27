#ifndef AGERUN_AGENCY_H
#define AGERUN_AGENCY_H

#include <stdbool.h>
#include <stdint.h>
#include "ar_data.h"
#include "ar_agent_registry.h"
#include "ar_log.h"

/* Forward declarations */
typedef struct ar_method_s ar_method_t;
typedef struct ar_agent_s ar_agent_t;
typedef struct ar_methodology_s ar_methodology_t;

/* Agency type */
typedef struct ar_agency_s ar_agency_t;

/* Constants */
#define AGENCY_FILE_NAME "agency.agerun"

/**
 * Create a new agency instance
 * @param ref_log The log instance to use for the agency's methodology (borrowed reference)
 * @return New agency instance (ownership transferred), or NULL on failure
 * @note Ownership: Caller must destroy the returned agency. The agency creates and owns its own methodology.
 */
ar_agency_t* ar_agency__create(ar_log_t *ref_log);

/**
 * Destroy an agency instance
 * @param own_agency The agency to destroy (ownership transferred)
 * @note Ownership: Destroys all agent resources and the agency itself.
 */
void ar_agency__destroy(ar_agency_t *own_agency);




/* Instance-based API functions */

/**
 * Get the current number of active agents (instance version)
 * @param ref_agency The agency instance (borrowed reference)
 * @return Number of active agents (value type, not a reference)
 */
int ar_agency__count_agents(ar_agency_t *ref_agency);

/**
 * Count the number of agents using a specific method (instance version)
 * @param ref_agency The agency instance (borrowed reference)
 * @param ref_method The method to check (borrowed reference)
 * @return Number of active agents using the method
 * @note Ownership: Does not take ownership of the method reference.
 */
int ar_agency__count_agents_using_method(ar_agency_t *ref_agency, const ar_method_t *ref_method);

/**
 * Update agents using a specific method to use a different method (instance version)
 * @param mut_agency The agency instance (mutable reference)
 * @param ref_old_method The old method being used (borrowed reference)
 * @param ref_new_method The new method to use (borrowed reference)
 * @return Number of agents updated
 * @note Ownership: Does not take ownership of either method reference.
 */
int ar_agency__update_agent_methods(ar_agency_t *mut_agency, const ar_method_t *ref_old_method, const ar_method_t *ref_new_method);

/**
 * Create a new agent with ID allocation and tracking (instance version)
 * @param mut_agency The agency instance (mutable reference)
 * @param ref_method_name Name of the method to use (borrowed reference)
 * @param ref_version Version string of the method (NULL for latest)
 * @param ref_context Context data (NULL for empty, borrowed reference)
 * @return Unique agent ID, or 0 on failure
 * @note Ownership: Function does not take ownership of ref_context.
 */
int64_t ar_agency__create_agent(ar_agency_t *mut_agency, 
                                               const char *ref_method_name, 
                                               const char *ref_version, 
                                               const ar_data_t *ref_context);

/**
 * Destroy an agent by ID (instance version)
 * @param mut_agency The agency instance (mutable reference)
 * @param agent_id ID of the agent to destroy
 * @return true if successful, false otherwise
 * @note Ownership: Destroys all resources owned by the agent.
 */
bool ar_agency__destroy_agent(ar_agency_t *mut_agency, int64_t agent_id);

/**
 * Send a message to an agent by ID (instance version)
 * @param mut_agency The agency instance (mutable reference)
 * @param agent_id ID of the agent to send to
 * @param own_message Message data (ownership is transferred on success)
 * @return true if successful, false otherwise
 * @note Ownership: Function takes ownership of own_message.
 */
bool ar_agency__send_to_agent(ar_agency_t *mut_agency, 
                                             int64_t agent_id, 
                                             ar_data_t *own_message);

/**
 * Get agent memory by ID (instance version)
 * @param ref_agency The agency instance (borrowed reference)
 * @param agent_id ID of the agent
 * @return Const pointer to agent's memory data, or NULL if agent doesn't exist
 * @note Ownership: Returns a borrowed reference.
 */
const ar_data_t* ar_agency__get_agent_memory(ar_agency_t *ref_agency, 
                                                            int64_t agent_id);

/**
 * Reset agency state (instance version)
 * @param mut_agency The agency instance (mutable reference)
 * @note Ownership: Destroys all agent resources and resets agency state.
 */
void ar_agency__reset(ar_agency_t *mut_agency);

/**
 * Save all persistent agents to disk (instance version)
 * @param ref_agency The agency instance (borrowed reference)
 * @param ref_filename Optional filename (NULL for default)
 * @return true if successful, false otherwise
 * @note Ownership: Does not affect ownership of any agent resources.
 */
bool ar_agency__save_agents(ar_agency_t *ref_agency, const char *ref_filename);

/**
 * Load all persistent agents from disk (instance version)
 * @param mut_agency The agency instance (mutable reference)
 * @param ref_filename Optional filename (NULL for default)
 * @return true if successful, false otherwise
 * @note Ownership: Creates new agents with their own resources.
 */
bool ar_agency__load_agents(ar_agency_t *mut_agency, const char *ref_filename);

/**
 * Get the registry from an agency instance
 * @param ref_agency The agency instance (borrowed reference)
 * @return The agent registry (borrowed reference), or NULL if not initialized
 */
ar_agent_registry_t* ar_agency__get_registry(ar_agency_t *ref_agency);

/**
 * Get the methodology from an agency instance
 * @param ref_agency The agency instance (borrowed reference)
 * @return The methodology instance, or NULL if not available
 * @note Ownership: Returns a borrowed reference.
 */
ar_methodology_t* ar_agency__get_methodology(ar_agency_t *ref_agency);

/**
 * Get the first agent ID in the agency (instance version)
 * @param ref_agency The agency instance (borrowed reference)
 * @return The first agent ID, or 0 if no agents exist
 */
int64_t ar_agency__get_first_agent(ar_agency_t *ref_agency);

/**
 * Get the next agent ID after the given one (instance version)
 * @param ref_agency The agency instance (borrowed reference)
 * @param current_id The current agent ID
 * @return The next agent ID, or 0 if no more agents
 */
int64_t ar_agency__get_next_agent(ar_agency_t *ref_agency, int64_t current_id);

/**
 * Check if an agent has pending messages (instance version)
 * @param ref_agency The agency instance (borrowed reference)
 * @param agent_id The agent ID to check
 * @return true if the agent has messages, false otherwise
 */
bool ar_agency__agent_has_messages(ar_agency_t *ref_agency, int64_t agent_id);

/**
 * Get the next message for an agent (instance version)
 * @param mut_agency The agency instance (mutable reference)
 * @param agent_id The agent ID
 * @return The message data (ownership transferred), or NULL if no messages
 * @note Ownership: Caller takes ownership of the returned message.
 */
ar_data_t* ar_agency__get_agent_message(ar_agency_t *mut_agency, int64_t agent_id);

/**
 * Get agent method by ID (instance version)
 * @param ref_agency The agency instance (borrowed reference)
 * @param agent_id ID of the agent
 * @return Const pointer to agent's method, or NULL if agent doesn't exist
 * @note Ownership: Returns a borrowed reference.
 */
const ar_method_t* ar_agency__get_agent_method(ar_agency_t *ref_agency, int64_t agent_id);

/**
 * Get mutable agent memory by ID (instance version)
 * @param mut_agency The agency instance (mutable reference)
 * @param agent_id ID of the agent
 * @return Mutable pointer to agent's memory data
 * @note Ownership: Returns a mutable reference, do not destroy.
 */
ar_data_t* ar_agency__get_agent_mutable_memory(ar_agency_t *mut_agency, int64_t agent_id);

/**
 * Get agent context by ID (instance version)
 * @param ref_agency The agency instance (borrowed reference)
 * @param agent_id ID of the agent
 * @return Const pointer to agent's context data, or NULL if agent doesn't exist
 * @note Ownership: Returns a borrowed reference.
 */
const ar_data_t* ar_agency__get_agent_context(ar_agency_t *ref_agency, int64_t agent_id);

/**
 * Check if an agent exists using a specific agency instance
 * @param ref_agency The agency instance to use (borrowed reference)
 * @param agent_id ID of the agent to check
 * @return true if the agent exists, false otherwise
 */
bool ar_agency__agent_exists(ar_agency_t *ref_agency, int64_t agent_id);

#endif /* AGERUN_AGENCY_H */


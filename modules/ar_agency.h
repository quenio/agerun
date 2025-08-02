#ifndef AGERUN_AGENCY_H
#define AGERUN_AGENCY_H

#include <stdbool.h>
#include <stdint.h>
#include "ar_data.h"
#include "ar_agent_registry.h"

/* Forward declarations */
typedef struct ar_method_s ar_method_t;
typedef struct ar_agent_s ar_agent_t;
typedef struct ar_agency_s ar_agency_t;
typedef struct ar_methodology_s ar_methodology_t;

/* Constants */
#define AGENCY_FILE_NAME "agency.agerun"

/**
 * Create a new agency instance
 * @param ref_methodology The methodology instance to use (borrowed reference)
 * @return New agency instance (ownership transferred), or NULL on failure
 * @note Ownership: Caller must destroy the returned agency.
 */
ar_agency_t* ar_agency__create(ar_methodology_t *ref_methodology);

/**
 * Destroy an agency instance
 * @param own_agency The agency to destroy (ownership transferred)
 * @note Ownership: Destroys all agent resources and the agency itself.
 */
void ar_agency__destroy(ar_agency_t *own_agency);

/**
 * Set initialization state - called by system init
 * @param initialized Initialization state to set (value type, not a reference)
 */
void ar_agency__set_initialized(bool initialized);


/**
 * Reset agency state (used during shutdown)
 * @note Ownership: Destroys all agent resources and resets global state.
 */
void ar_agency__reset(void);

/**
 * Get the current number of active agents
 * @return Number of active agents (value type, not a reference)
 */
int ar_agency__count_agents(void);

/**
 * Save all persistent agents to disk
 * @return true if successful, false otherwise
 * @note Ownership: Does not affect ownership of any agent resources.
 */
bool ar_agency__save_agents(void);

/**
 * Load all persistent agents from disk
 * @return true if successful, false otherwise
 * @note Ownership: Creates new agents with their own resources.
 */
bool ar_agency__load_agents(void);

/**
 * Update agents using a specific method to use a different method
 * @param ref_old_method The old method being used (borrowed reference)
 * @param ref_new_method The new method to use (borrowed reference)
 * @param send_lifecycle_events If true, send sleep/wake messages for each updated agent
 * @return Number of agents updated (each agent will have 2 messages queued if lifecycle events are sent)
 * @note Ownership: Does not take ownership of either method reference.
 *       If send_lifecycle_events is true, the update process involves:
 *       1. Agent finishes processing current message
 *       2. Sleep message is sent to agent
 *       3. Method reference is updated
 *       4. Wake message is sent to agent
 *       IMPORTANT: The caller must process 2*update_count messages after this call
 *       to ensure all sleep and wake messages are processed.
 */
int ar_agency__update_agent_methods(const ar_method_t *ref_old_method, const ar_method_t *ref_new_method, bool send_lifecycle_events);

/**
 * Count the number of agents using a specific method
 * @param ref_method The method to check (borrowed reference)
 * @return Number of active agents using the method
 * @note Ownership: Does not take ownership of the method reference.
 */
int ar_agency__count_agents_using_method(const ar_method_t *ref_method);

/**
 * Get the first active agent ID
 * @return First active agent ID, or 0 if no active agents
 */
int64_t ar_agency__get_first_agent(void);

/**
 * Get the next active agent ID after the given agent
 * @param current_id Current agent ID
 * @return Next active agent ID, or 0 if no more active agents
 */
int64_t ar_agency__get_next_agent(int64_t current_id);

/**
 * Check if an agent has messages in its queue
 * @param agent_id Agent ID to check
 * @return true if agent has messages, false otherwise
 */
bool ar_agency__agent_has_messages(int64_t agent_id);

/**
 * Get and remove the first message from an agent's queue
 * @param agent_id Agent ID
 * @return Message data (ownership transferred), or NULL if no messages
 * @note Ownership: Returns an owned value that caller must destroy
 */
ar_data_t* ar_agency__get_agent_message(int64_t agent_id);

/**
 * Create a new agent with ID allocation and tracking
 * @param ref_method_name Name of the method to use (borrowed reference)
 * @param ref_version Version string of the method (NULL for latest)
 * @param ref_context Context data (NULL for empty, borrowed reference)
 * @return Unique agent ID, or 0 on failure
 * @note Ownership: Function does not take ownership of ref_context.
 */
int64_t ar_agency__create_agent(const char *ref_method_name, const char *ref_version, const ar_data_t *ref_context);

/**
 * Destroy an agent by ID
 * @param agent_id ID of the agent to destroy
 * @return true if successful, false otherwise
 * @note Ownership: Destroys all resources owned by the agent.
 */
bool ar_agency__destroy_agent(int64_t agent_id);

/**
 * Send a message to an agent by ID
 * @param agent_id ID of the agent to send to
 * @param own_message Message data (ownership is transferred on success)
 * @return true if successful, false otherwise
 * @note Ownership: Function takes ownership of own_message.
 */
bool ar_agency__send_to_agent(int64_t agent_id, ar_data_t *own_message);

/**
 * Check if an agent exists
 * @param agent_id ID of the agent to check
 * @return true if the agent exists, false otherwise
 */
bool ar_agency__agent_exists(int64_t agent_id);

/**
 * Get agent memory by ID
 * @param agent_id ID of the agent
 * @return Const pointer to agent's memory data, or NULL if agent doesn't exist
 * @note Ownership: Returns a borrowed reference.
 */
const ar_data_t* ar_agency__get_agent_memory(int64_t agent_id);

/**
 * Get agent context by ID
 * @param agent_id ID of the agent
 * @return Const pointer to agent's context data, or NULL if agent doesn't exist
 * @note Ownership: Returns a borrowed reference.
 */
const ar_data_t* ar_agency__get_agent_context(int64_t agent_id);

/**
 * Check if an agent is active
 * @param agent_id ID of the agent to check
 * @return true if the agent is active, false otherwise
 */
bool ar_agency__is_agent_active(int64_t agent_id);

/**
 * Get agent method by ID
 * @param agent_id ID of the agent
 * @return Const pointer to agent's method, or NULL if agent doesn't exist
 * @note Ownership: Returns a borrowed reference.
 */
const ar_method_t* ar_agency__get_agent_method(int64_t agent_id);

/**
 * Get agent's method info by ID
 * @param agent_id ID of the agent
 * @param out_method_name Output pointer for method name (optional, can be NULL)
 * @param out_method_version Output pointer for method version (optional, can be NULL)
 * @return true if agent has a method, false otherwise
 * @note Ownership: The returned strings are borrowed references.
 */
bool ar_agency__get_agent_method_info(int64_t agent_id, const char **out_method_name, const char **out_method_version);

/**
 * Get mutable agent memory by ID (for internal use)
 * @param agent_id ID of the agent
 * @return Mutable pointer to agent's memory data
 * @note Ownership: Returns a mutable reference, do not destroy.
 */
ar_data_t* ar_agency__get_agent_mutable_memory(int64_t agent_id);

/**
 * Update agent method by ID
 * @param agent_id ID of the agent to update
 * @param ref_new_method The new method (borrowed reference)
 * @param send_sleep_wake If true, send sleep/wake messages during update
 * @return true if successful, false otherwise
 */
bool ar_agency__update_agent_method(int64_t agent_id, const ar_method_t *ref_new_method, bool send_sleep_wake);

/**
 * Set agent active status by ID
 * @param agent_id ID of the agent
 * @param is_active New active status
 * @return true if successful, false otherwise
 */
bool ar_agency__set_agent_active(int64_t agent_id, bool is_active);

/**
 * Count all active agents
 * @return Number of active agents
 */
int ar_agency__count_active_agents(void);

/**
 * Set agent ID (for persistence restoration)
 * @param old_id The current agent ID
 * @param new_id The new agent ID to set
 * @return true if successful, false if agent not found
 */
bool ar_agency__set_agent_id(int64_t old_id, int64_t new_id);

/**
 * Get the agent registry (for internal modules only)
 * @return The agent registry (borrowed reference), or NULL if not initialized
 * @note This is provided for agent_store module to access registry functions.
 */
ar_agent_registry_t* ar_agency__get_registry(void);

/* Instance-based API functions */

/**
 * Get the current number of active agents (instance version)
 * @param ref_agency The agency instance (borrowed reference)
 * @return Number of active agents (value type, not a reference)
 */
int ar_agency__count_agents_with_instance(ar_agency_t *ref_agency);

/**
 * Create a new agent with ID allocation and tracking (instance version)
 * @param mut_agency The agency instance (mutable reference)
 * @param ref_method_name Name of the method to use (borrowed reference)
 * @param ref_version Version string of the method (NULL for latest)
 * @param ref_context Context data (NULL for empty, borrowed reference)
 * @return Unique agent ID, or 0 on failure
 * @note Ownership: Function does not take ownership of ref_context.
 */
int64_t ar_agency__create_agent_with_instance(ar_agency_t *mut_agency, 
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
bool ar_agency__destroy_agent_with_instance(ar_agency_t *mut_agency, int64_t agent_id);

/**
 * Send a message to an agent by ID (instance version)
 * @param mut_agency The agency instance (mutable reference)
 * @param agent_id ID of the agent to send to
 * @param own_message Message data (ownership is transferred on success)
 * @return true if successful, false otherwise
 * @note Ownership: Function takes ownership of own_message.
 */
bool ar_agency__send_to_agent_with_instance(ar_agency_t *mut_agency, 
                                             int64_t agent_id, 
                                             ar_data_t *own_message);

/**
 * Get agent memory by ID (instance version)
 * @param ref_agency The agency instance (borrowed reference)
 * @param agent_id ID of the agent
 * @return Const pointer to agent's memory data, or NULL if agent doesn't exist
 * @note Ownership: Returns a borrowed reference.
 */
const ar_data_t* ar_agency__get_agent_memory_with_instance(ar_agency_t *ref_agency, 
                                                            int64_t agent_id);

/**
 * Reset agency state (instance version)
 * @param mut_agency The agency instance (mutable reference)
 * @note Ownership: Destroys all agent resources and resets agency state.
 */
void ar_agency__reset_with_instance(ar_agency_t *mut_agency);

/**
 * Save all persistent agents to disk (instance version)
 * @param ref_agency The agency instance (borrowed reference)
 * @param ref_filename Optional filename (NULL for default)
 * @return true if successful, false otherwise
 * @note Ownership: Does not affect ownership of any agent resources.
 */
bool ar_agency__save_agents_with_instance(ar_agency_t *ref_agency, const char *ref_filename);

/**
 * Load all persistent agents from disk (instance version)
 * @param mut_agency The agency instance (mutable reference)
 * @param ref_filename Optional filename (NULL for default)
 * @return true if successful, false otherwise
 * @note Ownership: Creates new agents with their own resources.
 */
bool ar_agency__load_agents_with_instance(ar_agency_t *mut_agency, const char *ref_filename);

/**
 * Get the registry from an agency instance
 * @param ref_agency The agency instance (borrowed reference)
 * @return The agent registry (borrowed reference), or NULL if not initialized
 */
ar_agent_registry_t* ar_agency__get_registry_with_instance(ar_agency_t *ref_agency);

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
int64_t ar_agency__get_first_agent_with_instance(ar_agency_t *ref_agency);

/**
 * Get the next agent ID after the given one (instance version)
 * @param ref_agency The agency instance (borrowed reference)
 * @param current_id The current agent ID
 * @return The next agent ID, or 0 if no more agents
 */
int64_t ar_agency__get_next_agent_with_instance(ar_agency_t *ref_agency, int64_t current_id);

/**
 * Check if an agent has pending messages (instance version)
 * @param ref_agency The agency instance (borrowed reference)
 * @param agent_id The agent ID to check
 * @return true if the agent has messages, false otherwise
 */
bool ar_agency__agent_has_messages_with_instance(ar_agency_t *ref_agency, int64_t agent_id);

/**
 * Get the next message for an agent (instance version)
 * @param mut_agency The agency instance (mutable reference)
 * @param agent_id The agent ID
 * @return The message data (ownership transferred), or NULL if no messages
 * @note Ownership: Caller takes ownership of the returned message.
 */
ar_data_t* ar_agency__get_agent_message_with_instance(ar_agency_t *mut_agency, int64_t agent_id);

#endif /* AGERUN_AGENCY_H */


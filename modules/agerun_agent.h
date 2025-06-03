#ifndef AGERUN_AGENT_H
#define AGERUN_AGENT_H

#include <stdbool.h>
#include <stdint.h>
#include "agerun_data.h"
#include "agerun_list.h"

/* Constants */
#define MAX_AGENTS 1024

/* Forward declaration for method_t */
typedef struct method_s method_t;

/* Opaque agent type */
typedef struct agent_s agent_t;

/**
 * Create a new agent instance
 * @param ref_method_name Name of the method to use (borrowed reference)
 * @param ref_version Version string of the method (NULL for latest)
 * @param ref_context Context data (NULL for empty, borrowed reference, not owned by agent)
 * @return Unique agent ID, or 0 on failure
 * @note Ownership: Function does not take ownership of ref_context, it just references it.
 */
int64_t ar_agent_create(const char *ref_method_name, const char *ref_version, const data_t *ref_context);

/**
 * Destroy an agent instance
 * @param agent_id ID of the agent to destroy (value type, not a reference)
 * @return true if successful, false otherwise
 * @note Ownership: Destroys all resources owned by the agent (memory, message queue, context).
 */
bool ar_agent_destroy(int64_t agent_id);

/**
 * Send a message to an agent
 * @param agent_id ID of the agent to send to
 * @param own_message Message data (any supported data type - ownership is transferred on success)
 * @return true if successful, false otherwise
 * @note Ownership: Function takes ownership of own_message.
 *       Caller should set own_message = NULL after this call.
 *       If sending fails, function will destroy the message.
 */
bool ar_agent_send(int64_t agent_id, data_t *own_message);

/**
 * Check if an agent exists
 * @param agent_id ID of the agent to check (value type, not a reference)
 * @return true if the agent exists, false otherwise
 */
bool ar_agent_exists(int64_t agent_id);

/**
 * Get the memory data of an agent
 * @param agent_id ID of the agent
 * @return Const pointer to agent's memory data, or NULL if agent doesn't exist
 * @note Ownership: Returns a borrowed reference to the agent's memory.
 *       Caller must not modify or destroy the returned data.
 */
const data_t* ar_agent_get_memory(int64_t agent_id);

/**
 * Get the mutable memory data of an agent (for internal use)
 * @param agent_id ID of the agent
 * @return Mutable pointer to agent's memory data, or NULL if agent doesn't exist
 * @note Ownership: Returns a mutable reference to the agent's memory.
 *       Caller must not destroy the returned data.
 */
data_t* ar_agent_get_mutable_memory(int64_t agent_id);

/**
 * Get the context data of an agent
 * @param agent_id ID of the agent
 * @return Const pointer to agent's context data, or NULL if agent doesn't exist or has no context
 * @note Ownership: Returns a borrowed reference to the agent's context.
 *       Caller must not modify or destroy the returned data.
 */
const data_t* ar_agent_get_context(int64_t agent_id);

/**
 * Check if an agent is active
 * @param agent_id ID of the agent to check
 * @return true if the agent is active, false otherwise
 */
bool ar_agent_is_active(int64_t agent_id);

/**
 * Get the method information of an agent
 * @param agent_id ID of the agent
 * @param out_method_name Output pointer for method name (optional, can be NULL)
 * @param out_method_version Output pointer for method version (optional, can be NULL)
 * @return true if agent exists and has a method, false otherwise
 * @note Ownership: The returned strings are borrowed references.
 *       Caller must not modify or free the returned strings.
 */
bool ar_agent_get_method_info(int64_t agent_id, const char **out_method_name, const char **out_method_version);

/**
 * Get the method reference of an agent (for internal use)
 * @param agent_id ID of the agent
 * @return Const pointer to agent's method, or NULL if agent doesn't exist
 * @note Ownership: Returns a borrowed reference to the agent's method.
 *       Caller must not modify or destroy the returned method.
 */
const method_t* ar_agent_get_method(int64_t agent_id);

/**
 * Set the active status of an agent (for internal use)
 * @param agent_id ID of the agent
 * @param is_active New active status
 * @return true if successful, false if agent doesn't exist
 */
bool ar_agent_set_active(int64_t agent_id, bool is_active);

/**
 * Get direct access to an agent structure (for internal use only)
 * @param agent_id ID of the agent
 * @return Pointer to agent structure, or NULL if agent doesn't exist
 * @note This function is for internal use by closely coupled modules only.
 *       It will be removed once all modules are updated to use accessor functions.
 */
agent_t* ar_agent_get_internal(int64_t agent_id);

/**
 * Get agents array - internal use only by agency module
 * @return Pointer to the agents array (borrowed reference)
 * @note Ownership: Returns a borrowed reference that caller must not destroy.
 * @note This is for agency module use only during refactoring.
 */
agent_t* ar_agent_get_agents_internal(void);

/**
 * Get next agent ID - internal use only by agency module
 * @return Next agent ID to be assigned (value type, not a reference)
 * @note This is for agency module use only during refactoring.
 */
int64_t ar_agent_get_next_id_internal(void);

/**
 * Set next agent ID - internal use only by agency module
 * @param id Next agent ID to set (value type, not a reference)
 * @note This is for agency module use only during refactoring.
 */
void ar_agent_set_next_id_internal(int64_t id);

/**
 * Reset all agents - internal use only by agency module
 * @note This destroys all agent resources and resets the agent array
 */
void ar_agent_reset_all(void);

/**
 * Get the first active agent ID
 * @return First active agent ID, or 0 if no active agents
 */
int64_t ar_agent_get_first_active(void);

/**
 * Get the next active agent ID after the given agent
 * @param current_id Current agent ID
 * @return Next active agent ID, or 0 if no more active agents
 */
int64_t ar_agent_get_next_active(int64_t current_id);

/**
 * Check if an agent has messages in its queue
 * @param agent_id Agent ID to check
 * @return true if agent has messages, false otherwise
 */
bool ar_agent_has_messages(int64_t agent_id);

/**
 * Get and remove the first message from an agent's queue
 * @param agent_id Agent ID
 * @return Message data (ownership transferred), or NULL if no messages
 * @note Ownership: Returns an owned value that caller must destroy
 */
data_t* ar_agent_get_message(int64_t agent_id);

/**
 * Count all active agents
 * @return Number of active agents
 */
int ar_agent_count_active(void);

/**
 * Get list of all active agent IDs
 * @return List of agent IDs (ownership transferred)
 * @note Ownership: Returns an owned list that caller must destroy with ar_list_destroy()
 */
list_t* ar_agent_get_active_list(void);

/**
 * Count agents using a specific method
 * @param ref_method The method to check (borrowed reference)
 * @return Number of active agents using the method
 */
int ar_agent_count_by_method(const method_t *ref_method);

/**
 * Update the method for all agents using a specific method
 * @param ref_old_method The old method (borrowed reference)
 * @param ref_new_method The new method (borrowed reference)
 * @param send_sleep_wake If true, send sleep/wake messages during update
 * @return Number of agents updated
 */
int ar_agent_update_method(const method_t *ref_old_method, const method_t *ref_new_method, bool send_sleep_wake);

/**
 * Get agent info for persistence
 * @param agent_id The agent ID
 * @param out_method Output pointer for method (borrowed reference)
 * @param out_memory Output pointer for memory (borrowed reference)
 * @return true if agent exists and info retrieved, false otherwise
 */
bool ar_agent_get_persistence_info(int64_t agent_id, const method_t **out_method, const data_t **out_memory);

/**
 * Set agent ID (for persistence restoration)
 * @param old_id The current agent ID
 * @param new_id The new agent ID to set
 * @return true if successful, false if agent not found
 */
bool ar_agent_set_id(int64_t old_id, int64_t new_id);

#endif /* AGERUN_AGENT_H */

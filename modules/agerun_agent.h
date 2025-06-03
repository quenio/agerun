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

#endif /* AGERUN_AGENT_H */

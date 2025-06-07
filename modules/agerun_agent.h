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
 * @return New agent instance (ownership transferred), or NULL on failure
 * @note Ownership: Function does not take ownership of ref_context, it just references it.
 *       Caller takes ownership of the returned agent.
 */
agent_t* ar_agent_create(const char *ref_method_name, const char *ref_version, const data_t *ref_context);

/**
 * Destroy an agent instance
 * @param own_agent Agent to destroy (ownership transferred)
 * @note Ownership: Destroys all resources owned by the agent (memory, message queue).
 */
void ar_agent_destroy(agent_t *own_agent);

/**
 * Send a message to an agent
 * @param mut_agent Agent to send to (mutable reference)
 * @param own_message Message data (any supported data type - ownership is transferred on success)
 * @return true if successful, false otherwise
 * @note Ownership: Function takes ownership of own_message.
 *       Caller should set own_message = NULL after this call.
 *       If sending fails, function will destroy the message.
 */
bool ar_agent_send(agent_t *mut_agent, data_t *own_message);

/**
 * Get the ID of an agent
 * @param ref_agent Agent to query (borrowed reference)
 * @return Agent's ID, or 0 if agent is NULL
 */
int64_t ar_agent_get_id(const agent_t *ref_agent);

/**
 * Get the memory data of an agent
 * @param ref_agent Agent to query (borrowed reference)
 * @return Const pointer to agent's memory data, or NULL if agent is NULL
 * @note Ownership: Returns a borrowed reference to the agent's memory.
 *       Caller must not modify or destroy the returned data.
 */
const data_t* ar_agent_get_memory(const agent_t *ref_agent);

/**
 * Get the mutable memory data of an agent (for internal use)
 * @param mut_agent Agent to query (mutable reference)
 * @return Mutable pointer to agent's memory data, or NULL if agent is NULL
 * @note Ownership: Returns a mutable reference to the agent's memory.
 *       Caller must not destroy the returned data.
 */
data_t* ar_agent_get_mutable_memory(agent_t *mut_agent);

/**
 * Get the context data of an agent
 * @param ref_agent Agent to query (borrowed reference)
 * @return Const pointer to agent's context data, or NULL if agent is NULL or has no context
 * @note Ownership: Returns a borrowed reference to the agent's context.
 *       Caller must not modify or destroy the returned data.
 */
const data_t* ar_agent_get_context(const agent_t *ref_agent);

/**
 * Check if an agent is active
 * @param ref_agent Agent to check (borrowed reference)
 * @return true if the agent is active, false otherwise
 */
bool ar_agent_is_active(const agent_t *ref_agent);

/**
 * Get the method information of an agent
 * @param ref_agent Agent to query (borrowed reference)
 * @param out_method_name Output pointer for method name (optional, can be NULL)
 * @param out_method_version Output pointer for method version (optional, can be NULL)
 * @return true if agent has a method, false otherwise
 * @note Ownership: The returned strings are borrowed references.
 *       Caller must not modify or free the returned strings.
 */
bool ar_agent_get_method_info(const agent_t *ref_agent, const char **out_method_name, const char **out_method_version);

/**
 * Get the method reference of an agent (for internal use)
 * @param ref_agent Agent to query (borrowed reference)
 * @return Const pointer to agent's method, or NULL if agent is NULL
 * @note Ownership: Returns a borrowed reference to the agent's method.
 *       Caller must not modify or destroy the returned method.
 */
const method_t* ar_agent_get_method(const agent_t *ref_agent);

/**
 * Set the active status of an agent (for internal use)
 * @param mut_agent Agent to modify (mutable reference)
 * @param is_active New active status
 */
void ar_agent_set_active(agent_t *mut_agent, bool is_active);



/**
 * Check if an agent has messages in its queue
 * @param ref_agent Agent to check (borrowed reference)
 * @return true if agent has messages, false otherwise
 */
bool ar_agent_has_messages(const agent_t *ref_agent);

/**
 * Get and remove the first message from an agent's queue
 * @param mut_agent Agent to get message from (mutable reference)
 * @return Message data (ownership transferred), or NULL if no messages
 * @note Ownership: Returns an owned value that caller must destroy
 */
data_t* ar_agent_get_message(agent_t *mut_agent);


/**
 * Update the method for an agent
 * @param mut_agent Agent to update (mutable reference)
 * @param ref_new_method The new method (borrowed reference)
 * @param send_sleep_wake If true, send sleep/wake messages during update
 * @return true if successful, false otherwise
 */
bool ar_agent_update_method(agent_t *mut_agent, const method_t *ref_new_method, bool send_sleep_wake);

/**
 * Set agent ID (for persistence restoration)
 * @param mut_agent The agent to modify (mutable reference)
 * @param new_id The new agent ID to set
 */
void ar_agent_set_id(agent_t *mut_agent, int64_t new_id);

#endif /* AGERUN_AGENT_H */

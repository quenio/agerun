#ifndef AGERUN_AGENT_H
#define AGERUN_AGENT_H

#include <stdbool.h>
#include <stdint.h>
#include "agerun_data.h"
#include "agerun_list.h"

/* Constants */
#define MAX_AGENTS 1024

/* Type definitions */
typedef int64_t agent_id_t;

/* Forward declaration for method_t */
typedef struct method_s method_t;

/* Agent Definition */
typedef struct agent_s {
    agent_id_t id;
    const method_t *ref_method; // Borrowed reference to method
    bool is_active;
    list_t *own_message_queue;  // Using list as a message queue, owned by agent
    data_t *own_memory;        // Memory owned by agent
    const data_t *ref_context;  // Context is read-only reference, not owned
} agent_t;

/**
 * Create a new agent instance
 * @param ref_method_name Name of the method to use (borrowed reference)
 * @param ref_version Version string of the method (NULL for latest)
 * @param ref_context Context data (NULL for empty, borrowed reference, not owned by agent)
 * @return Unique agent ID, or 0 on failure
 * @note Ownership: Function does not take ownership of ref_context, it just references it.
 */
agent_id_t ar_agent_create(const char *ref_method_name, const char *ref_version, const data_t *ref_context);

/**
 * Destroy an agent instance
 * @param agent_id ID of the agent to destroy (value type, not a reference)
 * @return true if successful, false otherwise
 * @note Ownership: Destroys all resources owned by the agent (memory, message queue, context).
 */
bool ar_agent_destroy(agent_id_t agent_id);

/**
 * Send a message to an agent
 * @param agent_id ID of the agent to send to
 * @param own_message Message data (any supported data type - ownership is transferred on success)
 * @return true if successful, false otherwise
 * @note Ownership: Function takes ownership of own_message.
 *       Caller should set own_message = NULL after this call.
 *       If sending fails, function will destroy the message.
 */
bool ar_agent_send(agent_id_t agent_id, data_t *own_message);

/**
 * Check if an agent exists
 * @param agent_id ID of the agent to check (value type, not a reference)
 * @return true if the agent exists, false otherwise
 */
bool ar_agent_exists(agent_id_t agent_id);

#endif /* AGERUN_AGENT_H */

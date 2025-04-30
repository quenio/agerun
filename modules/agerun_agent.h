#ifndef AGERUN_AGENT_H
#define AGERUN_AGENT_H

#include <stdbool.h>
#include <stdint.h>
#include "agerun_data.h"
#include "agerun_list.h"

/* Constants */
#define MAX_AGENTS 1024
#define MAX_METHOD_NAME_LENGTH 64

/* Type definitions */
typedef int64_t agent_id_t;
typedef int32_t version_t;

/* Agent Definition */
typedef struct agent_s {
    agent_id_t id;
    char method_name[MAX_METHOD_NAME_LENGTH];
    version_t method_version;
    bool is_active;
    bool is_persistent;
    list_t *own_message_queue;  // Using list as a message queue, owned by agent
    data_t *own_memory;        // Memory owned by agent
    data_t *mut_context;       // Context mutable by agent but not owned
} agent_t;

/**
 * Create a new agent instance
 * @param ref_method_name Name of the method to use (borrowed reference)
 * @param version Version of the method (0 for latest)
 * @param mut_context Context data (NULL for empty, mutable reference, not owned by agent)
 * @return Unique agent ID, or 0 on failure
 * @note Ownership: Function does not take ownership of mut_context, it just references it.
 */
agent_id_t ar_agent_create(const char *ref_method_name, version_t version, data_t *mut_context);

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

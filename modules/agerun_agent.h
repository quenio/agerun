#ifndef AGERUN_AGENT_H
#define AGERUN_AGENT_H

#include <stdbool.h>
#include <stdint.h>
#include "agerun_data.h"
#include "agerun_map.h"
#include "agerun_queue.h"

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
    queue_t queue;
    map_t *memory;
    map_t *context;
} agent_t;

/**
 * Create a new agent instance
 * @param method_name Name of the method to use
 * @param version Version of the method (0 for latest)
 * @param context Context dictionary (NULL for empty)
 * @return Unique agent ID, or 0 on failure
 */
agent_id_t ar_agent_create(const char *method_name, version_t version, void *context);

/**
 * Destroy an agent instance
 * @param agent_id ID of the agent to destroy
 * @return true if successful, false otherwise
 */
bool ar_agent_destroy(agent_id_t agent_id);

/**
 * Send a message to an agent
 * @param agent_id ID of the agent to send to
 * @param message Message content
 * @return true if successful, false otherwise
 */
bool ar_agent_send(agent_id_t agent_id, const char *message);

/**
 * Check if an agent exists
 * @param agent_id ID of the agent to check
 * @return true if the agent exists, false otherwise
 */
bool ar_agent_exists(agent_id_t agent_id);

#endif /* AGERUN_AGENT_H */

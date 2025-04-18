#ifndef AGERUN_AGENT_H
#define AGERUN_AGENT_H

#include <stdbool.h>
#include <stdint.h>
#include "agerun_data.h"
#include "agerun_queue.h"

/* Constants */
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
    dict_t memory;
    dict_t *context;
} agent_t;

#endif /* AGERUN_AGENT_H */

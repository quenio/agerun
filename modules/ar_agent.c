/* Agerun Agent Implementation */
#include "ar_agent.h"
#include "ar_method.h"
#include "ar_list.h"
#include "ar_heap.h"
#include "ar_map.h"
#include "ar_data.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>

/* Agent Definition (moved from header for opaque type) */
struct ar_agent_s {
    int64_t id;
    const ar_method_t *ref_method; // Borrowed reference to method
    bool is_active;
    ar_list_t *own_message_queue;  // Using list as a message queue, owned by agent
    ar_data_t *own_memory;        // Memory owned by agent
    const ar_data_t *ref_context;  // Context is read-only reference, not owned
};

/* Constants */

/* Implementation */

ar_agent_t* ar_agent__create_with_method(const ar_method_t *ref_method, const ar_data_t *ref_context) {
    if (!ref_method) {
        return NULL;
    }
    
    // Create agent structure
    ar_agent_t *own_agent = AR__HEAP__MALLOC(sizeof(ar_agent_t), "agent");
    if (!own_agent) {
        return NULL;
    }
    
    own_agent->id = 0;  // ID will be set by agency when it registers the agent
    own_agent->is_active = true;
    own_agent->ref_method = ref_method;  // Just store reference
    own_agent->ref_context = ref_context;  // Store reference, we don't own it
    
    own_agent->own_message_queue = ar_list__create();
    if (!own_agent->own_message_queue) {
        AR__HEAP__FREE(own_agent);
        return NULL;
    }
    
    own_agent->own_memory = ar_data__create_map();
    if (!own_agent->own_memory) {
        ar_list__destroy(own_agent->own_message_queue);
        AR__HEAP__FREE(own_agent);
        return NULL;
    }
    
    return own_agent;
}

/* ar_agent__create removed - use ar_agency__create_agent instead */

void ar_agent__destroy(ar_agent_t *own_agent) {
    if (!own_agent) {
        return;
    }
    
    // Destroy memory if owned
    if (own_agent->own_memory) {
        ar_data__destroy(own_agent->own_memory);
        own_agent->own_memory = NULL;
    }
    
    // We don't own the context, just clear the reference
    own_agent->ref_context = NULL;
    
    // Destroy any pending messages and the queue
    if (own_agent->own_message_queue) {
        // First, destroy any remaining messages in the queue
        ar_data_t *own_msg = NULL;
        while ((own_msg = ar_list__remove_first(own_agent->own_message_queue)) != NULL) {
            // Transfer ownership back before destroying
            ar_data__destroy_if_owned(own_msg, own_agent);
            own_msg = NULL; // Mark as destroyed
        }
        // Then destroy the queue itself
        ar_list__destroy(own_agent->own_message_queue);
        own_agent->own_message_queue = NULL; // Mark as destroyed
    }
    
    // Free the agent structure
    AR__HEAP__FREE(own_agent);
}

bool ar_agent__send(ar_agent_t *mut_agent, ar_data_t *own_message) {
    if (!mut_agent || !own_message) {
        // Destroy the message if we have one but no agent
        if (own_message) {
            // We don't know who owns it, just try to destroy
            ar_data__destroy(own_message);
        }
        return false;
    }
    
    // DEBUG: Log direct agent sends (internal)
    fprintf(stderr, "DEBUG [AGENT_SEND]: Direct send to agent id=%lld, ", (long long)mut_agent->id);
    switch (ar_data__get_type(own_message)) {
        case AR_DATA_TYPE__INTEGER:
            fprintf(stderr, "type=INTEGER, value=%lld\n", (long long)ar_data__get_integer(own_message));
            break;
        case AR_DATA_TYPE__STRING:
            fprintf(stderr, "type=STRING, value=\"%s\"\n", ar_data__get_string(own_message));
            break;
        case AR_DATA_TYPE__MAP:
            fprintf(stderr, "type=MAP\n");
            break;
        case AR_DATA_TYPE__LIST:
            fprintf(stderr, "type=LIST\n");
            break;
        case AR_DATA_TYPE__DOUBLE:
            fprintf(stderr, "type=DOUBLE, value=%f\n", ar_data__get_double(own_message));
            break;
        default:
            fprintf(stderr, "type=UNKNOWN\n");
            break;
    }
    
    if (!mut_agent->own_message_queue) {
        // If agent has no message queue, destroy the message
        // Transfer ownership back from agent before destroying
        ar_data__destroy_if_owned(own_message, mut_agent);
        own_message = NULL; // Mark as destroyed
        return false;
    }
    
    // Take ownership for the agent before adding to queue
    ar_data__take_ownership(own_message, mut_agent);
    
    // Agent module adds to the queue
    bool result = ar_list__add_last(mut_agent->own_message_queue, own_message);
    
    // If we couldn't add to the queue, destroy the message
    if (!result) {
        // Transfer ownership back from agent before destroying
        ar_data__destroy_if_owned(own_message, mut_agent);
    }
    
    return result;
}

/* Accessor functions for opaque type */

int64_t ar_agent__get_id(const ar_agent_t *ref_agent) {
    if (!ref_agent) {
        return 0;
    }
    return ref_agent->id;
}

const ar_data_t* ar_agent__get_memory(const ar_agent_t *ref_agent) {
    if (!ref_agent) {
        return NULL;
    }
    return ref_agent->own_memory;
}

ar_data_t* ar_agent__get_mutable_memory(ar_agent_t *mut_agent) {
    if (!mut_agent) {
        return NULL;
    }
    return mut_agent->own_memory;
}

const ar_data_t* ar_agent__get_context(const ar_agent_t *ref_agent) {
    if (!ref_agent) {
        return NULL;
    }
    
    return ref_agent->ref_context;
}

bool ar_agent__is_active(const ar_agent_t *ref_agent) {
    if (!ref_agent) {
        return false;
    }
    return ref_agent->is_active;
}

bool ar_agent__get_method_info(const ar_agent_t *ref_agent, const char **out_method_name, const char **out_method_version) {
    if (!ref_agent || !ref_agent->ref_method) {
        return false;
    }
    
    if (out_method_name) {
        *out_method_name = ar_method__get_name(ref_agent->ref_method);
    }
    if (out_method_version) {
        *out_method_version = ar_method__get_version(ref_agent->ref_method);
    }
    return true;
}


const ar_method_t* ar_agent__get_method(const ar_agent_t *ref_agent) {
    if (!ref_agent) {
        return NULL;
    }
    
    return ref_agent->ref_method;
}

void ar_agent__set_active(ar_agent_t *mut_agent, bool is_active) {
    if (!mut_agent) {
        return;
    }
    mut_agent->is_active = is_active;
}


bool ar_agent__has_messages(const ar_agent_t *ref_agent) {
    if (!ref_agent || !ref_agent->own_message_queue) {
        return false;
    }
    
    return ar_list__count(ref_agent->own_message_queue) > 0;
}

ar_data_t* ar_agent__get_message(ar_agent_t *mut_agent) {
    if (!mut_agent || !mut_agent->own_message_queue) {
        return NULL;
    }
    
    ar_data_t *own_message = ar_list__remove_first(mut_agent->own_message_queue);
    if (own_message) {
        // Transfer ownership from agent to NULL (making it unowned so system can destroy it)
        ar_data__drop_ownership(own_message, mut_agent);
    }
    return own_message;
}

bool ar_agent__update_method(ar_agent_t *mut_agent, const ar_method_t *ref_new_method) {
    if (!mut_agent || !ref_new_method) {
        return false;
    }
    
    // Update the method
    mut_agent->ref_method = ref_new_method;
    
    return true;
}

void ar_agent__set_id(ar_agent_t *mut_agent, int64_t new_id) {
    if (!mut_agent) {
        return;
    }
    mut_agent->id = new_id;
}


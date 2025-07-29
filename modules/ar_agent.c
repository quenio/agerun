/* Agerun Agent Implementation */
#include "ar_agent.h"
#include "ar_method.h"
#include "ar_methodology.h"
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

/* Static variables for commonly used messages */
static const char g_sleep_message[] = "__sleep__";
static const char g_wake_message[] = "__wake__";

/* Implementation */
ar_agent_t* ar_agent__create(const char *ref_method_name, const char *ref_version, const ar_data_t *ref_context) {
    if (!ref_method_name) {
        return NULL;
    }
    
    // Lookup method using methodology
    const ar_method_t *ref_method = ar_methodology__get_method(ref_method_name, ref_version);
    if (!ref_method) {
        return NULL;  // Method not found
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
    
    // Send wake message
    ar_data_t *own_wake_msg = ar_data__create_string(g_wake_message);
    if (own_wake_msg) {
        // Mark agent as owner of the message
        ar_data__take_ownership(own_wake_msg, own_agent);
        ar_agent__send(own_agent, own_wake_msg);
        // Note: The wake message will be processed when the system runs
    }
    
    return own_agent;
}

void ar_agent__destroy(ar_agent_t *own_agent) {
    if (!own_agent) {
        return;
    }
    
    // Send sleep message before destruction
    ar_data_t *own_sleep_msg = ar_data__create_string(g_sleep_message);
    if (own_sleep_msg) {
        // Mark agent as owner of the message
        ar_data__take_ownership(own_sleep_msg, own_agent);
        bool sent = ar_list__add_last(own_agent->own_message_queue, own_sleep_msg);
        if (!sent) {
            // Transfer ownership back before destroying
            ar_data__destroy_if_owned(own_sleep_msg, own_agent);
        }
        // Note: The sleep message will be processed before the agent is destroyed
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
    
    if (!mut_agent->own_message_queue) {
        // If agent has no message queue, destroy the message
        // Transfer ownership back from agent before destroying
        ar_data__destroy_if_owned(own_message, mut_agent);
        own_message = NULL; // Mark as destroyed
        return false;
    }
    
    // Note: The caller should have already marked the agent as owner
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

bool ar_agent__update_method(ar_agent_t *mut_agent, const ar_method_t *ref_new_method, bool send_sleep_wake) {
    if (!mut_agent || !ref_new_method) {
        return false;
    }
    
    if (send_sleep_wake) {
        // Send sleep message before update
        ar_data_t *own_sleep_msg = ar_data__create_string(g_sleep_message);
        if (own_sleep_msg) {
            // Mark agent as owner of the message
            ar_data__take_ownership(own_sleep_msg, mut_agent);
            if (!ar_list__add_last(mut_agent->own_message_queue, own_sleep_msg)) {
                // Transfer ownership back before destroying
                ar_data__destroy_if_owned(own_sleep_msg, mut_agent);
            }
        }
    }
    
    // Update the method
    mut_agent->ref_method = ref_new_method;
    
    if (send_sleep_wake) {
        // Send wake message after update
        ar_data_t *own_wake_msg = ar_data__create_string(g_wake_message);
        if (own_wake_msg) {
            // Mark agent as owner of the message
            ar_data__take_ownership(own_wake_msg, mut_agent);
            if (!ar_list__add_last(mut_agent->own_message_queue, own_wake_msg)) {
                // Transfer ownership back before destroying
                ar_data__destroy_if_owned(own_wake_msg, mut_agent);
            }
        }
    }
    
    return true;
}

void ar_agent__set_id(ar_agent_t *mut_agent, int64_t new_id) {
    if (!mut_agent) {
        return;
    }
    mut_agent->id = new_id;
}


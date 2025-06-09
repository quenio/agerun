/* Agerun Agent Implementation */
#include "agerun_agent.h"
#include "agerun_method.h"
#include "agerun_methodology.h"
#include "agerun_list.h"
#include "agerun_heap.h"
#include "agerun_map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/* Agent Definition (moved from header for opaque type) */
struct agent_s {
    int64_t id;
    const method_t *ref_method; // Borrowed reference to method
    bool is_active;
    list_t *own_message_queue;  // Using list as a message queue, owned by agent
    data_t *own_memory;        // Memory owned by agent
    const data_t *ref_context;  // Context is read-only reference, not owned
};

/* Constants */

/* Static variables for commonly used messages */
static const char g_sleep_message[] = "__sleep__";
static const char g_wake_message[] = "__wake__";

/* Implementation */
agent_t* ar__agent__create(const char *ref_method_name, const char *ref_version, const data_t *ref_context) {
    if (!ref_method_name) {
        return NULL;
    }
    
    // Lookup method using methodology
    const method_t *ref_method = ar__methodology__get_method(ref_method_name, ref_version);
    if (!ref_method) {
        return NULL;  // Method not found
    }
    
    // Create agent structure
    agent_t *own_agent = AR__HEAP__MALLOC(sizeof(agent_t), "agent");
    if (!own_agent) {
        return NULL;
    }
    
    own_agent->id = 0;  // ID will be set by agency when it registers the agent
    own_agent->is_active = true;
    own_agent->ref_method = ref_method;  // Just store reference
    own_agent->ref_context = ref_context;  // Store reference, we don't own it
    
    own_agent->own_message_queue = ar__list__create();
    if (!own_agent->own_message_queue) {
        AR__HEAP__FREE(own_agent);
        return NULL;
    }
    
    own_agent->own_memory = ar__data__create_map();
    if (!own_agent->own_memory) {
        ar__list__destroy(own_agent->own_message_queue);
        AR__HEAP__FREE(own_agent);
        return NULL;
    }
    
    // Send wake message
    data_t *own_wake_msg = ar__data__create_string(g_wake_message);
    if (own_wake_msg) {
        ar__agent__send(own_agent, own_wake_msg);
        // Note: The wake message will be processed when the system runs
    }
    
    return own_agent;
}

void ar__agent__destroy(agent_t *own_agent) {
    if (!own_agent) {
        return;
    }
    
    // Send sleep message before destruction
    data_t *own_sleep_msg = ar__data__create_string(g_sleep_message);
    if (own_sleep_msg) {
        bool sent = ar__list__add_last(own_agent->own_message_queue, own_sleep_msg);
        if (!sent) {
            ar__data__destroy(own_sleep_msg);
        }
        // Note: The sleep message will be processed before the agent is destroyed
    }
    
    // Destroy memory if owned
    if (own_agent->own_memory) {
        ar__data__destroy(own_agent->own_memory);
        own_agent->own_memory = NULL;
    }
    
    // We don't own the context, just clear the reference
    own_agent->ref_context = NULL;
    
    // Destroy any pending messages and the queue
    if (own_agent->own_message_queue) {
        // First, destroy any remaining messages in the queue
        data_t *own_msg = NULL;
        while ((own_msg = ar__list__remove_first(own_agent->own_message_queue)) != NULL) {
            ar__data__destroy(own_msg);
            own_msg = NULL; // Mark as destroyed
        }
        // Then destroy the queue itself
        ar__list__destroy(own_agent->own_message_queue);
        own_agent->own_message_queue = NULL; // Mark as destroyed
    }
    
    // Free the agent structure
    AR__HEAP__FREE(own_agent);
}

bool ar__agent__send(agent_t *mut_agent, data_t *own_message) {
    if (!mut_agent || !own_message) {
        // Destroy the message if we have one but no agent
        if (own_message) {
            ar__data__destroy(own_message);
        }
        return false;
    }
    
    if (!mut_agent->own_message_queue) {
        // If agent has no message queue, destroy the message
        ar__data__destroy(own_message);
        own_message = NULL; // Mark as destroyed
        return false;
    }
    
    // Agent module takes ownership of the message and adds to the queue
    bool result = ar__list__add_last(mut_agent->own_message_queue, own_message);
    
    // If we couldn't add to the queue, destroy the message
    if (!result) {
        ar__data__destroy(own_message);
    }
    
    return result;
}

/* Accessor functions for opaque type */

int64_t ar__agent__get_id(const agent_t *ref_agent) {
    if (!ref_agent) {
        return 0;
    }
    return ref_agent->id;
}

const data_t* ar__agent__get_memory(const agent_t *ref_agent) {
    if (!ref_agent) {
        return NULL;
    }
    return ref_agent->own_memory;
}

data_t* ar__agent__get_mutable_memory(agent_t *mut_agent) {
    if (!mut_agent) {
        return NULL;
    }
    return mut_agent->own_memory;
}

const data_t* ar__agent__get_context(const agent_t *ref_agent) {
    if (!ref_agent) {
        return NULL;
    }
    
    return ref_agent->ref_context;
}

bool ar__agent__is_active(const agent_t *ref_agent) {
    if (!ref_agent) {
        return false;
    }
    return ref_agent->is_active;
}

bool ar__agent__get_method_info(const agent_t *ref_agent, const char **out_method_name, const char **out_method_version) {
    if (!ref_agent || !ref_agent->ref_method) {
        return false;
    }
    
    if (out_method_name) {
        *out_method_name = ar__method__get_name(ref_agent->ref_method);
    }
    if (out_method_version) {
        *out_method_version = ar__method__get_version(ref_agent->ref_method);
    }
    return true;
}


const method_t* ar__agent__get_method(const agent_t *ref_agent) {
    if (!ref_agent) {
        return NULL;
    }
    
    return ref_agent->ref_method;
}

void ar__agent__set_active(agent_t *mut_agent, bool is_active) {
    if (!mut_agent) {
        return;
    }
    mut_agent->is_active = is_active;
}


bool ar__agent__has_messages(const agent_t *ref_agent) {
    if (!ref_agent || !ref_agent->own_message_queue) {
        return false;
    }
    
    return ar__list__count(ref_agent->own_message_queue) > 0;
}

data_t* ar__agent__get_message(agent_t *mut_agent) {
    if (!mut_agent || !mut_agent->own_message_queue) {
        return NULL;
    }
    
    return ar__list__remove_first(mut_agent->own_message_queue);
}

bool ar__agent__update_method(agent_t *mut_agent, const method_t *ref_new_method, bool send_sleep_wake) {
    if (!mut_agent || !ref_new_method) {
        return false;
    }
    
    if (send_sleep_wake) {
        // Send sleep message before update
        data_t *own_sleep_msg = ar__data__create_string(g_sleep_message);
        if (own_sleep_msg) {
            ar__list__add_last(mut_agent->own_message_queue, own_sleep_msg);
        }
    }
    
    // Update the method
    mut_agent->ref_method = ref_new_method;
    
    if (send_sleep_wake) {
        // Send wake message after update
        data_t *own_wake_msg = ar__data__create_string(g_wake_message);
        if (own_wake_msg) {
            ar__list__add_last(mut_agent->own_message_queue, own_wake_msg);
        }
    }
    
    return true;
}

void ar__agent__set_id(agent_t *mut_agent, int64_t new_id) {
    if (!mut_agent) {
        return;
    }
    mut_agent->id = new_id;
}


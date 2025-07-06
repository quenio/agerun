/* Agerun Agency Implementation - Refactored to use new modules */
#include "ar_agency.h"
#include "ar_agent.h"
#include "ar_agent_registry.h"
#include "ar_agent_store.h"
#include "ar_agent_update.h"
#include "ar_heap.h"
#include <stdio.h>

/* Global State */
static bool g_is_initialized = false;
static ar_agent_registry_t *g_own_registry = NULL; /* Owned by the agency module */

/* Implementation */
void ar_agency__set_initialized(bool initialized) {
    g_is_initialized = initialized;
    
    if (initialized && !g_own_registry) {
        /* Create the registry when agency is initialized */
        g_own_registry = ar_agent_registry__create();
        if (!g_own_registry) {
            printf("Error: Failed to create agent registry\n");
            g_is_initialized = false;
            return;
        }
    } else if (!initialized && g_own_registry) {
        /* Destroy the registry when agency is shutdown */
        ar_agent_registry__destroy(g_own_registry);
        g_own_registry = NULL;
    }
}

void ar_agency__reset(void) {
    if (!g_is_initialized || !g_own_registry) {
        return;
    }
    
    // Destroy all agents
    int64_t agent_id = ar_agent_registry__get_first(g_own_registry);
    while (agent_id != 0) {
        int64_t next_id = ar_agent_registry__get_next(g_own_registry, agent_id);
        ar_agency__destroy_agent(agent_id);
        agent_id = next_id;
    }
    
    // Clear the registry
    ar_agent_registry__clear(g_own_registry);
}

int ar_agency__count_agents(void) {
    if (!g_is_initialized || !g_own_registry) {
        return 0;
    }
    return ar_agent_registry__count(g_own_registry);
}

bool ar_agency__save_agents(void) {
    if (!g_is_initialized) {
        return false;
    }
    return ar_agent_store__save();
}

bool ar_agency__load_agents(void) {
    if (!g_is_initialized) {
        return false;
    }
    return ar_agent_store__load();
}

int ar_agency__update_agent_methods(const ar_method_t *ref_old_method, const ar_method_t *ref_new_method, bool send_lifecycle_events) {
    if (!g_is_initialized || !g_own_registry) {
        return 0;
    }
    
    // Delegate to agent_update module, passing our registry
    return ar_agent_update__update_methods(g_own_registry, ref_old_method, ref_new_method, send_lifecycle_events);
}

int ar_agency__count_agents_using_method(const ar_method_t *ref_method) {
    if (!g_is_initialized || !g_own_registry) {
        return 0;
    }
    
    // Delegate to agent_update module, passing our registry
    return ar_agent_update__count_using_method(g_own_registry, ref_method);
}

int64_t ar_agency__get_first_agent(void) {
    if (!g_is_initialized || !g_own_registry) {
        return 0;
    }
    return ar_agent_registry__get_first(g_own_registry);
}

int64_t ar_agency__get_next_agent(int64_t current_id) {
    if (!g_is_initialized || !g_own_registry) {
        return 0;
    }
    return ar_agent_registry__get_next(g_own_registry, current_id);
}

bool ar_agency__agent_has_messages(int64_t agent_id) {
    if (!g_is_initialized || !g_own_registry) {
        return false;
    }
    
    ar_agent_t *ref_agent = (ar_agent_t*)ar_agent_registry__find_agent(g_own_registry, agent_id);
    if (!ref_agent) {
        return false;
    }
    
    return ar_agent__has_messages(ref_agent);
}

ar_data_t* ar_agency__get_agent_message(int64_t agent_id) {
    if (!g_is_initialized || !g_own_registry) {
        return NULL;
    }
    
    ar_agent_t *mut_agent = (ar_agent_t*)ar_agent_registry__find_agent(g_own_registry, agent_id);
    if (!mut_agent) {
        return NULL;
    }
    
    return ar_agent__get_message(mut_agent);
}

int64_t ar_agency__create_agent(const char *ref_method_name, const char *ref_version, const ar_data_t *ref_context) {
    if (!g_is_initialized || !g_own_registry) {
        return 0;
    }
    
    // Create the agent using the agent module
    ar_agent_t *own_agent = ar_agent__create(ref_method_name, ref_version, ref_context);
    if (!own_agent) {
        return 0;
    }
    
    // Allocate an ID for the agent
    int64_t agent_id = ar_agent_registry__allocate_id(g_own_registry);
    if (agent_id == 0) {
        ar_agent__destroy(own_agent);
        return 0;
    }
    
    // Set the agent's ID
    ar_agent__set_id(own_agent, agent_id);
    
    // Register the ID in the registry
    if (!ar_agent_registry__register_id(g_own_registry, agent_id)) {
        ar_agent__destroy(own_agent);
        return 0;
    }
    
    // Track the agent in the registry
    if (!ar_agent_registry__track_agent(g_own_registry, agent_id, own_agent)) {
        ar_agent_registry__unregister_id(g_own_registry, agent_id);
        ar_agent__destroy(own_agent);
        return 0;
    }
    
    return agent_id;
}

bool ar_agency__destroy_agent(int64_t agent_id) {
    if (!g_is_initialized || !g_own_registry) {
        return false;
    }
    
    // Find the agent before unregistering
    ar_agent_t *own_agent = (ar_agent_t*)ar_agent_registry__find_agent(g_own_registry, agent_id);
    if (!own_agent) {
        return false;
    }
    
    // Unregister the ID (this also untracks the agent)
    ar_agent_registry__unregister_id(g_own_registry, agent_id);
    
    // Destroy the agent
    ar_agent__destroy(own_agent);
    
    return true;
}

bool ar_agency__send_to_agent(int64_t agent_id, ar_data_t *own_message) {
    if (!g_is_initialized || !g_own_registry) {
        if (own_message) {
            ar_data__destroy(own_message);
        }
        return false;
    }
    
    ar_agent_t *mut_agent = (ar_agent_t*)ar_agent_registry__find_agent(g_own_registry, agent_id);
    if (!mut_agent) {
        if (own_message) {
            ar_data__destroy(own_message);
        }
        return false;
    }
    
    return ar_agent__send(mut_agent, own_message);
}

bool ar_agency__agent_exists(int64_t agent_id) {
    if (!g_is_initialized || !g_own_registry) {
        return false;
    }
    
    return ar_agent_registry__is_registered(g_own_registry, agent_id);
}

const ar_data_t* ar_agency__get_agent_memory(int64_t agent_id) {
    if (!g_is_initialized || !g_own_registry) {
        return NULL;
    }
    
    ar_agent_t *ref_agent = (ar_agent_t*)ar_agent_registry__find_agent(g_own_registry, agent_id);
    if (!ref_agent) {
        return NULL;
    }
    
    return ar_agent__get_memory(ref_agent);
}

const ar_data_t* ar_agency__get_agent_context(int64_t agent_id) {
    if (!g_is_initialized || !g_own_registry) {
        return NULL;
    }
    
    ar_agent_t *ref_agent = (ar_agent_t*)ar_agent_registry__find_agent(g_own_registry, agent_id);
    if (!ref_agent) {
        return NULL;
    }
    
    return ar_agent__get_context(ref_agent);
}

bool ar_agency__is_agent_active(int64_t agent_id) {
    if (!g_is_initialized || !g_own_registry) {
        return false;
    }
    
    ar_agent_t *ref_agent = (ar_agent_t*)ar_agent_registry__find_agent(g_own_registry, agent_id);
    if (!ref_agent) {
        return false;
    }
    
    return ar_agent__is_active(ref_agent);
}

const ar_method_t* ar_agency__get_agent_method(int64_t agent_id) {
    if (!g_is_initialized || !g_own_registry) {
        return NULL;
    }
    
    ar_agent_t *ref_agent = (ar_agent_t*)ar_agent_registry__find_agent(g_own_registry, agent_id);
    if (!ref_agent) {
        return NULL;
    }
    
    return ar_agent__get_method(ref_agent);
}

bool ar_agency__get_agent_method_info(int64_t agent_id, const char **out_method_name, const char **out_method_version) {
    if (!g_is_initialized || !g_own_registry) {
        return false;
    }
    
    ar_agent_t *ref_agent = (ar_agent_t*)ar_agent_registry__find_agent(g_own_registry, agent_id);
    if (!ref_agent) {
        return false;
    }
    
    return ar_agent__get_method_info(ref_agent, out_method_name, out_method_version);
}

ar_data_t* ar_agency__get_agent_mutable_memory(int64_t agent_id) {
    if (!g_is_initialized || !g_own_registry) {
        return NULL;
    }
    
    ar_agent_t *mut_agent = (ar_agent_t*)ar_agent_registry__find_agent(g_own_registry, agent_id);
    if (!mut_agent) {
        return NULL;
    }
    
    return ar_agent__get_mutable_memory(mut_agent);
}

bool ar_agency__update_agent_method(int64_t agent_id, const ar_method_t *ref_new_method, bool send_sleep_wake) {
    if (!g_is_initialized || !g_own_registry) {
        return false;
    }
    
    ar_agent_t *mut_agent = (ar_agent_t*)ar_agent_registry__find_agent(g_own_registry, agent_id);
    if (!mut_agent) {
        return false;
    }
    
    return ar_agent__update_method(mut_agent, ref_new_method, send_sleep_wake);
}

bool ar_agency__set_agent_active(int64_t agent_id, bool is_active) {
    if (!g_is_initialized || !g_own_registry) {
        return false;
    }
    
    ar_agent_t *mut_agent = (ar_agent_t*)ar_agent_registry__find_agent(g_own_registry, agent_id);
    if (!mut_agent) {
        return false;
    }
    
    ar_agent__set_active(mut_agent, is_active);
    return true;
}

int ar_agency__count_active_agents(void) {
    return ar_agency__count_agents();
}

bool ar_agency__set_agent_id(int64_t old_id, int64_t new_id) {
    if (!g_is_initialized || !g_own_registry) {
        return false;
    }
    
    ar_agent_t *mut_agent = (ar_agent_t*)ar_agent_registry__find_agent(g_own_registry, old_id);
    if (!mut_agent) {
        return false;
    }
    
    // Update the agent's ID
    ar_agent__set_id(mut_agent, new_id);
    
    // Update in registry: untrack from old ID, register new ID, track with new ID
    ar_agent_registry__untrack_agent(g_own_registry, old_id);
    ar_agent_registry__unregister_id(g_own_registry, old_id);
    
    if (!ar_agent_registry__register_id(g_own_registry, new_id)) {
        // Restore old ID if registration fails
        ar_agent__set_id(mut_agent, old_id);
        ar_agent_registry__register_id(g_own_registry, old_id);
        ar_agent_registry__track_agent(g_own_registry, old_id, mut_agent);
        return false;
    }
    
    if (!ar_agent_registry__track_agent(g_own_registry, new_id, mut_agent)) {
        // Restore old ID if tracking fails
        ar_agent__set_id(mut_agent, old_id);
        ar_agent_registry__unregister_id(g_own_registry, new_id);
        ar_agent_registry__register_id(g_own_registry, old_id);
        ar_agent_registry__track_agent(g_own_registry, old_id, mut_agent);
        return false;
    }
    
    return true;
}

ar_agent_registry_t* ar_agency__get_registry(void) {
    return g_own_registry;
}

/* End of implementation */

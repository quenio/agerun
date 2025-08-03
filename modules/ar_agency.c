/* Agerun Agency Implementation - Refactored to use new modules */
#include "ar_agency.h"
#include "ar_agent.h"
#include "ar_agent_registry.h"
#include "ar_agent_store.h"
#include "ar_agent_update.h"
#include "ar_methodology.h"
#include "ar_method.h"
#include "ar_log.h"
#include "ar_io.h"
#include "ar_heap.h"
#include <stdio.h>

/* Agency structure */
struct ar_agency_s {
    bool is_initialized;
    ar_log_t *ref_log;                      /* Borrowed reference from system */
    ar_agent_registry_t *own_registry;      /* Owned by the agency */
    ar_methodology_t *own_methodology;      /* Owned by the agency */
    ar_agent_store_t *own_agent_store;      /* Owned by the agency */
};

/* Global State */
static bool g_is_initialized = false;
static ar_agent_registry_t *g_own_registry = NULL; /* Owned by the agency module */
static ar_agency_t *g_default_agency = NULL; /* Global instance for backward compatibility */

/* Helper to get global instance */
static ar_agency_t* _get_global_instance(void) {
    if (!g_default_agency && g_is_initialized && g_own_registry) {
        // Create wrapper agency instance using existing global registry
        g_default_agency = AR__HEAP__MALLOC(sizeof(ar_agency_t), "agency global instance");
        if (g_default_agency) {
            g_default_agency->is_initialized = true;
            g_default_agency->ref_log = NULL; // No log for global instance
            g_default_agency->own_registry = g_own_registry;
            g_default_agency->own_methodology = NULL; // Will use global methodology for now
        }
    }
    return g_default_agency;
}

/* Instance-based implementation */
ar_agency_t* ar_agency__create(ar_log_t *ref_log) {
    ar_agency_t *own_agency = AR__HEAP__MALLOC(sizeof(ar_agency_t), "agency");
    if (!own_agency) {
        printf("Error: Failed to allocate memory for agency\n");
        return NULL;
    }
    
    own_agency->is_initialized = true;
    own_agency->ref_log = ref_log;  // Store log reference
    
    // Create owned methodology with provided log
    own_agency->own_methodology = ar_methodology__create(ref_log);
    if (!own_agency->own_methodology) {
        if (ref_log) {
            ar_log__error(ref_log, "Agency: Failed to create methodology");
        }
        AR__HEAP__FREE(own_agency);
        return NULL;
    }
    
    own_agency->own_registry = ar_agent_registry__create();
    if (!own_agency->own_registry) {
        if (ref_log) {
            ar_log__error(ref_log, "Agency: Failed to create agent registry");
        }
        ar_methodology__destroy(own_agency->own_methodology);
        AR__HEAP__FREE(own_agency);
        return NULL;
    }
    
    // Create owned agent store with the registry
    own_agency->own_agent_store = ar_agent_store__create(own_agency->own_registry);
    if (!own_agency->own_agent_store) {
        if (ref_log) {
            ar_log__error(ref_log, "Agency: Failed to create agent store");
        }
        ar_agent_registry__destroy(own_agency->own_registry);
        ar_methodology__destroy(own_agency->own_methodology);
        AR__HEAP__FREE(own_agency);
        return NULL;
    }
    
    return own_agency;
}

void ar_agency__destroy(ar_agency_t *own_agency) {
    if (!own_agency) {
        return;
    }
    
    // Destroy all agents
    if (own_agency->own_registry) {
        int64_t agent_id = ar_agent_registry__get_first(own_agency->own_registry);
        while (agent_id != 0) {
            int64_t next_id = ar_agent_registry__get_next(own_agency->own_registry, agent_id);
            
            // Find and destroy the agent
            ar_agent_t *own_agent = (ar_agent_t*)ar_agent_registry__find_agent(own_agency->own_registry, agent_id);
            if (own_agent) {
                ar_agent_registry__unregister_id(own_agency->own_registry, agent_id);
                ar_agent__destroy(own_agent);
            }
            
            agent_id = next_id;
        }
        
        ar_agent_registry__destroy(own_agency->own_registry);
    }
    
    // Destroy the owned agent store
    if (own_agency->own_agent_store) {
        ar_agent_store__destroy(own_agency->own_agent_store);
    }
    
    // Destroy the owned methodology
    if (own_agency->own_methodology) {
        ar_methodology__destroy(own_agency->own_methodology);
    }
    
    AR__HEAP__FREE(own_agency);
}

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
        /* Clean up global instance first */
        if (g_default_agency) {
            // Don't destroy the registry through the instance since it's shared
            g_default_agency->own_registry = NULL;
            AR__HEAP__FREE(g_default_agency);
            g_default_agency = NULL;
        }
        
        /* Destroy the registry when agency is shutdown */
        ar_agent_registry__destroy(g_own_registry);
        g_own_registry = NULL;
    }
}

void ar_agency__reset(void) {
    ar_agency_t *ref_agency = _get_global_instance();
    if (ref_agency) {
        ar_agency__reset_with_instance(ref_agency);
    }
}

int ar_agency__count_agents(void) {
    ar_agency_t *ref_agency = _get_global_instance();
    return ref_agency ? ar_agency__count_agents_with_instance(ref_agency) : 0;
}

bool ar_agency__save_agents(void) {
    ar_agency_t *ref_agency = _get_global_instance();
    return ref_agency ? ar_agency__save_agents_with_instance(ref_agency, NULL) : false;
}

bool ar_agency__load_agents(void) {
    ar_agency_t *mut_agency = _get_global_instance();
    return mut_agency ? ar_agency__load_agents_with_instance(mut_agency, NULL) : false;
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
    ar_agency_t *mut_agency = _get_global_instance();
    return mut_agency ? ar_agency__create_agent_with_instance(mut_agency, ref_method_name, ref_version, ref_context) : 0;
}

bool ar_agency__destroy_agent(int64_t agent_id) {
    ar_agency_t *mut_agency = _get_global_instance();
    return mut_agency ? ar_agency__destroy_agent_with_instance(mut_agency, agent_id) : false;
}

bool ar_agency__send_to_agent(int64_t agent_id, ar_data_t *own_message) {
    ar_agency_t *mut_agency = _get_global_instance();
    return mut_agency ? ar_agency__send_to_agent_with_instance(mut_agency, agent_id, own_message) : false;
}

bool ar_agency__agent_exists(int64_t agent_id) {
    ar_agency_t *ref_agency = _get_global_instance();
    return ref_agency ? ar_agency__agent_exists_with_instance(ref_agency, agent_id) : false;
}

const ar_data_t* ar_agency__get_agent_memory(int64_t agent_id) {
    ar_agency_t *ref_agency = _get_global_instance();
    return ref_agency ? ar_agency__get_agent_memory_with_instance(ref_agency, agent_id) : NULL;
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

/* Instance-based API implementation */

int ar_agency__count_agents_with_instance(ar_agency_t *ref_agency) {
    if (!ref_agency || !ref_agency->is_initialized || !ref_agency->own_registry) {
        return 0;
    }
    return ar_agent_registry__count(ref_agency->own_registry);
}

int ar_agency__count_agents_using_method_with_instance(ar_agency_t *ref_agency, const ar_method_t *ref_method) {
    if (!ref_agency || !ref_agency->is_initialized || !ref_agency->own_registry) {
        return 0;
    }
    
    // Delegate to agent_update module, passing the agency's registry
    return ar_agent_update__count_using_method(ref_agency->own_registry, ref_method);
}

int ar_agency__update_agent_methods_with_instance(ar_agency_t *mut_agency, const ar_method_t *ref_old_method, const ar_method_t *ref_new_method, bool send_lifecycle_events) {
    if (!mut_agency || !mut_agency->is_initialized || !mut_agency->own_registry) {
        return 0;
    }
    
    // Delegate to agent_update module, passing the agency's registry
    return ar_agent_update__update_methods(mut_agency->own_registry, ref_old_method, ref_new_method, send_lifecycle_events);
}

int64_t ar_agency__create_agent_with_instance(ar_agency_t *mut_agency, 
                                               const char *ref_method_name, 
                                               const char *ref_version, 
                                               const ar_data_t *ref_context) {
    if (!mut_agency || !mut_agency->is_initialized || !mut_agency->own_registry) {
        return 0;
    }
    
    // Look up the method using our methodology
    const ar_method_t *ref_method = NULL;
    if (mut_agency->own_methodology) {
        // Debug: Log that we're looking up in instance methodology
        if (mut_agency->ref_log) {
            char debug_msg[256];
            snprintf(debug_msg, sizeof(debug_msg), 
                    "Agency: Looking up method '%s' version '%s' in instance methodology", 
                    ref_method_name, ref_version ? ref_version : "latest");
            ar_log__info(mut_agency->ref_log, debug_msg);
        }
        
        ref_method = ar_methodology__get_method_with_instance(mut_agency->own_methodology, ref_method_name, ref_version);
        if (!ref_method) {
            if (mut_agency->ref_log) {
                char error_msg[256];
                snprintf(error_msg, sizeof(error_msg), 
                        "Agency: Method '%s' version '%s' not found in methodology", 
                        ref_method_name, ref_version ? ref_version : "latest");
                ar_log__error(mut_agency->ref_log, error_msg);
            }
            return 0;
        }
    } else {
        // Fallback to global methodology for backward compatibility
        ref_method = ar_methodology__get_method(ref_method_name, ref_version);
        if (!ref_method) {
            printf("Agency: Method '%s' version '%s' not found in global methodology\n", 
                   ref_method_name, ref_version ? ref_version : "latest");
            return 0;
        }
    }
    
    // Create the agent using the method reference we found
    ar_agent_t *own_agent = ar_agent__create_with_method(ref_method, ref_context);
    if (!own_agent) {
        if (mut_agency->ref_log) {
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), 
                    "Agency: Failed to create agent for method '%s'", ref_method_name);
            ar_log__error(mut_agency->ref_log, error_msg);
        }
        return 0;
    }
    
    
    // Allocate an ID for the agent
    int64_t agent_id = ar_agent_registry__allocate_id(mut_agency->own_registry);
    if (agent_id == 0) {
        ar_agent__destroy(own_agent);
        return 0;
    }
    
    // Set the agent's ID
    ar_agent__set_id(own_agent, agent_id);
    
    // Register the ID in the registry
    if (!ar_agent_registry__register_id(mut_agency->own_registry, agent_id)) {
        ar_agent__destroy(own_agent);
        return 0;
    }
    
    // Track the agent in the registry
    if (!ar_agent_registry__track_agent(mut_agency->own_registry, agent_id, own_agent)) {
        ar_agent_registry__unregister_id(mut_agency->own_registry, agent_id);
        ar_agent__destroy(own_agent);
        return 0;
    }
    
    return agent_id;
}

bool ar_agency__destroy_agent_with_instance(ar_agency_t *mut_agency, int64_t agent_id) {
    if (!mut_agency || !mut_agency->is_initialized || !mut_agency->own_registry) {
        return false;
    }
    
    // Find the agent before unregistering
    ar_agent_t *own_agent = (ar_agent_t*)ar_agent_registry__find_agent(mut_agency->own_registry, agent_id);
    if (!own_agent) {
        return false;
    }
    
    // Unregister the ID (this also untracks the agent)
    ar_agent_registry__unregister_id(mut_agency->own_registry, agent_id);
    
    // Destroy the agent
    ar_agent__destroy(own_agent);
    
    return true;
}

bool ar_agency__send_to_agent_with_instance(ar_agency_t *mut_agency, 
                                             int64_t agent_id, 
                                             ar_data_t *own_message) {
    if (!mut_agency || !mut_agency->is_initialized || !mut_agency->own_registry) {
        if (own_message) {
            ar_data__destroy(own_message);
        }
        return false;
    }
    
    // DEBUG: Log every message being sent
    if (own_message) {
        fprintf(stderr, "DEBUG [SEND]: Sending message to agent %lld, ", (long long)agent_id);
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
    }
    
    ar_agent_t *mut_agent = (ar_agent_t*)ar_agent_registry__find_agent(mut_agency->own_registry, agent_id);
    if (!mut_agent) {
        if (own_message) {
            ar_data__destroy(own_message);
        }
        return false;
    }
    
    return ar_agent__send(mut_agent, own_message);
}

const ar_data_t* ar_agency__get_agent_memory_with_instance(ar_agency_t *ref_agency, 
                                                            int64_t agent_id) {
    if (!ref_agency || !ref_agency->is_initialized || !ref_agency->own_registry) {
        return NULL;
    }
    
    ar_agent_t *ref_agent = (ar_agent_t*)ar_agent_registry__find_agent(ref_agency->own_registry, agent_id);
    if (!ref_agent) {
        return NULL;
    }
    
    return ar_agent__get_memory(ref_agent);
}

void ar_agency__reset_with_instance(ar_agency_t *mut_agency) {
    if (!mut_agency || !mut_agency->is_initialized || !mut_agency->own_registry) {
        return;
    }
    
    // Destroy all agents
    int64_t agent_id = ar_agent_registry__get_first(mut_agency->own_registry);
    while (agent_id != 0) {
        int64_t next_id = ar_agent_registry__get_next(mut_agency->own_registry, agent_id);
        ar_agency__destroy_agent_with_instance(mut_agency, agent_id);
        agent_id = next_id;
    }
    
    // Clear the registry
    ar_agent_registry__clear(mut_agency->own_registry);
}

bool ar_agency__save_agents_with_instance(ar_agency_t *ref_agency, const char *ref_filename) {
    if (!ref_agency || !ref_agency->is_initialized) {
        return false;
    }
    
    // Use the agency's own agent store instance
    // In future, this could support custom filenames
    (void)ref_filename; // Unused for now
    return ar_agent_store__save(ref_agency->own_agent_store);
}

bool ar_agency__load_agents_with_instance(ar_agency_t *mut_agency, const char *ref_filename) {
    if (!mut_agency || !mut_agency->is_initialized) {
        return false;
    }
    
    // Use the agency's own agent store instance
    // In future, this could support custom filenames
    (void)ref_filename; // Unused for now
    return ar_agent_store__load(mut_agency->own_agent_store);
}

ar_agent_registry_t* ar_agency__get_registry_with_instance(ar_agency_t *ref_agency) {
    if (!ref_agency || !ref_agency->is_initialized) {
        return NULL;
    }
    return ref_agency->own_registry;
}

ar_methodology_t* ar_agency__get_methodology(ar_agency_t *ref_agency) {
    if (!ref_agency || !ref_agency->is_initialized) {
        return NULL;
    }
    return ref_agency->own_methodology;
}

int64_t ar_agency__get_first_agent_with_instance(ar_agency_t *ref_agency) {
    if (!ref_agency || !ref_agency->is_initialized || !ref_agency->own_registry) {
        return 0;
    }
    return ar_agent_registry__get_first(ref_agency->own_registry);
}

int64_t ar_agency__get_next_agent_with_instance(ar_agency_t *ref_agency, int64_t current_id) {
    if (!ref_agency || !ref_agency->is_initialized || !ref_agency->own_registry) {
        return 0;
    }
    return ar_agent_registry__get_next(ref_agency->own_registry, current_id);
}

bool ar_agency__agent_has_messages_with_instance(ar_agency_t *ref_agency, int64_t agent_id) {
    if (!ref_agency || !ref_agency->is_initialized || !ref_agency->own_registry) {
        return false;
    }
    
    ar_agent_t *ref_agent = (ar_agent_t*)ar_agent_registry__find_agent(ref_agency->own_registry, agent_id);
    if (!ref_agent) {
        return false;
    }
    
    return ar_agent__has_messages(ref_agent);
}

ar_data_t* ar_agency__get_agent_message_with_instance(ar_agency_t *mut_agency, int64_t agent_id) {
    if (!mut_agency || !mut_agency->is_initialized || !mut_agency->own_registry) {
        return NULL;
    }
    
    ar_agent_t *mut_agent = (ar_agent_t*)ar_agent_registry__find_agent(mut_agency->own_registry, agent_id);
    if (!mut_agent) {
        return NULL;
    }
    
    return ar_agent__get_message(mut_agent);
}

const ar_method_t* ar_agency__get_agent_method_with_instance(ar_agency_t *ref_agency, int64_t agent_id) {
    if (!ref_agency || !ref_agency->is_initialized || !ref_agency->own_registry) {
        return NULL;
    }
    
    ar_agent_t *ref_agent = (ar_agent_t*)ar_agent_registry__find_agent(ref_agency->own_registry, agent_id);
    if (!ref_agent) {
        return NULL;
    }
    
    return ar_agent__get_method(ref_agent);
}

ar_data_t* ar_agency__get_agent_mutable_memory_with_instance(ar_agency_t *mut_agency, int64_t agent_id) {
    if (!mut_agency || !mut_agency->is_initialized || !mut_agency->own_registry) {
        return NULL;
    }
    
    ar_agent_t *mut_agent = (ar_agent_t*)ar_agent_registry__find_agent(mut_agency->own_registry, agent_id);
    if (!mut_agent) {
        return NULL;
    }
    
    return ar_agent__get_mutable_memory(mut_agent);
}

const ar_data_t* ar_agency__get_agent_context_with_instance(ar_agency_t *ref_agency, int64_t agent_id) {
    if (!ref_agency || !ref_agency->is_initialized || !ref_agency->own_registry) {
        return NULL;
    }
    
    ar_agent_t *ref_agent = (ar_agent_t*)ar_agent_registry__find_agent(ref_agency->own_registry, agent_id);
    if (!ref_agent) {
        return NULL;
    }
    
    return ar_agent__get_context(ref_agent);
}

bool ar_agency__agent_exists_with_instance(ar_agency_t *ref_agency, int64_t agent_id) {
    if (!ref_agency || !ref_agency->is_initialized || !ref_agency->own_registry) {
        return false;
    }
    
    return ar_agent_registry__is_registered(ref_agency->own_registry, agent_id);
}

/* End of implementation */

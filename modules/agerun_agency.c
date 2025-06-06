/* Agerun Agency Implementation - Refactored to use new modules */
#include "agerun_agency.h"
#include "agerun_agent.h"
#include "agerun_agent_store.h"
#include "agerun_agent_update.h"
#include "agerun_heap.h"

/* Global State */
static bool g_is_initialized = false;

/* Implementation */
void ar_agency_set_initialized(bool initialized) {
    g_is_initialized = initialized;
}

void ar_agency_reset(void) {
    ar_agent_reset_all();
}

int ar_agency_count_agents(void) {
    return ar_agent_count_active();
}

bool ar_agency_save_agents(void) {
    if (!g_is_initialized) {
        return false;
    }
    return ar_agent_store_save();
}

bool ar_agency_load_agents(void) {
    if (!g_is_initialized) {
        return false;
    }
    return ar_agent_store_load();
}

int ar_agency_update_agent_methods(const method_t *ref_old_method, const method_t *ref_new_method) {
    if (!ref_old_method || !ref_new_method) {
        return 0;
    }
    
    // Use the agent update module with lifecycle events enabled
    // This matches the original behavior documented in the header
    return ar_agent_update_methods(ref_old_method, ref_new_method, true);
}

int ar_agency_count_agents_using_method(const method_t *ref_method) {
    return ar_agent_update_count_using_method(ref_method);
}

int64_t ar_agency_get_first_agent(void) {
    return ar_agent_get_first_active();
}

int64_t ar_agency_get_next_agent(int64_t current_id) {
    return ar_agent_get_next_active(current_id);
}

bool ar_agency_agent_has_messages(int64_t agent_id) {
    // The registry module doesn't handle messages, so we use the agent module directly
    // This is fine because message handling is not a registry concern
    return ar_agent_has_messages(agent_id);
}

data_t* ar_agency_get_agent_message(int64_t agent_id) {
    // The registry module doesn't handle messages, so we use the agent module directly
    // This is fine because message handling is not a registry concern
    return ar_agent_get_message(agent_id);
}

/* End of implementation */

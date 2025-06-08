/**
 * @file agerun_agent_update.c
 * @brief Implementation of the agent update module
 */

#include "agerun_agent_update.h"
#include "agerun_agent.h"
#include "agerun_method.h"
#include "agerun_semver.h"
#include "agerun_io.h"
#include "agerun_heap.h"
#include <stdio.h>
#include <string.h>

/* Update agents using a specific method to use a different method */
int ar__agent_update__update_methods(agent_registry_t *ref_registry,
                                  const method_t *ref_old_method, 
                                  const method_t *ref_new_method,
                                  bool send_lifecycle_events) {
    if (!ref_registry || !ref_old_method || !ref_new_method) {
        return 0;
    }
    
    // Verify that the methods are compatible
    if (!ar__agent_update__are_compatible(ref_old_method, ref_new_method)) {
        ar_io_warning("Cannot update agents to incompatible method version");
        return 0;
    }
    
    // Get version strings for logging
    const char *method_name = ar_method_get_name(ref_old_method);
    const char *old_version = ar_method_get_version(ref_old_method);
    const char *new_version = ar_method_get_version(ref_new_method);
    
    ar_io_info("Updating agents from method %s version %s to version %s",
               method_name, old_version, new_version);
    
    // Update all agents that use the old method
    int count = 0;
    int64_t agent_id = ar_agent_registry_get_first(ref_registry);
    while (agent_id != 0) {
        agent_t *mut_agent = (agent_t*)ar_agent_registry_find_agent(ref_registry, agent_id);
        if (mut_agent && ar_agent_get_method(mut_agent) == ref_old_method) {
            if (ar_agent_update_method(mut_agent, ref_new_method, send_lifecycle_events)) {
                count++;
            }
        }
        agent_id = ar_agent_registry_get_next(ref_registry, agent_id);
    }
    
    if (count > 0) {
        ar_io_info("Updated %d agents to new method version", count);
        if (send_lifecycle_events) {
            ar_io_info("Queued %d sleep and %d wake messages", count, count);
        }
    }
    
    return count;
}

/* Count the number of agents using a specific method */
int ar__agent_update__count_using_method(agent_registry_t *ref_registry,
                                      const method_t *ref_method) {
    if (!ref_registry || !ref_method) {
        return 0;
    }
    
    // Count agents using this method
    int count = 0;
    int64_t agent_id = ar_agent_registry_get_first(ref_registry);
    while (agent_id != 0) {
        agent_t *ref_agent = (agent_t*)ar_agent_registry_find_agent(ref_registry, agent_id);
        if (ref_agent && ar_agent_get_method(ref_agent) == ref_method) {
            count++;
        }
        agent_id = ar_agent_registry_get_next(ref_registry, agent_id);
    }
    
    return count;
}

/* Check if two method versions are compatible for update */
bool ar__agent_update__are_compatible(const method_t *ref_old_method,
                                   const method_t *ref_new_method) {
    if (!ref_old_method || !ref_new_method) {
        return false;
    }
    
    // Get method names
    const char *old_name = ar_method_get_name(ref_old_method);
    const char *new_name = ar_method_get_name(ref_new_method);
    
    // Methods must have the same name
    if (!old_name || !new_name || strcmp(old_name, new_name) != 0) {
        return false;
    }
    
    // Get version strings
    const char *old_version = ar_method_get_version(ref_old_method);
    const char *new_version = ar_method_get_version(ref_new_method);
    
    if (!old_version || !new_version) {
        return false;
    }
    
    // Check version compatibility (same major version)
    return ar_semver_are_compatible(old_version, new_version);
}

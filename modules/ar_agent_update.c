/**
 * @file agerun_agent_update.c
 * @brief Implementation of the agent update module
 */

#include "ar_agent_update.h"
#include "ar_agent.h"
#include "ar_method.h"
#include "ar_semver.h"
#include "ar_io.h"
#include "ar_heap.h"
#include <stdio.h>
#include <string.h>

/* Update agents using a specific method to use a different method */
int ar_agent_update__update_methods(ar_agent_registry_t *ref_registry,
                                  const ar_method_t *ref_old_method, 
                                  const ar_method_t *ref_new_method,
                                  bool send_lifecycle_events) {
    if (!ref_registry || !ref_old_method || !ref_new_method) {
        return 0;
    }
    
    // Verify that the methods are compatible
    if (!ar_agent_update__are_compatible(ref_old_method, ref_new_method)) {
        ar_io__warning("Cannot update agents to incompatible method version");
        return 0;
    }
    
    // Get version strings for logging
    const char *method_name = ar_method__get_name(ref_old_method);
    const char *old_version = ar_method__get_version(ref_old_method);
    const char *new_version = ar_method__get_version(ref_new_method);
    
    ar_io__info("Updating agents from method %s version %s to version %s",
               method_name, old_version, new_version);
    
    // Update all agents that use the old method
    int count = 0;
    int64_t agent_id = ar_agent_registry__get_first(ref_registry);
    while (agent_id != 0) {
        ar_agent_t *mut_agent = (ar_agent_t*)ar_agent_registry__find_agent(ref_registry, agent_id);
        if (mut_agent && ar_agent__get_method(mut_agent) == ref_old_method) {
            if (ar_agent__update_method(mut_agent, ref_new_method, send_lifecycle_events)) {
                count++;
            }
        }
        agent_id = ar_agent_registry__get_next(ref_registry, agent_id);
    }
    
    if (count > 0) {
        ar_io__info("Updated %d agents to new method version", count);
        if (send_lifecycle_events) {
            ar_io__info("Queued %d sleep and %d wake messages", count, count);
        }
    }
    
    return count;
}

/* Count the number of agents using a specific method */
int ar_agent_update__count_using_method(ar_agent_registry_t *ref_registry,
                                      const ar_method_t *ref_method) {
    if (!ref_registry || !ref_method) {
        return 0;
    }
    
    // Count agents using this method
    int count = 0;
    int64_t agent_id = ar_agent_registry__get_first(ref_registry);
    while (agent_id != 0) {
        ar_agent_t *ref_agent = (ar_agent_t*)ar_agent_registry__find_agent(ref_registry, agent_id);
        if (ref_agent && ar_agent__get_method(ref_agent) == ref_method) {
            count++;
        }
        agent_id = ar_agent_registry__get_next(ref_registry, agent_id);
    }
    
    return count;
}

/* Check if two method versions are compatible for update */
bool ar_agent_update__are_compatible(const ar_method_t *ref_old_method,
                                   const ar_method_t *ref_new_method) {
    if (!ref_old_method || !ref_new_method) {
        return false;
    }
    
    // Get method names
    const char *old_name = ar_method__get_name(ref_old_method);
    const char *new_name = ar_method__get_name(ref_new_method);
    
    // Methods must have the same name
    if (!old_name || !new_name || strcmp(old_name, new_name) != 0) {
        return false;
    }
    
    // Get version strings
    const char *old_version = ar_method__get_version(ref_old_method);
    const char *new_version = ar_method__get_version(ref_new_method);
    
    if (!old_version || !new_version) {
        return false;
    }
    
    // Check version compatibility (same major version)
    return ar_semver__are_compatible(old_version, new_version);
}

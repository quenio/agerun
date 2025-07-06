#ifndef AGERUN_AGENT_UPDATE_H
#define AGERUN_AGENT_UPDATE_H

#include <stdbool.h>
#include <stdint.h>
#include "ar_agent_registry.h"

/* Forward declarations */
typedef struct method_s ar_method_t;

/**
 * @file agerun_agent_update.h
 * @brief Agent update module for managing method version updates
 * 
 * This module handles updating agents from one method version to another,
 * including version compatibility checking and lifecycle event coordination.
 */

/**
 * Update agents using a specific method to use a different method
 * @param ref_registry The agent registry to use (borrowed reference)
 * @param ref_old_method The old method being used (borrowed reference)
 * @param ref_new_method The new method to use (borrowed reference)
 * @param send_lifecycle_events If true, send sleep/wake messages during update
 * @return Number of agents updated
 * @note Ownership: Does not take ownership of any references.
 *       The update process involves:
 *       1. Agent finishes processing current message
 *       2. Sleep message is sent to agent (if send_lifecycle_events is true)
 *       3. Method reference is updated
 *       4. Wake message is sent to agent (if send_lifecycle_events is true)
 *       IMPORTANT: The caller must process 2*update_count messages after this call
 *       if lifecycle events are enabled to ensure all sleep and wake messages are processed.
 */
int ar_agent_update__update_methods(ar_agent_registry_t *ref_registry,
                                    const ar_method_t *ref_old_method, 
                                    const ar_method_t *ref_new_method,
                                    bool send_lifecycle_events);

/**
 * Count the number of agents using a specific method
 * @param ref_registry The agent registry to use (borrowed reference)
 * @param ref_method The method to check (borrowed reference)
 * @return Number of active agents using the method
 * @note Ownership: Does not take ownership of any references.
 */
int ar_agent_update__count_using_method(ar_agent_registry_t *ref_registry,
                                        const ar_method_t *ref_method);

/**
 * Check if two method versions are compatible for update
 * @param ref_old_method The old method (borrowed reference)
 * @param ref_new_method The new method (borrowed reference)
 * @return true if methods are compatible (same name, compatible versions)
 * @note Ownership: Does not take ownership of either method reference.
 */
bool ar_agent_update__are_compatible(const ar_method_t *ref_old_method,
                                     const ar_method_t *ref_new_method);

#endif /* AGERUN_AGENT_UPDATE_H */

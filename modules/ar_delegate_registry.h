#ifndef AGERUN_DELEGATE_REGISTRY_H
#define AGERUN_DELEGATE_REGISTRY_H

#include <stdbool.h>
#include <stdint.h>

/* Forward declarations */
typedef struct ar_delegate_s ar_delegate_t;
typedef struct ar_delegate_registry_s ar_delegate_registry_t;

/**
 * @file ar_delegate_registry.h
 * @brief Delegate registry module for managing delegate instances
 *
 * This module handles delegate ID registration, tracking proxy objects,
 * and providing iteration capabilities over the delegate registry.
 * It serves as a central registry for all delegate instances in the system.
 */

/**
 * Create a new delegate registry
 * @return New delegate registry instance, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy
 */
ar_delegate_registry_t* ar_delegate_registry__create(void);

/**
 * Destroy a delegate registry
 * @param own_registry The registry to destroy (ownership transferred)
 * @note This destroys all registered delegates and the registry itself
 */
void ar_delegate_registry__destroy(ar_delegate_registry_t *own_registry);

/**
 * Get the number of registered delegates
 * @param ref_registry The registry to query (borrowed reference)
 * @return Number of registered delegates
 */
int ar_delegate_registry__count(const ar_delegate_registry_t *ref_registry);

/**
 * Register a delegate with a given ID
 * @param mut_registry The registry to modify (mutable reference)
 * @param proxy_id The delegate ID (negative integer by convention)
 * @param own_proxy The delegate to register (ownership transferred)
 * @return true if successful, false otherwise
 * @note Ownership: Registry takes ownership of the delegate
 */
bool ar_delegate_registry__register(ar_delegate_registry_t *mut_registry, int64_t proxy_id, ar_delegate_t *own_proxy);

/**
 * Unregister a delegate ID
 * @param mut_registry The registry to modify (mutable reference)
 * @param proxy_id The delegate ID to unregister
 * @return true if successful, false otherwise
 * @note This destroys the delegate
 */
bool ar_delegate_registry__unregister(ar_delegate_registry_t *mut_registry, int64_t proxy_id);

/**
 * Find a registered proxy by ID
 * @param ref_registry The registry to query (borrowed reference)
 * @param proxy_id The delegate ID to find
 * @return The delegate object, or NULL if not found
 * @note Returns a borrowed reference - caller should not free
 */
ar_delegate_t* ar_delegate_registry__find(const ar_delegate_registry_t *ref_registry, int64_t proxy_id);

/**
 * Check if a delegate ID is registered
 * @param ref_registry The registry to query (borrowed reference)
 * @param proxy_id The delegate ID to check
 * @return true if registered, false otherwise
 */
bool ar_delegate_registry__is_registered(const ar_delegate_registry_t *ref_registry, int64_t proxy_id);

/**
 * Clear all delegates from the registry
 * @param mut_registry The registry to clear (mutable reference)
 * @note This destroys all registered delegates
 */
void ar_delegate_registry__clear(ar_delegate_registry_t *mut_registry);

#endif /* AGERUN_DELEGATE_REGISTRY_H */

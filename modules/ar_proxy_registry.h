#ifndef AGERUN_PROXY_REGISTRY_H
#define AGERUN_PROXY_REGISTRY_H

#include <stdbool.h>
#include <stdint.h>

/* Forward declarations */
typedef struct ar_proxy_s ar_proxy_t;
typedef struct ar_proxy_registry_s ar_proxy_registry_t;

/**
 * @file ar_proxy_registry.h
 * @brief Proxy registry module for managing proxy instances
 *
 * This module handles proxy ID registration, tracking proxy objects,
 * and providing iteration capabilities over the proxy registry.
 * It serves as a central registry for all proxy instances in the system.
 */

/**
 * Create a new proxy registry
 * @return New proxy registry instance, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy
 */
ar_proxy_registry_t* ar_proxy_registry__create(void);

/**
 * Destroy a proxy registry
 * @param own_registry The registry to destroy (ownership transferred)
 * @note This destroys all registered proxies and the registry itself
 */
void ar_proxy_registry__destroy(ar_proxy_registry_t *own_registry);

/**
 * Get the number of registered proxies
 * @param ref_registry The registry to query (borrowed reference)
 * @return Number of registered proxies
 */
int ar_proxy_registry__count(const ar_proxy_registry_t *ref_registry);

/**
 * Register a proxy with a given ID
 * @param mut_registry The registry to modify (mutable reference)
 * @param proxy_id The proxy ID (negative integer by convention)
 * @param own_proxy The proxy to register (ownership transferred)
 * @return true if successful, false otherwise
 * @note Ownership: Registry takes ownership of the proxy
 */
bool ar_proxy_registry__register(ar_proxy_registry_t *mut_registry, int64_t proxy_id, ar_proxy_t *own_proxy);

/**
 * Unregister a proxy ID
 * @param mut_registry The registry to modify (mutable reference)
 * @param proxy_id The proxy ID to unregister
 * @return true if successful, false otherwise
 * @note This destroys the proxy
 */
bool ar_proxy_registry__unregister(ar_proxy_registry_t *mut_registry, int64_t proxy_id);

/**
 * Find a registered proxy by ID
 * @param ref_registry The registry to query (borrowed reference)
 * @param proxy_id The proxy ID to find
 * @return The proxy object, or NULL if not found
 * @note Returns a borrowed reference - caller should not free
 */
ar_proxy_t* ar_proxy_registry__find(const ar_proxy_registry_t *ref_registry, int64_t proxy_id);

/**
 * Check if a proxy ID is registered
 * @param ref_registry The registry to query (borrowed reference)
 * @param proxy_id The proxy ID to check
 * @return true if registered, false otherwise
 */
bool ar_proxy_registry__is_registered(const ar_proxy_registry_t *ref_registry, int64_t proxy_id);

/**
 * Clear all proxies from the registry
 * @param mut_registry The registry to clear (mutable reference)
 * @note This destroys all registered proxies
 */
void ar_proxy_registry__clear(ar_proxy_registry_t *mut_registry);

#endif /* AGERUN_PROXY_REGISTRY_H */

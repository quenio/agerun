/**
 * @file ar_proxy.h
 * @brief Proxy module for managing external communication
 *
 * The proxy module provides a foundation for mediating between agents
 * and external resources (files, network, logging, etc.) with built-in
 * security controls and sandboxing.
 */

#ifndef AGERUN_PROXY_H
#define AGERUN_PROXY_H

/**
 * Opaque type for a proxy instance
 */
typedef struct ar_proxy_s ar_proxy_t;

/**
 * Creates a new proxy instance
 * @return A new proxy instance, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
ar_proxy_t* ar_proxy__create(void);

/**
 * Destroys a proxy instance
 * @param own_proxy The proxy to destroy
 * @note Ownership: Takes ownership and destroys the proxy.
 */
void ar_proxy__destroy(ar_proxy_t *own_proxy);

#endif /* AGERUN_PROXY_H */

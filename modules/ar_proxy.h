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

#include <stdbool.h>
#include <stdint.h>
#include "ar_log.h"
#include "ar_data.h"

/**
 * Opaque type for a proxy instance
 */
typedef struct ar_proxy_s ar_proxy_t;

/**
 * Creates a new proxy instance
 * @param ref_log The log instance for error reporting (borrowed reference)
 * @param type The proxy type identifier (borrowed string, e.g., "file", "network", "log")
 * @return A new proxy instance, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 *       The proxy borrows the log reference - caller must ensure log outlives proxy.
 *       The proxy borrows the type string - caller must ensure string outlives proxy.
 */
ar_proxy_t* ar_proxy__create(ar_log_t *ref_log, const char *type);

/**
 * Destroys a proxy instance
 * @param own_proxy The proxy to destroy
 * @note Ownership: Takes ownership and destroys the proxy.
 */
void ar_proxy__destroy(ar_proxy_t *own_proxy);

/**
 * Gets the log instance from a proxy
 * @param ref_proxy The proxy instance
 * @return The log instance, or NULL if not set
 * @note Ownership: Returns a borrowed reference. Do not destroy.
 */
ar_log_t* ar_proxy__get_log(const ar_proxy_t *ref_proxy);

/**
 * Gets the type identifier from a proxy
 * @param ref_proxy The proxy instance
 * @return The type identifier, or NULL if not set
 * @note Ownership: Returns a borrowed reference. Do not destroy.
 */
const char* ar_proxy__get_type(const ar_proxy_t *ref_proxy);

/**
 * Handles a message sent to this proxy
 * @param ref_proxy The proxy instance
 * @param ref_message The message to handle (borrowed reference)
 * @param sender_id The ID of the agent sending the message
 * @return true if message was handled, false otherwise
 * @note Ownership: Proxy borrows the message, does not take ownership.
 *       Caller retains ownership and must destroy the message.
 */
bool ar_proxy__handle_message(ar_proxy_t *ref_proxy, ar_data_t *ref_message, int64_t sender_id);

#endif /* AGERUN_PROXY_H */

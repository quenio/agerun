#ifndef AGERUN_NETWORK_DELEGATE_H
#define AGERUN_NETWORK_DELEGATE_H

#include <stddef.h>
#include <stdint.h>
#include "ar_log.h"
#include "ar_data.h"

/**
 * Opaque type for a network delegate instance
 */
typedef struct ar_network_delegate_s ar_network_delegate_t;

/**
 * Creates a new network delegate instance
 * @param ref_log The log instance for error reporting (borrowed reference, may be NULL)
 * @param ref_whitelist Array of allowed URL prefixes (borrowed strings)
 * @param whitelist_count Number of entries in the whitelist
 * @param max_response_size Maximum response size for stubbed requests (0 uses default limit)
 * @param timeout_seconds Timeout for stubbed requests (0 uses default timeout)
 * @return A new network delegate instance, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 *       The delegate borrows the log reference - caller must ensure log outlives delegate.
 *       The delegate copies the whitelist entries internally.
 */
ar_network_delegate_t* ar_network_delegate__create(
    ar_log_t *ref_log,
    const char **ref_whitelist,
    size_t ref_whitelist_count,
    size_t ref_max_response_size,
    int ref_timeout_seconds);

/**
 * Destroys a network delegate instance
 * @param own_delegate The network delegate to destroy
 * @note Ownership: Takes ownership and destroys the delegate.
 */
void ar_network_delegate__destroy(ar_network_delegate_t *own_delegate);

/**
 * Gets the type identifier for a network delegate
 * @param ref_delegate The network delegate instance (borrowed reference)
 * @return The type string "network"
 * @note Ownership: Returns a borrowed string reference.
 */
const char* ar_network_delegate__get_type(const ar_network_delegate_t *ref_delegate);

/**
 * Handles a network delegate message
 * @param mut_delegate The network delegate instance (mutable reference)
 * @param ref_message The message to handle (borrowed reference)
 * @param sender_id The agent sending the message
 * @return A response map indicating success or error
 * @note Ownership: Returns an owned value that caller must destroy.
 */
ar_data_t* ar_network_delegate__handle_message(
    ar_network_delegate_t *mut_delegate,
    ar_data_t *ref_message,
    int64_t sender_id);

#endif /* AGERUN_NETWORK_DELEGATE_H */

#ifndef AGERUN_LOG_DELEGATE_H
#define AGERUN_LOG_DELEGATE_H

#include <stdint.h>
#include "ar_log.h"
#include "ar_data.h"

/**
 * Opaque type for a log delegate instance
 */
typedef struct ar_log_delegate_s ar_log_delegate_t;

/**
 * Creates a new log delegate instance
 * @param ref_log The log instance for error reporting (borrowed reference, may be NULL)
 * @param ref_min_level Minimum log level to accept ("info", "warning", "error"); NULL uses "info"
 * @return A new log delegate instance, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 *       The delegate borrows the log reference - caller must ensure log outlives delegate.
 */
ar_log_delegate_t* ar_log_delegate__create(ar_log_t *ref_log, const char *ref_min_level);

/**
 * Destroys a log delegate instance
 * @param own_delegate The log delegate to destroy
 * @note Ownership: Takes ownership and destroys the delegate.
 */
void ar_log_delegate__destroy(ar_log_delegate_t *own_delegate);

/**
 * Gets the type identifier for a log delegate
 * @param ref_delegate The log delegate instance (borrowed reference)
 * @return The type string "log"
 * @note Ownership: Returns a borrowed string reference.
 */
const char* ar_log_delegate__get_type(const ar_log_delegate_t *ref_delegate);

/**
 * Handles a log delegate message
 * @param mut_delegate The log delegate instance (mutable reference)
 * @param ref_message The message to handle (borrowed reference)
 * @param sender_id The agent sending the message
 * @return A response map indicating success or error
 * @note Ownership: Returns an owned value that caller must destroy.
 */
ar_data_t* ar_log_delegate__handle_message(
    ar_log_delegate_t *mut_delegate,
    ar_data_t *ref_message,
    int64_t sender_id);

#endif /* AGERUN_LOG_DELEGATE_H */

#ifndef AGERUN_FILE_DELEGATE_H
#define AGERUN_FILE_DELEGATE_H

#include <stddef.h>
#include <stdint.h>
#include "ar_log.h"
#include "ar_data.h"

/**
 * Opaque type for a file delegate instance
 */
typedef struct ar_file_delegate_s ar_file_delegate_t;

/**
 * Creates a new file delegate instance
 * @param ref_log The log instance for error reporting (borrowed reference, may be NULL)
 * @param ref_allowed_path The allowed directory path for file operations (borrowed string)
 * @param max_file_size Maximum file size for read operations (0 uses default limit)
 * @return A new file delegate instance, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 *       The delegate borrows the log reference - caller must ensure log outlives delegate.
 *       NULL log is acceptable - the log module handles NULL gracefully.
 *       The delegate copies the allowed_path string internally.
 */
ar_file_delegate_t* ar_file_delegate__create(
    ar_log_t *ref_log,
    const char *ref_allowed_path,
    size_t max_file_size);

/**
 * Destroys a file delegate instance
 * @param own_delegate The file delegate to destroy
 * @note Ownership: Takes ownership and destroys the delegate.
 */
void ar_file_delegate__destroy(ar_file_delegate_t *own_delegate);

/**
 * Gets the type identifier for a file delegate
 * @param ref_delegate The file delegate instance (borrowed reference)
 * @return The type string "file"
 * @note Ownership: Returns a borrowed string reference.
 */
const char* ar_file_delegate__get_type(const ar_file_delegate_t *ref_delegate);

/**
 * Handles a file delegate message
 * @param mut_delegate The file delegate instance (mutable reference)
 * @param ref_message The message to handle (borrowed reference)
 * @param sender_id The agent sending the message
 * @return A response map indicating success or error
 * @note Ownership: Returns an owned value that caller must destroy.
 */
ar_data_t* ar_file_delegate__handle_message(
    ar_file_delegate_t *mut_delegate,
    ar_data_t *ref_message,
    int64_t sender_id);

#endif /* AGERUN_FILE_DELEGATE_H */

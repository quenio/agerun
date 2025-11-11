#ifndef AGERUN_FILE_DELEGATE_H
#define AGERUN_FILE_DELEGATE_H

#include "ar_log.h"

/**
 * Opaque type for a file delegate instance
 */
typedef struct ar_file_delegate_s ar_file_delegate_t;

/**
 * Creates a new file delegate instance
 * @param ref_log The log instance for error reporting (borrowed reference, may be NULL)
 * @param ref_allowed_path The allowed directory path for file operations (borrowed string)
 * @return A new file delegate instance, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 *       The delegate borrows the log reference - caller must ensure log outlives delegate.
 *       NULL log is acceptable - the log module handles NULL gracefully.
 *       The delegate borrows the allowed_path string - caller must ensure string outlives delegate.
 */
ar_file_delegate_t* ar_file_delegate__create(ar_log_t *ref_log, const char *ref_allowed_path);

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

#endif /* AGERUN_FILE_DELEGATE_H */


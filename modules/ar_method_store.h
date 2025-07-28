#ifndef AGERUN_METHOD_STORE_H
#define AGERUN_METHOD_STORE_H

/**
 * @file ar_method_store.h
 * @brief Method store module for persisting and loading method definitions
 * 
 * This module handles saving and loading method definitions to/from persistent storage.
 * It manages the methodology.agerun file format and ensures data integrity during
 * persistence operations.
 */

#include <stdbool.h>

/* Forward declarations */
typedef struct ar_method_registry_s ar_method_registry_t;
typedef struct ar_method_store_s ar_method_store_t;
typedef struct ar_log_s ar_log_t;

/**
 * Create a new method store instance
 * @param ref_log The log instance to use for error reporting (borrowed reference, may be NULL)
 * @param ref_registry The method registry to use for method access (required)
 * @param ref_file_path The file path to use for persistence (required)
 * @return New method store instance or NULL on failure
 * @note Ownership: Caller owns the returned store
 * @note The store borrows the registry reference and copies the file path
 * @note If ref_log is NULL, the store works normally but doesn't propagate logs to methods
 */
ar_method_store_t* ar_method_store__create(ar_log_t *ref_log,
                                           ar_method_registry_t *ref_registry, 
                                           const char *ref_file_path);

/**
 * Destroy a method store instance
 * @param own_store The store to destroy
 * @note Ownership: Takes ownership and destroys the store
 */
void ar_method_store__destroy(ar_method_store_t *own_store);

/**
 * Get the path to the method store file
 * @param ref_store The method store instance
 * @return Path to the store file (constant string)
 * @note Ownership: Borrows store reference
 */
const char* ar_method_store__get_path(ar_method_store_t *ref_store);

/**
 * Check if method store file exists
 * @param ref_store The method store instance
 * @return true if store file exists, false otherwise
 * @note Ownership: Borrows store reference
 */
bool ar_method_store__exists(ar_method_store_t *ref_store);

/**
 * Save all methods from the store's registry to persistent storage
 * @param ref_store The method store instance
 * @return true if successful, false otherwise
 * @note Creates a backup of existing file before saving
 * @note Ownership: Borrows store reference, does not affect ownership
 */
bool ar_method_store__save(ar_method_store_t *ref_store);

/**
 * Load methods from persistent storage into the store's registry
 * @param mut_store The method store instance
 * @return true if successful, false otherwise
 * @note Ownership: Mutates the store's registry by adding loaded methods
 */
bool ar_method_store__load(ar_method_store_t *mut_store);

/**
 * Delete the method store file
 * @param ref_store The method store instance
 * @return true if successful or file didn't exist, false on error
 * @note Creates a backup before deletion
 * @note Ownership: Borrows store reference
 */
bool ar_method_store__delete(ar_method_store_t *ref_store);

#endif /* AGERUN_METHOD_STORE_H */
#ifndef AGERUN_METHOD_REGISTRY_H
#define AGERUN_METHOD_REGISTRY_H

/**
 * @file ar_method_registry.h
 * @brief Method registry module for managing method storage and lookup
 * 
 * This module handles method registration, storage, and lookup operations.
 * It manages multiple versions of methods and provides enumeration capabilities.
 */

/* Forward declarations */
typedef struct ar_method_s ar_method_t;
typedef struct ar_list_s ar_list_t;

/* Opaque type for method registry */
typedef struct ar_method_registry_s ar_method_registry_t;

/**
 * Create a new method registry
 * @return New method registry instance, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy
 */
ar_method_registry_t* ar_method_registry__create(void);

/**
 * Destroy a method registry
 * @param own_registry The registry to destroy (ownership transferred)
 * @note This destroys all registered methods
 */
void ar_method_registry__destroy(ar_method_registry_t *own_registry);

/**
 * Register a method in the registry
 * @param mut_registry The registry to modify (mutable reference)
 * @param own_method The method to register (ownership transferred)
 * @note The registry takes ownership of the method
 */
void ar_method_registry__register_method(ar_method_registry_t *mut_registry, ar_method_t *own_method);

/**
 * Get the number of unique method names in the registry
 * @param ref_registry The registry to query (borrowed reference)
 * @return Number of unique method names
 */
int ar_method_registry__get_unique_name_count(const ar_method_registry_t *ref_registry);

/**
 * Find the index of a method by name
 * @param ref_registry The registry to search (borrowed reference)
 * @param ref_name The method name to find (borrowed reference)
 * @return Index of the method (0-based), or -1 if not found
 */
int ar_method_registry__find_method_index(const ar_method_registry_t *ref_registry, const char *ref_name);

/**
 * Get a method by exact name and version match
 * @param ref_registry The registry to search (borrowed reference)
 * @param ref_name The method name (borrowed reference)
 * @param ref_version The exact version to match (borrowed reference)
 * @return The method if found (borrowed reference), or NULL if not found
 * @note Returns a borrowed reference - caller should not destroy
 */
ar_method_t* ar_method_registry__get_method_by_exact_match(const ar_method_registry_t *ref_registry,
                                                           const char *ref_name,
                                                           const char *ref_version);

/**
 * Get the latest version of a method by name
 * @param ref_registry The registry to search (borrowed reference)
 * @param ref_name The method name (borrowed reference)
 * @return The latest version of the method (borrowed reference), or NULL if not found
 * @note Returns a borrowed reference - caller should not destroy
 * @note Uses semantic versioning to determine the latest version
 */
ar_method_t* ar_method_registry__get_latest_version(const ar_method_registry_t *ref_registry,
                                                    const char *ref_name);

/**
 * Unregister a specific version of a method
 * @param mut_registry The registry to modify (mutable reference)
 * @param ref_name The method name (borrowed reference)
 * @param ref_version The specific version to unregister (borrowed reference)
 * @note This destroys the specified method version
 * @note If this is the last version of the method, the method name is removed
 */
void ar_method_registry__unregister_method(ar_method_registry_t *mut_registry,
                                          const char *ref_name,
                                          const char *ref_version);

/**
 * Check if a method exists in the registry
 * @param ref_registry The registry to search (borrowed reference)
 * @param ref_name The method name (borrowed reference)
 * @param ref_version Optional specific version to check (borrowed reference, can be NULL)
 * @return 1 if the method exists, 0 otherwise
 * @note If ref_version is NULL, checks if any version of the method exists
 * @note If ref_version is provided, checks if that specific version exists
 */
int ar_method_registry__method_exists(const ar_method_registry_t *ref_registry,
                                     const char *ref_name,
                                     const char *ref_version);

/**
 * Get all methods in the registry
 * @param ref_registry The registry to enumerate (borrowed reference)
 * @return A list containing all methods (owned value)
 * @note Returns an owned list that caller must destroy
 * @note The list contains borrowed references to methods
 * @note The list includes all versions of all methods
 */
ar_list_t* ar_method_registry__get_all_methods(const ar_method_registry_t *ref_registry);

#endif /* AGERUN_METHOD_REGISTRY_H */
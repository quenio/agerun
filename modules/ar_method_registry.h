#ifndef AGERUN_METHOD_REGISTRY_H
#define AGERUN_METHOD_REGISTRY_H

/**
 * @file ar_method_registry.h
 * @brief Method registry module for managing method storage and lookup
 * 
 * This module handles method registration, storage, and lookup operations.
 * It manages multiple versions of methods and provides enumeration capabilities.
 */

/* Forward declaration */
typedef struct ar_method_s ar_method_t;

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

#endif /* AGERUN_METHOD_REGISTRY_H */
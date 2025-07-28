#ifndef AGERUN_METHOD_RESOLVER_H
#define AGERUN_METHOD_RESOLVER_H

/**
 * @file ar_method_resolver.h
 * @brief Method resolver module for handling version resolution and method selection
 * 
 * This module handles version resolution logic including partial version matching
 * as defined in the AgeRun specification. It acts as an intermediary between
 * the methodology module and the method registry.
 */

/* Forward declarations */
typedef struct ar_method_s ar_method_t;
typedef struct ar_method_registry_s ar_method_registry_t;

/* Opaque type for method resolver */
typedef struct ar_method_resolver_s ar_method_resolver_t;

/**
 * Create a new method resolver
 * @param ref_registry The method registry to use for lookups (borrowed reference)
 * @return New method resolver instance, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy
 * @note The resolver borrows the registry reference and does not take ownership
 */
ar_method_resolver_t* ar_method_resolver__create(ar_method_registry_t *ref_registry);

/**
 * Destroy a method resolver
 * @param own_resolver The resolver to destroy (ownership transferred)
 */
void ar_method_resolver__destroy(ar_method_resolver_t *own_resolver);

/**
 * Resolve a method by name and version
 * @param ref_resolver The resolver to use (borrowed reference)
 * @param ref_name The method name (borrowed reference)
 * @param ref_version The version string (borrowed reference, can be NULL)
 * @return The resolved method (borrowed reference), or NULL if not found
 * @note Version can be:
 *       - NULL/empty: returns latest version
 *       - Full semver (e.g., "1.2.3"): returns exact match
 *       - Partial version (e.g., "1" or "1.2"): returns latest matching version
 * @note Returns a borrowed reference - caller should not destroy
 */
ar_method_t* ar_method_resolver__resolve_method(ar_method_resolver_t *ref_resolver,
                                                const char *ref_name,
                                                const char *ref_version);

#endif /* AGERUN_METHOD_RESOLVER_H */
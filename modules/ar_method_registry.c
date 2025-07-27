#include "ar_method_registry.h"
#include "ar_method.h"
#include "ar_heap.h"
#include "ar_semver.h"
#include "ar_list.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Constants from ar_methodology.c */
#define MAX_METHODS 100
#define MAX_VERSIONS_PER_METHOD 32

/**
 * Internal structure for method registry
 */
struct ar_method_registry_s {
    ar_method_t ***own_methods;    /* Dynamic 2D array for method storage */
    int *own_method_counts;        /* Dynamic array of method counts per name */
    int method_name_count;         /* Number of unique method names */
    int max_methods;               /* Current capacity for method names */
};

/**
 * Create a new method registry
 * @return New method registry instance, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy
 */
ar_method_registry_t* ar_method_registry__create(void) {
    ar_method_registry_t *own_registry = AR__HEAP__MALLOC(sizeof(ar_method_registry_t), "ar_method_registry_t");
    if (!own_registry) {
        return NULL;
    }
    
    // Initialize with initial capacity
    own_registry->max_methods = 10;
    own_registry->method_name_count = 0;
    
    // Allocate the methods array
    own_registry->own_methods = AR__HEAP__MALLOC(
        sizeof(ar_method_t**) * (size_t)own_registry->max_methods, 
        "method registry methods array"
    );
    if (!own_registry->own_methods) {
        AR__HEAP__FREE(own_registry);
        return NULL;
    }
    
    // Allocate the method counts array
    own_registry->own_method_counts = AR__HEAP__MALLOC(
        sizeof(int) * (size_t)own_registry->max_methods,
        "method registry counts array"
    );
    if (!own_registry->own_method_counts) {
        AR__HEAP__FREE(own_registry->own_methods);
        AR__HEAP__FREE(own_registry);
        return NULL;
    }
    
    // Initialize arrays
    for (int i = 0; i < own_registry->max_methods; i++) {
        own_registry->own_methods[i] = NULL;
        own_registry->own_method_counts[i] = 0;
    }
    
    return own_registry;
}

/**
 * Destroy a method registry
 * @param own_registry The registry to destroy (ownership transferred)
 * @note This destroys all registered methods
 */
void ar_method_registry__destroy(ar_method_registry_t *own_registry) {
    if (own_registry) {
        // Clean up all methods
        if (own_registry->own_methods) {
            for (int i = 0; i < own_registry->method_name_count; i++) {
                if (own_registry->own_methods[i]) {
                    // Free all versions for this method name
                    for (int j = 0; j < own_registry->own_method_counts[i]; j++) {
                        if (own_registry->own_methods[i][j]) {
                            ar_method__destroy(own_registry->own_methods[i][j]);
                        }
                    }
                    // Free the versions array for this method name
                    AR__HEAP__FREE(own_registry->own_methods[i]);
                }
            }
            // Free the methods array itself
            AR__HEAP__FREE(own_registry->own_methods);
        }
        
        // Free the method counts array
        if (own_registry->own_method_counts) {
            AR__HEAP__FREE(own_registry->own_method_counts);
        }
        
        // Free the registry
        AR__HEAP__FREE(own_registry);
    }
}

/**
 * Grow the registry storage capacity
 * @param mut_registry The registry to grow (mutable reference)
 * @return true if successful, false on allocation failure
 */
static bool _grow_registry(ar_method_registry_t *mut_registry) {
    // Double the capacity
    int new_capacity = mut_registry->max_methods * 2;
    
    // Reallocate methods array
    ar_method_t ***new_methods = AR__HEAP__REALLOC(
        mut_registry->own_methods,
        sizeof(ar_method_t**) * (size_t)new_capacity,
        "method registry methods array"
    );
    if (!new_methods) {
        return false;
    }
    mut_registry->own_methods = new_methods;
    
    // Reallocate counts array
    int *new_counts = AR__HEAP__REALLOC(
        mut_registry->own_method_counts,
        sizeof(int) * (size_t)new_capacity,
        "method registry counts array"
    );
    if (!new_counts) {
        return false;
    }
    mut_registry->own_method_counts = new_counts;
    
    // Initialize new slots
    for (int i = mut_registry->max_methods; i < new_capacity; i++) {
        mut_registry->own_methods[i] = NULL;
        mut_registry->own_method_counts[i] = 0;
    }
    
    mut_registry->max_methods = new_capacity;
    return true;
}

/**
 * Register a method in the registry
 * @param mut_registry The registry to modify (mutable reference)
 * @param own_method The method to register (ownership transferred)
 * @note The registry takes ownership of the method
 */
void ar_method_registry__register_method(ar_method_registry_t *mut_registry, ar_method_t *own_method) {
    if (!mut_registry || !own_method) {
        if (own_method) {
            ar_method__destroy(own_method);
        }
        return;
    }
    
    const char *method_name = ar_method__get_name(own_method);
    
    // Check if method with this name already exists
    int method_idx = ar_method_registry__find_method_index(mut_registry, method_name);
    
    if (method_idx < 0) {
        // New method name, create new entry
        
        // Check if we need to grow the arrays
        if (mut_registry->method_name_count >= mut_registry->max_methods) {
            if (!_grow_registry(mut_registry)) {
                ar_method__destroy(own_method);
                return;
            }
        }
        
        method_idx = mut_registry->method_name_count++;
        
        // Allocate versions array for this method
        mut_registry->own_methods[method_idx] = AR__HEAP__MALLOC(
            sizeof(ar_method_t*) * MAX_VERSIONS_PER_METHOD,
            "method versions array"
        );
        if (!mut_registry->own_methods[method_idx]) {
            ar_method__destroy(own_method);
            mut_registry->method_name_count--;
            return;
        }
        
        // Initialize versions array
        for (int i = 0; i < MAX_VERSIONS_PER_METHOD; i++) {
            mut_registry->own_methods[method_idx][i] = NULL;
        }
        mut_registry->own_method_counts[method_idx] = 0;
    }
    
    // Add this version to the method's versions array
    int version_idx = mut_registry->own_method_counts[method_idx];
    mut_registry->own_methods[method_idx][version_idx] = own_method;
    mut_registry->own_method_counts[method_idx]++;
}

/**
 * Get the number of unique method names in the registry
 * @param ref_registry The registry to query (borrowed reference)
 * @return Number of unique method names
 */
int ar_method_registry__get_unique_name_count(const ar_method_registry_t *ref_registry) {
    if (!ref_registry) {
        return 0;
    }
    return ref_registry->method_name_count;
}

/**
 * Find the index of a method by name
 * @param ref_registry The registry to search (borrowed reference)
 * @param ref_name The method name to find (borrowed reference)
 * @return Index of the method (0-based), or -1 if not found
 */
int ar_method_registry__find_method_index(const ar_method_registry_t *ref_registry, const char *ref_name) {
    if (!ref_registry || !ref_name) {
        return -1;
    }
    
    for (int i = 0; i < ref_registry->method_name_count; i++) {
        if (ref_registry->own_methods[i] && ref_registry->own_methods[i][0] != NULL && 
            strcmp(ar_method__get_name(ref_registry->own_methods[i][0]), ref_name) == 0) {
            return i;
        }
    }
    
    return -1;
}

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
                                                           const char *ref_version) {
    if (!ref_registry || !ref_name || !ref_version) {
        return NULL;
    }
    
    // Find the method index
    int method_idx = ar_method_registry__find_method_index(ref_registry, ref_name);
    if (method_idx < 0) {
        return NULL;
    }
    
    // Search through all versions for exact match
    for (int i = 0; i < ref_registry->own_method_counts[method_idx]; i++) {
        ar_method_t *method = ref_registry->own_methods[method_idx][i];
        if (method && strcmp(ar_method__get_version(method), ref_version) == 0) {
            return method;
        }
    }
    
    return NULL;
}

/**
 * Get the latest version of a method by name
 * @param ref_registry The registry to search (borrowed reference)
 * @param ref_name The method name (borrowed reference)
 * @return The latest version of the method (borrowed reference), or NULL if not found
 * @note Returns a borrowed reference - caller should not destroy
 * @note Uses semantic versioning to determine the latest version
 */
ar_method_t* ar_method_registry__get_latest_version(const ar_method_registry_t *ref_registry,
                                                    const char *ref_name) {
    if (!ref_registry || !ref_name) {
        return NULL;
    }
    
    // Find the method index
    int method_idx = ar_method_registry__find_method_index(ref_registry, ref_name);
    if (method_idx < 0 || ref_registry->own_method_counts[method_idx] == 0) {
        return NULL;
    }
    
    // Use semantic versioning to find the latest version
    int latest_idx = 0;
    for (int i = 1; i < ref_registry->own_method_counts[method_idx]; i++) {
        if (ref_registry->own_methods[method_idx][i] != NULL && 
            ref_registry->own_methods[method_idx][latest_idx] != NULL &&
            ar_semver__compare(
                ar_method__get_version(ref_registry->own_methods[method_idx][i]),
                ar_method__get_version(ref_registry->own_methods[method_idx][latest_idx])
            ) > 0) {
            latest_idx = i;
        }
    }
    
    if (latest_idx >= 0 && ref_registry->own_methods[method_idx][latest_idx] != NULL) {
        return ref_registry->own_methods[method_idx][latest_idx];
    }
    
    return NULL;
}

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
                                          const char *ref_version) {
    if (!mut_registry || !ref_name || !ref_version) {
        return;
    }
    
    // Find the method index
    int method_idx = ar_method_registry__find_method_index(mut_registry, ref_name);
    if (method_idx < 0) {
        return; // Method not found
    }
    
    // Find the specific version
    int version_idx = -1;
    for (int i = 0; i < mut_registry->own_method_counts[method_idx]; i++) {
        ar_method_t *method = mut_registry->own_methods[method_idx][i];
        if (method && strcmp(ar_method__get_version(method), ref_version) == 0) {
            version_idx = i;
            break;
        }
    }
    
    if (version_idx < 0) {
        return; // Version not found
    }
    
    // Destroy the method
    ar_method__destroy(mut_registry->own_methods[method_idx][version_idx]);
    
    // Shift remaining versions down
    for (int i = version_idx; i < mut_registry->own_method_counts[method_idx] - 1; i++) {
        mut_registry->own_methods[method_idx][i] = mut_registry->own_methods[method_idx][i + 1];
    }
    
    // Clear the last slot
    mut_registry->own_methods[method_idx][mut_registry->own_method_counts[method_idx] - 1] = NULL;
    mut_registry->own_method_counts[method_idx]--;
    
    // If this was the last version, remove the method name entry
    if (mut_registry->own_method_counts[method_idx] == 0) {
        // Free the versions array for this method
        AR__HEAP__FREE(mut_registry->own_methods[method_idx]);
        
        // Shift remaining method entries down
        for (int i = method_idx; i < mut_registry->method_name_count - 1; i++) {
            mut_registry->own_methods[i] = mut_registry->own_methods[i + 1];
            mut_registry->own_method_counts[i] = mut_registry->own_method_counts[i + 1];
        }
        
        // Clear the last slots
        mut_registry->own_methods[mut_registry->method_name_count - 1] = NULL;
        mut_registry->own_method_counts[mut_registry->method_name_count - 1] = 0;
        mut_registry->method_name_count--;
    }
}

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
                                     const char *ref_version) {
    if (!ref_registry || !ref_name) {
        return 0;
    }
    
    // Check if the method name exists
    int method_idx = ar_method_registry__find_method_index(ref_registry, ref_name);
    if (method_idx < 0) {
        return 0;
    }
    
    // If no specific version requested, the method exists
    if (!ref_version) {
        return 1;
    }
    
    // Check for the specific version
    for (int i = 0; i < ref_registry->own_method_counts[method_idx]; i++) {
        ar_method_t *method = ref_registry->own_methods[method_idx][i];
        if (method && strcmp(ar_method__get_version(method), ref_version) == 0) {
            return 1;
        }
    }
    
    return 0;
}

/**
 * Get all methods in the registry
 * @param ref_registry The registry to enumerate (borrowed reference)
 * @return A list containing all methods (owned value)
 * @note Returns an owned list that caller must destroy
 * @note The list contains borrowed references to methods
 * @note The list includes all versions of all methods
 */
ar_list_t* ar_method_registry__get_all_methods(const ar_method_registry_t *ref_registry) {
    ar_list_t *own_list = ar_list__create();
    if (!own_list) {
        return NULL;
    }
    
    if (!ref_registry) {
        return own_list; // Return empty list
    }
    
    // Add all methods from all method names
    for (int i = 0; i < ref_registry->method_name_count; i++) {
        // Add all versions for this method name
        for (int j = 0; j < ref_registry->own_method_counts[i]; j++) {
            ar_method_t *method = ref_registry->own_methods[i][j];
            if (method) {
                ar_list__add_last(own_list, method);
            }
        }
    }
    
    return own_list;
}
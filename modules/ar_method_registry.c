#include "ar_method_registry.h"
#include "ar_method.h"
#include "ar_heap.h"
#include <stdlib.h>
#include <string.h>

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
    
    // For now, just add the method to the first available slot
    // (This is minimal implementation for green phase)
    int method_idx = mut_registry->method_name_count++;
    
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
    
    // Store the method
    mut_registry->own_methods[method_idx][0] = own_method;
    mut_registry->own_method_counts[method_idx] = 1;
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
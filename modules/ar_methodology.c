#include "ar_methodology.h"
#include "ar_method.h"
#include "ar_method_registry.h"
#include "ar_method_resolver.h"
#include "ar_method_store.h"
#include "ar_string.h"
#include "ar_heap.h"
#include "ar_semver.h"
#include "ar_agency.h"
#include "ar_io.h" /* Include the I/O utilities */
#include "ar_assert.h" /* Include the assertion utilities */
#include "ar_list.h" /* For enumeration */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>

/* Constants */
#define MAX_METHODS 100
#define MAX_METHOD_NAME_LENGTH 256
#define MAX_VERSIONS_PER_METHOD 32
#define BUFFER_SIZE 16384  /* Buffer size for save operations */
#define LINE_SIZE 256      /* Line buffer size for load operations */

/**
 * Validates the format of a methodology file to ensure it can be loaded
 * Provides detailed error message in case of validation failure
 *
 * @param filename Path to the methodology file
 * @param error_message Buffer to store error message if validation fails
 * @param error_size Size of the error message buffer
 * @return true if file is valid, false if there are formatting issues
 */

/* Constants */
#define MAX_INSTRUCTIONS_LENGTH 16384

/* Global State - REMOVED: Now using global instance exclusively */

/* Methodology instance structure */
struct ar_methodology_s {
    ar_log_t *ref_log;                    /* Borrowed reference to log instance */
    ar_method_registry_t *own_registry;   /* Owned method registry for storage */
    ar_method_resolver_t *own_resolver;   /* Owned method resolver for version resolution */
    ar_method_store_t *own_default_store; /* Owned method store for default persistence */
};

/* Global instance removed - use ar_methodology__create to create instances */



/**
 * Creates a new method object and registers it with the methodology module
 * @param ref_name Method name (borrowed reference)
 * @param ref_instructions The method implementation code (borrowed reference)
 * @param ref_version Semantic version string for this method (e.g., "1.0.0")
 * @return true if method was created and registered successfully, false otherwise
 * @note Ownership: This function creates and takes ownership of the method.
 *       The caller should not worry about destroying the method.
 */
/* ar_methodology__create_method removed - use instance-based API */





/* Old static array functions removed - using instance-based versions */





// Functions removed - were exposing internal state

/* ar_methodology__get_method removed - use instance-based API */

/* ar_methodology__save_methods removed - use instance-based API */

/* ar_methodology__cleanup removed - use ar_methodology__destroy on your instance */

/* ar_methodology__register_method removed - use instance-based API */

/* ar_methodology__load_methods removed - use instance-based API */

/* ar_methodology__unregister_method removed - use instance-based API */

ar_methodology_t* ar_methodology__create(ar_log_t *ref_log) {
    // Allocate memory for the methodology instance
    ar_methodology_t *own_methodology = AR__HEAP__MALLOC(sizeof(ar_methodology_t), "ar_methodology_t");
    if (!own_methodology) {
        return NULL;
    }
    
    // Store the log reference
    own_methodology->ref_log = ref_log;
    
    // Create the method registry
    own_methodology->own_registry = ar_method_registry__create();
    if (!own_methodology->own_registry) {
        AR__HEAP__FREE(own_methodology);
        return NULL;
    }
    
    // Create the method resolver
    own_methodology->own_resolver = ar_method_resolver__create(own_methodology->own_registry);
    if (!own_methodology->own_resolver) {
        ar_method_registry__destroy(own_methodology->own_registry);
        AR__HEAP__FREE(own_methodology);
        return NULL;
    }
    
    // Create the default method store
    own_methodology->own_default_store = ar_method_store__create(ref_log, own_methodology->own_registry, METHODOLOGY_FILE_NAME);
    if (!own_methodology->own_default_store) {
        ar_method_resolver__destroy(own_methodology->own_resolver);
        ar_method_registry__destroy(own_methodology->own_registry);
        AR__HEAP__FREE(own_methodology);
        return NULL;
    }
    
    return own_methodology;
}

/**
 * Destroy a methodology instance
 * @param own_methodology The methodology instance to destroy (takes ownership)
 * @note Ownership: Takes ownership and destroys the instance
 */
void ar_methodology__destroy(ar_methodology_t *own_methodology) {
    if (own_methodology) {
        // Destroy the default store
        if (own_methodology->own_default_store) {
            ar_method_store__destroy(own_methodology->own_default_store);
        }
        
        // Destroy the resolver
        if (own_methodology->own_resolver) {
            ar_method_resolver__destroy(own_methodology->own_resolver);
        }
        
        // Destroy the registry (it will handle all methods)
        if (own_methodology->own_registry) {
            ar_method_registry__destroy(own_methodology->own_registry);
        }
        
        // Free the instance
        AR__HEAP__FREE(own_methodology);
    }
}

/* Instance-aware versions of public functions */

ar_method_t* ar_methodology__get_method(ar_methodology_t *ref_methodology, 
                                                   const char *ref_name, 
                                                   const char *ref_version) {
    if (!ref_methodology) {
        return NULL;
    }
    
    // Use the resolver for all version resolution
    return ar_method_resolver__resolve_method(ref_methodology->own_resolver, ref_name, ref_version);
}

void ar_methodology__register_method(ar_methodology_t *mut_methodology, 
                                                   ar_method_t *own_method) {
    if (!mut_methodology || !own_method) {
        if (own_method) {
            ar_method__destroy(own_method);
        }
        return;
    }
    
    // Check for version conflicts before registering
    const char *method_name = ar_method__get_name(own_method);
    const char *method_version = ar_method__get_version(own_method);
    
    if (ar_method_registry__method_exists(mut_methodology->own_registry, method_name, method_version)) {
        ar_io__error("Method %s version %s already exists", method_name, method_version);
        ar_method__destroy(own_method); // Clean up the method
        return;
    }
    
    // Delegate to the registry
    ar_method_registry__register_method(mut_methodology->own_registry, own_method);
    // Ownership transferred to registry
}

bool ar_methodology__create_method(ar_methodology_t *mut_methodology,
                                                 const char *ref_name, 
                                                 const char *ref_instructions,
                                                 const char *ref_version) {
    if (!mut_methodology || !ref_name || !ref_instructions || !ref_version) {
        return false;
    }
    
    // Pass the ar_log from the methodology instance to ar_method__create
    ar_method_t *own_method = ar_method__create_with_log(ref_name, ref_instructions, ref_version, mut_methodology->ref_log);
    if (!own_method) {
        return false;
    }
    
    ar_methodology__register_method(mut_methodology, own_method);
    // ownership transferred
    
    return true;
}

bool ar_methodology__save_methods(ar_methodology_t *ref_methodology, 
                                                const char *ref_filename) {
    if (!ref_methodology || !ref_filename) {
        return false;
    }
    
    // Check if we should use the default store or create a temporary one
    if (strcmp(ref_filename, METHODOLOGY_FILE_NAME) == 0) {
        // Use the default store
        return ar_method_store__save(ref_methodology->own_default_store);
    } else {
        // Create a temporary store for the custom filename
        ar_method_store_t *own_temp_store = ar_method_store__create(ref_methodology->ref_log, ref_methodology->own_registry, ref_filename);
        if (!own_temp_store) {
            return false;
        }
        
        // Save using the temporary store
        bool result = ar_method_store__save(own_temp_store);
        
        // Clean up the temporary store
        ar_method_store__destroy(own_temp_store);
        
        return result;
    }
}

bool ar_methodology__load_methods(ar_methodology_t *mut_methodology, 
                                                const char *ref_filename) {
    if (!mut_methodology || !ref_filename) {
        return false;
    }
    
    // Check if we should use the default store or create a temporary one
    if (strcmp(ref_filename, METHODOLOGY_FILE_NAME) == 0) {
        // Use the default store
        return ar_method_store__load(mut_methodology->own_default_store);
    } else {
        // Create a temporary store for the custom filename
        ar_method_store_t *own_temp_store = ar_method_store__create(mut_methodology->ref_log, mut_methodology->own_registry, ref_filename);
        if (!own_temp_store) {
            return false;
        }
        
        // Load using the temporary store
        bool result = ar_method_store__load(own_temp_store);
        
        // Clean up the temporary store
        ar_method_store__destroy(own_temp_store);
        
        return result;
    }
}

bool ar_methodology__unregister_method(ar_methodology_t *mut_methodology,
                                                     const char *ref_name,
                                                     const char *ref_version) {
    if (!mut_methodology || !ref_name || !ref_version) {
        return false;
    }
    
    // Check if the method exists
    if (!ar_method_registry__method_exists(mut_methodology->own_registry, ref_name, ref_version)) {
        return false;
    }
    
    // Delegate to registry
    ar_method_registry__unregister_method(mut_methodology->own_registry, ref_name, ref_version);
    return true;
}

/**
 * Clean up all method definitions and free resources in a specific methodology instance
 * @param mut_methodology The methodology instance to cleanup (mutable reference)
 * @note This should be called before destroying the methodology instance
 * @note Ownership: This cleans up all methods owned by the instance
 */
void ar_methodology__cleanup(ar_methodology_t *mut_methodology) {
    if (!mut_methodology) {
        return;
    }
    
    // The registry, resolver, and store don't have clear functions.
    // The best we can do is unregister all methods one by one.
    // Get all methods and unregister them.
    
    if (mut_methodology->own_registry) {
        // Get all methods from the registry
        ar_list_t *own_methods = ar_method_registry__get_all_methods(mut_methodology->own_registry);
        if (own_methods) {
            // Get the count and items array
            size_t count = ar_list__count(own_methods);
            
            if (count > 0) {
                // Get array of method pointers
                void **ref_items = ar_list__items(own_methods);
                
                if (ref_items) {
                    // We need to collect names and versions first because unregistering
                    // will modify the registry while we're iterating
                    char **own_names = AR__HEAP__MALLOC(count * sizeof(char*), "method names array");
                    char **own_versions = AR__HEAP__MALLOC(count * sizeof(char*), "method versions array");
                    
                    if (own_names && own_versions) {
                        // Collect all method names and versions
                        for (size_t i = 0; i < count; i++) {
                            ar_method_t *ref_method = (ar_method_t*)ref_items[i];
                            if (ref_method) {
                                own_names[i] = AR__HEAP__STRDUP(ar_method__get_name(ref_method), "method name copy");
                                own_versions[i] = AR__HEAP__STRDUP(ar_method__get_version(ref_method), "method version copy");
                            } else {
                                own_names[i] = NULL;
                                own_versions[i] = NULL;
                            }
                        }
                        
                        // Now unregister all methods
                        for (size_t i = 0; i < count; i++) {
                            if (own_names[i] && own_versions[i]) {
                                ar_method_registry__unregister_method(mut_methodology->own_registry, 
                                                                      own_names[i], own_versions[i]);
                            }
                            
                            // Free the temporary strings
                            if (own_names[i]) {
                                AR__HEAP__FREE(own_names[i]);
                            }
                            if (own_versions[i]) {
                                AR__HEAP__FREE(own_versions[i]);
                            }
                        }
                    }
                    
                    // Free the arrays
                    if (own_names) {
                        AR__HEAP__FREE(own_names);
                    }
                    if (own_versions) {
                        AR__HEAP__FREE(own_versions);
                    }
                    
                    // Free the items array (owned by us)
                    AR__HEAP__FREE(ref_items);
                }
            }
            
            // Destroy the list
            ar_list__destroy(own_methods);
        }
    }
}

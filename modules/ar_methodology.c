#include "ar_methodology.h"
#include "ar_method.h"
#include "ar_method_registry.h"
#include "ar_method_resolver.h"
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
};

/* Global default instance for backward compatibility */
static ar_methodology_t *g_default_methodology = NULL;

/**
 * Get or create the global methodology instance
 * @return The global methodology instance (borrowed reference)
 */
static ar_methodology_t* _get_global_instance(void) {
    if (!g_default_methodology) {
        g_default_methodology = ar_methodology__create(NULL);
    }
    return g_default_methodology;
}



/**
 * Creates a new method object and registers it with the methodology module
 * @param ref_name Method name (borrowed reference)
 * @param ref_instructions The method implementation code (borrowed reference)
 * @param ref_version Semantic version string for this method (e.g., "1.0.0")
 * @return true if method was created and registered successfully, false otherwise
 * @note Ownership: This function creates and takes ownership of the method.
 *       The caller should not worry about destroying the method.
 */
bool ar_methodology__create_method(const char *ref_name, const char *ref_instructions, 
                              const char *ref_version) {
    ar_methodology_t *mut_instance = _get_global_instance();
    if (!mut_instance) {
        return false;
    }
    
    return ar_methodology__create_method_with_instance(mut_instance, ref_name, ref_instructions, ref_version);
}





/* Old static array functions removed - using instance-based versions */





// Functions removed - were exposing internal state

// Main method access function
ar_method_t* ar_methodology__get_method(const char *ref_name, const char *ref_version) {
    ar_methodology_t *ref_instance = _get_global_instance();
    if (!ref_instance) {
        return NULL;
    }
    
    return ar_methodology__get_method_with_instance(ref_instance, ref_name, ref_version);
}

bool ar_methodology__save_methods(void) {
    ar_methodology_t *ref_instance = _get_global_instance();
    if (!ref_instance) {
        return false;
    }
    
    // Delegate to the instance version
    return ar_methodology__save_methods_with_instance(ref_instance, METHODOLOGY_FILE_NAME);
}

void ar_methodology__cleanup(void) {
    // Clean up the global instance if it exists
    if (g_default_methodology) {
        ar_methodology__destroy(g_default_methodology);
        g_default_methodology = NULL;
    }
}

void ar_methodology__register_method(ar_method_t *own_method) {
    ar_methodology_t *mut_instance = _get_global_instance();
    if (!mut_instance) {
        if (own_method) {
            ar_method__destroy(own_method);
        }
        return;
    }
    
    ar_methodology__register_method_with_instance(mut_instance, own_method);
}

bool ar_methodology__load_methods(void) {
    ar_methodology_t *mut_instance = _get_global_instance();
    if (!mut_instance) {
        return false;
    }
    
    // Delegate to the instance version
    return ar_methodology__load_methods_with_instance(mut_instance, METHODOLOGY_FILE_NAME);
}

/**
 * Unregister a method from the methodology
 * @param ref_name Method name (borrowed reference)
 * @param ref_version Version string of the method to unregister
 * @return true if method was successfully unregistered, false otherwise
 * @note This will fail if there are active agents using this method
 */
bool ar_methodology__unregister_method(const char *ref_name, const char *ref_version) {
    ar_methodology_t *mut_instance = _get_global_instance();
    if (!mut_instance) {
        return false;
    }
    
    return ar_methodology__unregister_method_with_instance(mut_instance, ref_name, ref_version);
}

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
    
    return own_methodology;
}

/**
 * Destroy a methodology instance
 * @param own_methodology The methodology instance to destroy (takes ownership)
 * @note Ownership: Takes ownership and destroys the instance
 */
void ar_methodology__destroy(ar_methodology_t *own_methodology) {
    if (own_methodology) {
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

ar_method_t* ar_methodology__get_method_with_instance(ar_methodology_t *ref_methodology, 
                                                   const char *ref_name, 
                                                   const char *ref_version) {
    if (!ref_methodology) {
        return NULL;
    }
    
    // Use the resolver for all version resolution
    return ar_method_resolver__resolve_method(ref_methodology->own_resolver, ref_name, ref_version);
}

void ar_methodology__register_method_with_instance(ar_methodology_t *mut_methodology, 
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

bool ar_methodology__create_method_with_instance(ar_methodology_t *mut_methodology,
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
    
    ar_methodology__register_method_with_instance(mut_methodology, own_method);
    // ownership transferred
    
    return true;
}

bool ar_methodology__save_methods_with_instance(ar_methodology_t *ref_methodology, 
                                                const char *ref_filename) {
    if (!ref_methodology || !ref_filename) {
        return false;
    }
    
    FILE *fp;
    ar_file_result_t result = ar_io__open_file(ref_filename, "w", &fp);
    if (result != AR_FILE_RESULT__SUCCESS) {
        ar_io__error("Failed to open file %s for writing: %s", ref_filename, 
                    ar_io__error_message(result));
        return false;
    }
    
    // Buffer for formatted output
    char buffer[BUFFER_SIZE];
    
    // Get all methods from registry
    ar_list_t *own_all_methods = ar_method_registry__get_all_methods(ref_methodology->own_registry);
    if (!own_all_methods) {
        ar_io__close_file(fp, ref_filename);
        return false;
    }
    
    // Build a map of method names to count versions
    typedef struct {
        const char *name;
        int count;
        ar_method_t **methods;
    } method_group_t;
    
    method_group_t *groups = NULL;
    int group_count = 0;
    int group_capacity = 10;
    
    groups = AR__HEAP__MALLOC(sizeof(method_group_t) * (size_t)group_capacity, "method groups");
    if (!groups) {
        ar_list__destroy(own_all_methods);
        ar_io__close_file(fp, ref_filename);
        return false;
    }
    
    // Group methods by name
    void **own_items = ar_list__items(own_all_methods);
    size_t method_count = ar_list__count(own_all_methods);
    
    for (size_t i = 0; i < method_count; i++) {
        ar_method_t *method = (ar_method_t*)own_items[i];
        const char *name = ar_method__get_name(method);
        
        // Find or create group
        int group_idx = -1;
        for (int j = 0; j < group_count; j++) {
            if (strcmp(groups[j].name, name) == 0) {
                group_idx = j;
                break;
            }
        }
        
        if (group_idx < 0) {
            // New group
            if (group_count >= group_capacity) {
                // Grow groups array
                group_capacity *= 2;
                method_group_t *new_groups = AR__HEAP__REALLOC(groups, 
                    sizeof(method_group_t) * (size_t)group_capacity, "method groups");
                if (!new_groups) {
                    AR__HEAP__FREE(own_items);
                    ar_list__destroy(own_all_methods);
                    AR__HEAP__FREE(groups);
                    ar_io__close_file(fp, ref_filename);
                    return false;
                }
                groups = new_groups;
            }
            
            group_idx = group_count++;
            groups[group_idx].name = name;
            groups[group_idx].count = 0;
            groups[group_idx].methods = AR__HEAP__MALLOC(
                sizeof(ar_method_t*) * MAX_VERSIONS_PER_METHOD, "method versions");
            if (!groups[group_idx].methods) {
                AR__HEAP__FREE(own_items);
                ar_list__destroy(own_all_methods);
                for (int k = 0; k < group_count - 1; k++) {
                    AR__HEAP__FREE(groups[k].methods);
                }
                AR__HEAP__FREE(groups);
                ar_io__close_file(fp, ref_filename);
                return false;
            }
        }
        
        // Add method to group
        groups[group_idx].methods[groups[group_idx].count++] = method;
    }
    
    // Write method count (unique names)
    int written = snprintf(buffer, BUFFER_SIZE, "%d\n", group_count);
    if (written < 0 || written >= (int)BUFFER_SIZE || fputs(buffer, fp) == EOF) {
        ar_io__error("Failed to write method count to %s", ref_filename);
        AR__HEAP__FREE(own_items);
        ar_list__destroy(own_all_methods);
        for (int i = 0; i < group_count; i++) {
            AR__HEAP__FREE(groups[i].methods);
        }
        AR__HEAP__FREE(groups);
        ar_io__close_file(fp, ref_filename);
        return false;
    }
    
    // Write each method group
    for (int i = 0; i < group_count; i++) {
        // Write method name and version count
        written = snprintf(buffer, BUFFER_SIZE, "%s %d\n", groups[i].name, groups[i].count);
        if (written < 0 || written >= (int)BUFFER_SIZE || fputs(buffer, fp) == EOF) {
            ar_io__error("Failed to write method header to %s", ref_filename);
            AR__HEAP__FREE(own_items);
            ar_list__destroy(own_all_methods);
            for (int j = 0; j < group_count; j++) {
                AR__HEAP__FREE(groups[j].methods);
            }
            AR__HEAP__FREE(groups);
            ar_io__close_file(fp, ref_filename);
            return false;
        }
        
        // Write each version
        for (int j = 0; j < groups[i].count; j++) {
            ar_method_t *method = groups[i].methods[j];
            const char *version = ar_method__get_version(method);
            const char *instructions = ar_method__get_instructions(method);
            
            // Write version
            written = snprintf(buffer, BUFFER_SIZE, "%s\n", version);
            if (written < 0 || written >= (int)BUFFER_SIZE || fputs(buffer, fp) == EOF) {
                ar_io__error("Failed to write version to %s", ref_filename);
                AR__HEAP__FREE(own_items);
                ar_list__destroy(own_all_methods);
                for (int k = 0; k < group_count; k++) {
                    AR__HEAP__FREE(groups[k].methods);
                }
                AR__HEAP__FREE(groups);
                ar_io__close_file(fp, ref_filename);
                return false;
            }
            
            // Write instructions
            written = snprintf(buffer, BUFFER_SIZE, "%s\n", instructions ? instructions : "");
            if (written < 0 || written >= (int)BUFFER_SIZE || fputs(buffer, fp) == EOF) {
                ar_io__error("Failed to write instructions to %s", ref_filename);
                AR__HEAP__FREE(own_items);
                ar_list__destroy(own_all_methods);
                for (int k = 0; k < group_count; k++) {
                    AR__HEAP__FREE(groups[k].methods);
                }
                AR__HEAP__FREE(groups);
                ar_io__close_file(fp, ref_filename);
                return false;
            }
        }
    }
    
    // Clean up
    AR__HEAP__FREE(own_items);
    ar_list__destroy(own_all_methods);
    for (int i = 0; i < group_count; i++) {
        AR__HEAP__FREE(groups[i].methods);
    }
    AR__HEAP__FREE(groups);
    
    ar_io__close_file(fp, ref_filename);
    return true;
}

bool ar_methodology__load_methods_with_instance(ar_methodology_t *mut_methodology, 
                                                const char *ref_filename) {
    if (!mut_methodology || !ref_filename) {
        return false;
    }
    
    FILE *fp;
    ar_file_result_t result = ar_io__open_file(ref_filename, "r", &fp);
    if (result != AR_FILE_RESULT__SUCCESS) {
        if (result != AR_FILE_RESULT__ERROR_NOT_FOUND) {
            ar_io__error("Failed to open file %s for reading: %s", ref_filename,
                        ar_io__error_message(result));
        }
        return false;
    }
    
    char line[LINE_SIZE] = {0};
    char *end_ptr = NULL;
    
    // Read method count
    if (!ar_io__read_line(fp, line, (int)sizeof(line), ref_filename)) {
        ar_io__close_file(fp, ref_filename);
        return false;
    }
    
    errno = 0;
    long method_count = strtol(line, &end_ptr, 10);
    if (errno != 0 || end_ptr == line || method_count < 0 || method_count > MAX_METHODS) {
        ar_io__error("Invalid method count in %s: %s", ref_filename, line);
        ar_io__close_file(fp, ref_filename);
        return false;
    }
    
    // Load each method
    for (int i = 0; i < method_count; i++) {
        // Read method name and version count
        if (!ar_io__read_line(fp, line, (int)sizeof(line), ref_filename)) {
            ar_io__error("Failed to read method entry %d from %s", i+1, ref_filename);
            ar_io__close_file(fp, ref_filename);
            return false;
        }
        
        char method_name[MAX_METHOD_NAME_LENGTH] = {0};
        int version_count = 0;
        if (sscanf(line, "%255s %d", method_name, &version_count) != 2) {
            ar_io__error("Malformed method entry in %s: %s", ref_filename, line);
            ar_io__close_file(fp, ref_filename);
            return false;
        }
        
        // Load each version
        for (int v = 0; v < version_count; v++) {
            // Read version
            if (!ar_io__read_line(fp, line, (int)sizeof(line), ref_filename)) {
                ar_io__error("Failed to read version for method %s", method_name);
                ar_io__close_file(fp, ref_filename);
                return false;
            }
            char *version = AR__HEAP__STRDUP(ar_string__trim(line), "method version");
            if (!version) {
                ar_io__close_file(fp, ref_filename);
                return false;
            }
            
            // Read instructions
            if (!ar_io__read_line(fp, line, (int)sizeof(line), ref_filename)) {
                AR__HEAP__FREE(version);
                ar_io__close_file(fp, ref_filename);
                return false;
            }
            
            // Instructions can be empty, so we don't check for that
            // Trim to remove newline
            char *instructions = AR__HEAP__STRDUP(ar_string__trim(line), "method instructions");
            if (!instructions) {
                AR__HEAP__FREE(version);
                ar_io__close_file(fp, ref_filename);
                return false;
            }
            
            // Create and register the method
            ar_method_t *method = ar_method__create_with_log(method_name, instructions, version, mut_methodology->ref_log);
            AR__HEAP__FREE(version);
            AR__HEAP__FREE(instructions);
            
            if (method) {
                ar_methodology__register_method_with_instance(mut_methodology, method);
            }
        }
    }
    
    ar_io__close_file(fp, ref_filename);
    return true;
}

bool ar_methodology__unregister_method_with_instance(ar_methodology_t *mut_methodology,
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

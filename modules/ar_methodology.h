#ifndef AGERUN_METHODOLOGY_H
#define AGERUN_METHODOLOGY_H

#include <stdbool.h>
#include "ar_method.h"

/* Constants */
#define METHODOLOGY_FILE_NAME "methodology.agerun"

/**
 * Creates a new method object and registers it with the methodology module
 * @param ref_name Method name (borrowed reference)
 * @param ref_instructions The method implementation code (borrowed reference)
 * @param ref_version Semantic version string for this method (e.g., "1.0.0")
 * @return true if method was created and registered successfully, false otherwise
 * @note Ownership: This function creates and takes ownership of the method.
 *       The caller should not worry about destroying the method.
 */
bool ar__methodology__create_method(const char *ref_name, const char *ref_instructions, 
                                const char *ref_version);

/**
 * Get a method definition by name and version
 * @param ref_name Method name (borrowed reference)
 * @param ref_version Method version string (NULL for latest)
 * @return Pointer to method definition (borrowed reference), or NULL if not found
 * @note Ownership: Returns a borrowed reference to the internal method. The caller
 *       should not modify or free the returned method.
 */
method_t* ar__methodology__get_method(const char *ref_name, const char *ref_version);


/* Internal functions removed to comply with Parnas principles */

/**
 * Save all method definitions to disk
 * @return true if successful, false otherwise
 */
bool ar__methodology__save_methods(void);

/**
 * Load all method definitions from disk
 * @return true if successful, false otherwise
 */
bool ar__methodology__load_methods(void);

/**
 * Clean up all method definitions and free resources
 * This should be called during system shutdown
 */
void ar__methodology__cleanup(void);

/**
 * Register a method with the methodology module
 * @param own_method The method to register (ownership is transferred to methodology)
 * @note Ownership: The methodology module takes ownership of the method.
 *       The caller should not access or free the method after this call.
 */
void ar__methodology__register_method(method_t *own_method);

/**
 * Unregister a method from the methodology
 * @param ref_name Method name (borrowed reference)
 * @param ref_version Version string of the method to unregister
 * @return true if method was successfully unregistered, false otherwise
 * @note This will fail if there are active agents using this method
 */
bool ar__methodology__unregister_method(const char *ref_name, const char *ref_version);

#endif /* AGERUN_METHODOLOGY_H */

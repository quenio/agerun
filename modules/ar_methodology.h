#ifndef AGERUN_METHODOLOGY_H
#define AGERUN_METHODOLOGY_H

#include <stdbool.h>
#include "ar_method.h"
#include "ar_log.h"

/* Constants */
#define METHODOLOGY_FILE_NAME "agerun.methodology"

/* Opaque type for methodology instance */
typedef struct ar_methodology_s ar_methodology_t;

/* ar_methodology__create_method removed - use ar_methodology__create_method instead */

/* ar_methodology__get_method removed - use ar_methodology__get_method instead */


/* Internal functions removed to comply with Parnas principles */

/* ar_methodology__save_methods removed - use ar_methodology__save_methods instead */

/* ar_methodology__load_methods removed - use ar_methodology__load_methods instead */

/* ar_methodology__cleanup removed - use ar_methodology__cleanup instead */

/* ar_methodology__register_method removed - use ar_methodology__register_method instead */

/* ar_methodology__unregister_method removed - use ar_methodology__unregister_method instead */

/**
 * Create a new methodology instance
 * @param ref_log Log instance for error reporting (borrowed reference, may be NULL)
 * @return New methodology instance or NULL on allocation failure
 * @note Ownership: Caller owns the returned instance and must destroy it
 */
ar_methodology_t* ar_methodology__create(ar_log_t *ref_log);

/**
 * Destroy a methodology instance
 * @param own_methodology The methodology instance to destroy (takes ownership)
 * @note Ownership: Takes ownership and destroys the instance
 */
void ar_methodology__destroy(ar_methodology_t *own_methodology);

/* Instance-aware versions of public functions */

/**
 * Get a method definition by name and version using a specific instance
 * @param ref_methodology The methodology instance to search in (borrowed reference)
 * @param ref_name Method name (borrowed reference)
 * @param ref_version Method version string (NULL for latest)
 * @return Pointer to method definition (borrowed reference), or NULL if not found
 * @note Ownership: Returns a borrowed reference to the internal method
 */
ar_method_t* ar_methodology__get_method(ar_methodology_t *ref_methodology, 
                                                   const char *ref_name, 
                                                   const char *ref_version);

/**
 * Register a method with a specific methodology instance
 * @param mut_methodology The methodology instance to register to (mutable reference)
 * @param own_method The method to register (ownership is transferred)
 * @note Ownership: The methodology instance takes ownership of the method
 */
void ar_methodology__register_method(ar_methodology_t *mut_methodology, 
                                                   ar_method_t *own_method);

/**
 * Create and register a method with a specific methodology instance
 * @param mut_methodology The methodology instance to use (mutable reference)
 * @param ref_name Method name (borrowed reference)
 * @param ref_instructions The method implementation code (borrowed reference)
 * @param ref_version Semantic version string for this method (e.g., "1.0.0")
 * @return true if method was created and registered successfully, false otherwise
 * @note Ownership: This function creates and takes ownership of the method
 */
bool ar_methodology__create_method(ar_methodology_t *mut_methodology,
                                                 const char *ref_name, 
                                                 const char *ref_instructions,
                                                 const char *ref_version);

/**
 * Save methods from a specific instance to disk
 * @param ref_methodology The methodology instance to save (borrowed reference)
 * @param ref_filename The filename to save to (borrowed reference)
 * @return true if successful, false otherwise
 */
bool ar_methodology__save_methods(ar_methodology_t *ref_methodology, 
                                                const char *ref_filename);

/**
 * Load methods from disk into a specific instance
 * @param mut_methodology The methodology instance to load into (mutable reference)
 * @param ref_filename The filename to load from (borrowed reference)
 * @return true if successful, false otherwise
 */
bool ar_methodology__load_methods(ar_methodology_t *mut_methodology, 
                                                const char *ref_filename);

/**
 * Unregister a method from a specific methodology instance
 * @param mut_methodology The methodology instance to unregister from (mutable reference)
 * @param ref_name Method name (borrowed reference)
 * @param ref_version Version string of the method to unregister
 * @return true if method was successfully unregistered, false otherwise
 * @note This will fail if there are active agents using this method
 */
bool ar_methodology__unregister_method(ar_methodology_t *mut_methodology,
                                                     const char *ref_name,
                                                     const char *ref_version);

/**
 * Clean up all method definitions and free resources in a specific methodology instance
 * @param mut_methodology The methodology instance to cleanup (mutable reference)
 * @note This should be called before destroying the methodology instance
 * @note Ownership: This cleans up all methods owned by the instance
 */
void ar_methodology__cleanup(ar_methodology_t *mut_methodology);

#endif /* AGERUN_METHODOLOGY_H */

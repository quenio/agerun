#ifndef AGERUN_METHODOLOGY_H
#define AGERUN_METHODOLOGY_H

#include <stdbool.h>
#include "ar_method.h"
#include "ar_log.h"

/* Constants */
#define METHODOLOGY_FILE_NAME "methodology.agerun"

/* Opaque type for methodology instance */
typedef struct ar_methodology_s ar_methodology_t;

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
                                const char *ref_version);

/**
 * Get a method definition by name and version
 * @param ref_name Method name (borrowed reference)
 * @param ref_version Method version string (NULL for latest)
 * @return Pointer to method definition (borrowed reference), or NULL if not found
 * @note Ownership: Returns a borrowed reference to the internal method. The caller
 *       should not modify or free the returned method.
 */
ar_method_t* ar_methodology__get_method(const char *ref_name, const char *ref_version);


/* Internal functions removed to comply with Parnas principles */

/**
 * Save all method definitions to disk
 * @return true if successful, false otherwise
 */
bool ar_methodology__save_methods(void);

/**
 * Load all method definitions from disk
 * @return true if successful, false otherwise
 */
bool ar_methodology__load_methods(void);

/**
 * Clean up all method definitions and free resources
 * This should be called during system shutdown
 */
void ar_methodology__cleanup(void);

/**
 * Register a method with the methodology module
 * @param own_method The method to register (ownership is transferred to methodology)
 * @note Ownership: The methodology module takes ownership of the method.
 *       The caller should not access or free the method after this call.
 */
void ar_methodology__register_method(ar_method_t *own_method);

/**
 * Unregister a method from the methodology
 * @param ref_name Method name (borrowed reference)
 * @param ref_version Version string of the method to unregister
 * @return true if method was successfully unregistered, false otherwise
 * @note This will fail if there are active agents using this method
 */
bool ar_methodology__unregister_method(const char *ref_name, const char *ref_version);

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
ar_method_t* ar_methodology__get_method_with_instance(ar_methodology_t *ref_methodology, 
                                                   const char *ref_name, 
                                                   const char *ref_version);

/**
 * Register a method with a specific methodology instance
 * @param mut_methodology The methodology instance to register to (mutable reference)
 * @param own_method The method to register (ownership is transferred)
 * @note Ownership: The methodology instance takes ownership of the method
 */
void ar_methodology__register_method_with_instance(ar_methodology_t *mut_methodology, 
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
bool ar_methodology__create_method_with_instance(ar_methodology_t *mut_methodology,
                                                 const char *ref_name, 
                                                 const char *ref_instructions,
                                                 const char *ref_version);

/**
 * Save methods from a specific instance to disk
 * @param ref_methodology The methodology instance to save (borrowed reference)
 * @param ref_filename The filename to save to (borrowed reference)
 * @return true if successful, false otherwise
 */
bool ar_methodology__save_methods_with_instance(ar_methodology_t *ref_methodology, 
                                                const char *ref_filename);

/**
 * Load methods from disk into a specific instance
 * @param mut_methodology The methodology instance to load into (mutable reference)
 * @param ref_filename The filename to load from (borrowed reference)
 * @return true if successful, false otherwise
 */
bool ar_methodology__load_methods_with_instance(ar_methodology_t *mut_methodology, 
                                                const char *ref_filename);

/**
 * Unregister a method from a specific methodology instance
 * @param mut_methodology The methodology instance to unregister from (mutable reference)
 * @param ref_name Method name (borrowed reference)
 * @param ref_version Version string of the method to unregister
 * @return true if method was successfully unregistered, false otherwise
 * @note This will fail if there are active agents using this method
 */
bool ar_methodology__unregister_method_with_instance(ar_methodology_t *mut_methodology,
                                                     const char *ref_name,
                                                     const char *ref_version);

#endif /* AGERUN_METHODOLOGY_H */

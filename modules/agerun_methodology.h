#ifndef AGERUN_METHODOLOGY_H
#define AGERUN_METHODOLOGY_H

#include <stdbool.h>
#include "agerun_method.h"

/* Constants */
#define METHODOLOGY_FILE_NAME "methodology.agerun"

/**
 * Creates a new method object and registers it with the methodology module
 * @param ref_name Method name (borrowed reference)
 * @param ref_instructions The method implementation code (borrowed reference)
 * @param version The version number for this method (pass 0 to auto-increment from previous_version)
 * @param previous_version Previous version number (0 for first version)
 * @param backward_compatible Whether the method is backward compatible
 * @param persist Whether agents using this method should persist
 * @return true if method was created and registered successfully, false otherwise
 * @note Ownership: This function creates and takes ownership of the method.
 *       The caller should not worry about destroying the method.
 */
bool ar_methodology_create_method(const char *ref_name, const char *ref_instructions, 
                              version_t version, version_t previous_version, 
                              bool backward_compatible, bool persist);

/**
 * Get a method definition by name and version
 * @param ref_name Method name (borrowed reference)
 * @param version Method version (0 for latest)
 * @return Pointer to method definition (borrowed reference), or NULL if not found
 * @note Ownership: Returns a borrowed reference to the internal method. The caller
 *       should not modify or free the returned method.
 */
method_t* ar_methodology_get_method(const char *ref_name, version_t version);

/**
 * Find the index of a method by name in the methods array
 * @param ref_name Method name to search for (borrowed reference)
 * @return Index of the method, or -1 if not found
 */
int ar_methodology_find_method_idx(const char *ref_name);

/**
 * Get a pointer to a specific method storage location
 * @param method_idx Index of the method in the methods array
 * @param version_idx Index of the version for this method
 * @return Pointer to the method storage location (mutable reference)
 * @note Ownership: Returns a mutable reference to internal storage.
 *       The caller should not free the returned method.
 */
method_t* ar_methodology_get_method_storage(int method_idx, int version_idx);

/**
 * Set a method pointer in the method storage location
 * @param method_idx Index of the method in the methods array
 * @param version_idx Index of the version for this method
 * @param ref_method Method pointer to store (methodology takes ownership)
 * @note Ownership: Methodology takes ownership of the method pointer.
 *       The caller should not use or free the method after this call.
 */
void ar_methodology_set_method_storage(int method_idx, int version_idx, method_t *ref_method);

/**
 * Get a pointer to the array of method counts
 * @return Pointer to the array of method counts (mutable reference)
 * @note Ownership: Returns a mutable reference to internal storage.
 *       The caller should not free the returned array.
 */
int* ar_methodology_get_method_counts(void);

/**
 * Get a pointer to the method name count variable
 * @return Pointer to the method name count (mutable reference)
 * @note Ownership: Returns a mutable reference to internal storage.
 *       The caller should not free the returned pointer.
 */
int* ar_methodology_get_method_name_count(void);

/**
 * Save all method definitions to disk
 * @return true if successful, false otherwise
 */
bool ar_methodology_save_methods(void);

/**
 * Load all method definitions from disk
 * @return true if successful, false otherwise
 */
bool ar_methodology_load_methods(void);

/**
 * Clean up all method definitions and free resources
 * This should be called during system shutdown
 */
void ar_methodology_cleanup(void);

/**
 * Register a method with the methodology module
 * @param own_method The method to register (ownership is transferred to methodology)
 * @note Ownership: The methodology module takes ownership of the method.
 *       The caller should not access or free the method after this call.
 */
void ar_methodology_register_method(method_t *own_method);

#endif /* AGERUN_METHODOLOGY_H */

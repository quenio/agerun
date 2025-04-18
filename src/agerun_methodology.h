#ifndef AGERUN_METHODOLOGY_H
#define AGERUN_METHODOLOGY_H

#include <stdbool.h>
#include "agerun_method.h"

/* Constants */
#define METHODOLOGY_FILE_NAME "methodology.agerun"

/**
 * Get a method definition by name and version
 * @param name Method name
 * @param version Method version (0 for latest)
 * @return Pointer to method definition, or NULL if not found
 */
method_t* ar_methodology_get_method(const char *name, version_t version);

/**
 * Find the index of a method by name in the methods array
 * @param name Method name to search for
 * @return Index of the method, or -1 if not found
 */
int ar_methodology_find_method_idx(const char *name);

/**
 * Get a pointer to a specific method storage location
 * @param method_idx Index of the method in the methods array
 * @param version_idx Index of the version for this method
 * @return Pointer to the method storage location
 */
method_t* ar_methodology_get_method_storage(int method_idx, int version_idx);

/**
 * Get a pointer to the array of method counts
 * @return Pointer to the array of method counts
 */
int* ar_methodology_get_method_counts(void);

/**
 * Get a pointer to the method name count variable
 * @return Pointer to the method name count
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

#endif /* AGERUN_METHODOLOGY_H */

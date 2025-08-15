#ifndef AR_YAML_H
#define AR_YAML_H

#include <stdbool.h>

/* Forward declarations */
typedef struct ar_data_s ar_data_t;

/**
 * @file ar_yaml.h
 * @brief YAML file I/O for ar_data_t structures
 * 
 * This module provides functions to write ar_data_t structures to YAML files
 * and read them back, using direct streaming to avoid memory duplication.
 * It supports maps, lists, and scalar values (strings, integers, doubles).
 */

/**
 * Write ar_data_t structure directly to YAML file
 * @param ref_data The data structure to write (borrowed reference)
 * @param ref_filename The output filename (borrowed reference)
 * @return true on success, false on error
 */
bool ar_yaml__write_to_file(const ar_data_t *ref_data, const char *ref_filename);

/**
 * Read YAML file into ar_data_t structure
 * @param ref_filename The input filename (borrowed reference)
 * @return New ar_data_t structure (ownership transferred), or NULL on error
 * @note Ownership: Caller must destroy the returned ar_data_t
 */
ar_data_t* ar_yaml__read_from_file(const char *ref_filename);

#endif /* AR_YAML_H */
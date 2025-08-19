#ifndef AR_YAML_READER_H
#define AR_YAML_READER_H

#include "ar_data.h"
#include "ar_log.h"

/* Opaque type */
typedef struct ar_yaml_reader_s ar_yaml_reader_t;

/**
 * @file ar_yaml_reader.h
 * @brief YAML file reading into ar_data_t structures
 * 
 * This module provides functions to read YAML files into ar_data_t structures,
 * supporting maps, lists, and scalar values (strings, integers, doubles).
 */

/**
 * Create a new YAML reader instance
 * @param ref_log Log instance for error reporting (borrowed reference, may be NULL)
 * @return New reader instance or NULL on allocation failure
 * @note Ownership: Caller owns the returned instance and must destroy it
 */
ar_yaml_reader_t* ar_yaml_reader__create(ar_log_t *ref_log);

/**
 * Destroy a YAML reader instance
 * @param own_reader The reader instance to destroy (takes ownership)
 * @note Ownership: Takes ownership and destroys the instance
 */
void ar_yaml_reader__destroy(ar_yaml_reader_t *own_reader);

/**
 * Read YAML file into ar_data_t structure using instance
 * @param mut_reader The reader instance to use (mutable reference)
 * @param ref_filename The input filename (borrowed reference)
 * @return New ar_data_t structure (ownership transferred), or NULL on error
 * @note Ownership: Caller must destroy the returned ar_data_t
 */
ar_data_t* ar_yaml_reader__read_from_file(ar_yaml_reader_t *mut_reader, const char *ref_filename);

#endif /* AR_YAML_READER_H */
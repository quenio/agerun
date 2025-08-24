#ifndef AR_YAML_WRITER_H
#define AR_YAML_WRITER_H

#include <stdbool.h>
#include "ar_data.h"
#include "ar_log.h"

/**
 * @file ar_yaml_writer.h
 * @brief YAML file writing for ar_data_t structures
 * 
 * This module provides functions to write ar_data_t structures to YAML files
 * using direct streaming to avoid memory duplication.
 * It supports maps, lists, and scalar values (strings, integers, doubles).
 */

/* Opaque writer type */
typedef struct ar_yaml_writer_s ar_yaml_writer_t;

/**
 * Create a new YAML writer instance
 * @param ref_log The log instance for error reporting (borrowed reference, can be NULL)
 * @return New writer instance (ownership transferred), or NULL on error
 * @note Ownership: Caller must destroy the returned writer
 */
ar_yaml_writer_t* ar_yaml_writer__create(ar_log_t *ref_log);

/**
 * Destroy a YAML writer instance
 * @param own_writer The writer instance to destroy (takes ownership)
 * @note Ownership: Takes ownership and destroys the instance
 */
void ar_yaml_writer__destroy(ar_yaml_writer_t *own_writer);

/**
 * Write ar_data_t structure to YAML file
 * @param mut_writer The writer instance to use (mutable reference)
 * @param ref_data The data structure to write (borrowed reference)
 * @param ref_filename The output filename (borrowed reference)
 * @return true on success, false on error
 * @note Ownership: Writer is borrowed, data and filename are borrowed
 */
bool ar_yaml_writer__write_to_file(ar_yaml_writer_t *mut_writer, 
                                    const ar_data_t *ref_data, 
                                    const char *ref_filename);


#endif /* AR_YAML_WRITER_H */
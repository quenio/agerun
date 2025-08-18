#ifndef AR_YAML_WRITER_H
#define AR_YAML_WRITER_H

#include <stdbool.h>
#include "ar_data.h"

/**
 * @file ar_yaml_writer.h
 * @brief YAML file writing for ar_data_t structures
 * 
 * This module provides functions to write ar_data_t structures to YAML files
 * using direct streaming to avoid memory duplication.
 * It supports maps, lists, and scalar values (strings, integers, doubles).
 */

/**
 * Write ar_data_t structure directly to YAML file
 * @param ref_data The data structure to write (borrowed reference)
 * @param ref_filename The output filename (borrowed reference)
 * @return true on success, false on error
 */
bool ar_yaml_writer__write_to_file(const ar_data_t *ref_data, const char *ref_filename);

#endif /* AR_YAML_WRITER_H */
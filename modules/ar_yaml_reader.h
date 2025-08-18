#ifndef AR_YAML_READER_H
#define AR_YAML_READER_H

#include "ar_data.h"

/**
 * @file ar_yaml_reader.h
 * @brief YAML file reading into ar_data_t structures
 * 
 * This module provides functions to read YAML files into ar_data_t structures,
 * supporting maps, lists, and scalar values (strings, integers, doubles).
 */

/**
 * Read YAML file into ar_data_t structure
 * @param ref_filename The input filename (borrowed reference)
 * @return New ar_data_t structure (ownership transferred), or NULL on error
 * @note Ownership: Caller must destroy the returned ar_data_t
 */
ar_data_t* ar_yaml_reader__read_from_file(const char *ref_filename);

#endif /* AR_YAML_READER_H */
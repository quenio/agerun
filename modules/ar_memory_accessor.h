#ifndef AR_MEMORY_ACCESSOR_H
#define AR_MEMORY_ACCESSOR_H

/**
 * @file ar_memory_accessor.h
 * @brief Memory accessor module for AgeRun
 *
 * This module provides utilities for accessing memory paths in the AgeRun system.
 * It centralizes the logic for checking if a path is a memory path and extracting
 * the key portion after the "memory." prefix.
 */

/**
 * Gets the memory key from a path if it's a memory path.
 * For example, given "memory.user.name", returns "user.name".
 * 
 * @param ref_path The path to check
 * @return BORROW: The key portion after "memory.", or NULL if not a memory path
 * @note The returned pointer points into the original string and should not be freed
 */
const char* ar_memory_accessor__get_key(const char *ref_path);

#endif /* AR_MEMORY_ACCESSOR_H */
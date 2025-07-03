#ifndef AGERUN_PATH_H
#define AGERUN_PATH_H

#include <stddef.h>  /* For size_t */
#include <stdbool.h> /* For bool */

/**
 * @file ar_path.h
 * @brief Path manipulation module for AgeRun
 *
 * This module provides an instantiable path abstraction for working with
 * various types of paths in the AgeRun system, including variable paths
 * (memory.x, context.y), file paths (/path/to/file), and generic paths.
 */

/* Opaque path type */
typedef struct ar_path_s ar_path_t;

/**
 * Creates a path object with a custom separator.
 *
 * @param ref_path The path string to parse
 * @param separator The character to use as path separator
 * @return OWNER: New path object, or NULL on error. Caller must destroy.
 */
ar_path_t* ar_path__create(const char *ref_path, char separator);

/**
 * Creates a variable path object (uses '.' as separator).
 * Used for paths like memory.x, context.y, message.z
 *
 * @param ref_path The variable path string
 * @return OWNER: New path object, or NULL on error. Caller must destroy.
 */
ar_path_t* ar_path__create_variable(const char *ref_path);

/**
 * Creates a file path object (uses '/' as separator).
 *
 * @param ref_path The file path string
 * @return OWNER: New path object, or NULL on error. Caller must destroy.
 */
ar_path_t* ar_path__create_file(const char *ref_path);

/**
 * Destroys a path object and frees its memory.
 *
 * @param own_path The path object to destroy (takes ownership)
 */
void ar_path__destroy(ar_path_t *own_path);

/**
 * Gets the original path string.
 *
 * @param ref_path The path object
 * @return BORROW: The original path string, or NULL if ref_path is NULL
 */
const char* ar_path__get_string(const ar_path_t *ref_path);

/**
 * Gets the separator character used by this path.
 *
 * @param ref_path The path object
 * @return The separator character, or '\0' if ref_path is NULL
 */
char ar_path__get_separator(const ar_path_t *ref_path);

/**
 * Gets the number of segments in the path.
 *
 * @param ref_path The path object
 * @return Number of segments, or 0 if ref_path is NULL or empty
 */
size_t ar_path__get_segment_count(const ar_path_t *ref_path);

/**
 * Gets a segment from the path by index.
 *
 * @param ref_path The path object
 * @param index The zero-based index of the segment
 * @return BORROW: The segment string, or NULL if index is out of bounds
 */
const char* ar_path__get_segment(const ar_path_t *ref_path, size_t index);

/**
 * Gets a copy of a segment from the path by index.
 *
 * @param ref_path The path object
 * @param index The zero-based index of the segment
 * @return OWNER: A copy of the segment, or NULL if index is out of bounds. Caller must free.
 */
char* ar_path__get_segment_copy(const ar_path_t *ref_path, size_t index);

/**
 * Gets the parent path (all segments except the last).
 *
 * @param ref_path The path object
 * @return OWNER: A new path object representing the parent, or NULL if no parent exists.
 *         Caller must destroy.
 */
ar_path_t* ar_path__get_parent(const ar_path_t *ref_path);

/**
 * Checks if the path starts with the given prefix.
 * 
 * @param ref_path The path object to check
 * @param ref_prefix The prefix string to check for
 * @return true if the path starts with the prefix, false otherwise
 */
bool ar_path__starts_with(const ar_path_t *ref_path, const char *ref_prefix);

/**
 * Gets the variable root (first segment) of the path.
 * For paths like "memory.user.name", returns "memory".
 * 
 * @param ref_path The path object
 * @return BORROW: The root segment, or NULL if path is NULL or empty
 */
const char* ar_path__get_variable_root(const ar_path_t *ref_path);

/**
 * Checks if this is a memory path (starts with "memory").
 * 
 * @param ref_path The path object to check
 * @return true if this is a memory path, false otherwise
 */
bool ar_path__is_memory_path(const ar_path_t *ref_path);

/**
 * Checks if this is a context path (starts with "context").
 * 
 * @param ref_path The path object to check
 * @return true if this is a context path, false otherwise
 */
bool ar_path__is_context_path(const ar_path_t *ref_path);

/**
 * Checks if this is a message path (starts with "message").
 * 
 * @param ref_path The path object to check
 * @return true if this is a message path, false otherwise
 */
bool ar_path__is_message_path(const ar_path_t *ref_path);

/**
 * Joins a base path with an additional path string.
 * 
 * @param ref_base The base path object
 * @param ref_suffix The suffix path string to append
 * @return OWNER: A new path object with the joined path, or NULL on error.
 *         Caller must destroy.
 */
ar_path_t* ar_path__join(const ar_path_t *ref_base, const char *ref_suffix);

/**
 * Normalizes a path by removing empty segments (caused by multiple separators).
 * For example, "memory..user" becomes "memory.user".
 * 
 * @param ref_path The path object to normalize
 * @return OWNER: A new normalized path object, or NULL on error.
 *         Caller must destroy.
 */
ar_path_t* ar_path__normalize(const ar_path_t *ref_path);

#endif /* AGERUN_PATH_H */
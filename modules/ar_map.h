#ifndef AGERUN_MAP_H
#define AGERUN_MAP_H

#include <stdbool.h>
#include <stddef.h>

/**
 * A key-value mapping structure that associates string keys with pointer values.
 * The map never owns or manages the memory for keys or values.
 * The caller is always responsible for allocating and freeing memory for both keys and values.
 */
typedef struct map_s ar_map_t;

/**
 * Create a new heap-allocated empty map
 * @return Pointer to the new map, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
ar_map_t* ar_map__create(void);

/**
 * Get a reference from map by key
 * @param ref_map The map to look up in (borrowed reference)
 * @param ref_key The key to lookup (borrowed reference)
 * @return Pointer to the referenced value, or NULL if not found
 * @note Ownership: Returns a borrowed reference. Caller does not own the returned value.
 */
void* ar_map__get(const ar_map_t *ref_map, const char *ref_key);

/**
 * Set a reference in map
 * @param mut_map The map to modify (mutable reference)
 * @param ref_key The key to set (borrowed reference, not copied)
 * @param ref_value The pointer to reference (borrowed reference)
 * @return true if successful, false otherwise
 * @note Ownership: The map does NOT take ownership of the key or value.
 *       The caller remains responsible for allocating and freeing the key string.
 *       The key string must remain valid for the lifetime of the map entry.
 */
bool ar_map__set(ar_map_t *mut_map, const char *ref_key, void *ref_value);

/**
 * Get the number of used entries in the map
 * @param ref_map The map to count (borrowed reference)
 * @return The number of used entries
 */
size_t ar_map__count(const ar_map_t *ref_map);

/**
 * Get an array of all refs in the map
 * @param ref_map The map to get refs from (borrowed reference)
 * @return Array of pointers to refs, or NULL on failure
 * @note Ownership: Returns an owned array that caller must free.
 *       The caller is responsible for freeing the returned array using free().
 *       The refs themselves are borrowed references and remain owned by their original owners.
 *       The caller can use ar_map_count() to determine the size of the array.
 */
void** ar_map__refs(const ar_map_t *ref_map);

/**
 * Free all resources in a map
 * @param own_map Map to free (owned value)
 * @note Ownership: Destroys the map structure itself.
 *       It does not free memory for keys or referenced values.
 *       The caller remains responsible for freeing all keys and values that were added to the map.
 */
void ar_map__destroy(ar_map_t *own_map);


#endif /* AGERUN_MAP_H */

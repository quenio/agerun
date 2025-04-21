#ifndef AGERUN_MAP_H
#define AGERUN_MAP_H

#include <stdbool.h>
#include <stddef.h>

/**
 * A key-value mapping structure that associates string keys with pointer values.
 * The map never owns or manages the memory for keys or values.
 * The caller is always responsible for allocating and freeing memory for both keys and values.
 */
typedef struct map_s map_t;

/**
 * Create a new heap-allocated empty map
 * @return Pointer to the new map, or NULL on failure
 */
map_t* ar_map_create(void);

/**
 * Get a reference from map by key
 * @param map Map
 * @param key Key to lookup
 * @return Pointer to the referenced value, or NULL if not found
 */
void* ar_map_get(const map_t *map, const char *key);

/**
 * Set a reference in map
 * @param map Map
 * @param key Key to set (the pointer is stored directly, not copied)
 * @param ref Pointer to value to reference
 * @return true if successful, false otherwise
 * @note The caller is responsible for allocating and freeing the key string.
 *       The key string must remain valid for the lifetime of the map entry.
 *       The map never takes ownership of keys or values.
 */
bool ar_map_set(map_t *map, const char *key, void *ref);

/**
 * Get the number of used entries in the map
 * @param map The map to count
 * @return The number of used entries
 */
size_t ar_map_count(const map_t *map);

/**
 * Get an array of all refs in the map
 * @param map The map to get refs from
 * @return Array of pointers to refs, or NULL on failure
 * @note The caller is responsible for freeing the returned array using free().
 *       The refs themselves are not copied and remain owned by the caller.
 *       The caller can use ar_map_count() to determine the size of the array.
 */
void** ar_map_refs(const map_t *map);

/**
 * Free all resources in a map
 * @param map Map to free
 * @note This function only frees the map structure itself.
 *       It does not free memory for keys or referenced values.
 *       The caller is responsible for freeing all keys and values that were added to the map.
 */
void ar_map_destroy(map_t *map);


#endif /* AGERUN_MAP_H */

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
 * Callback function type for map iteration
 * @param key The key for the current entry
 * @param value The value for the current entry
 * @param arg Additional argument passed to the iterator function
 * @return true to continue iteration, false to stop
 * @note The map does not own keys or values; it only provides access to these pointers during iteration.
 *       The caller must not free the key or value pointers during iteration, but is responsible
 *       for their eventual cleanup outside of the iteration context.
 */
typedef bool (*map_iterator_t)(const char *key, void *value, void *arg);

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
 * Iterate over all entries in the map
 * @param map Map to iterate over
 * @param iterator Function to call for each entry
 * @param arg Additional argument to pass to the iterator function
 * @return true if iteration completed, false if stopped early by iterator
 */
bool ar_map_iterate(const map_t *map, map_iterator_t iterator, void *arg);

/**
 * Free all resources in a map
 * @param map Map to free
 * @note This function only frees the map structure itself.
 *       It does not free memory for keys or referenced values.
 *       The caller is responsible for freeing all keys and values that were added to the map.
 */
void ar_map_destroy(map_t *map);


#endif /* AGERUN_MAP_H */

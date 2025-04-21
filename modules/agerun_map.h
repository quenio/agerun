#ifndef AGERUN_MAP_H
#define AGERUN_MAP_H

#include <stdbool.h>
#include <stddef.h>

/**
 * A key-value mapping structure that associates string keys with pointer values.
 */
typedef struct map_s map_t;

/**
 * Callback function type for map iteration
 * @param key The key for the current entry
 * @param value The value for the current entry
 * @param arg Additional argument passed to the iterator function
 * @return true to continue iteration, false to stop
 * @note This intentionally uses const-qualified char* for the key to match the map storage,
 *       but the internal implementation does NOT pass ownership of keys or values to the caller.
 *       The callback must NOT free the key or value pointers during iteration, as they are still
 *       owned by the map. The map module only provides access to these pointers during iteration.
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
 * @note The caller is responsible for ensuring the key string remains valid
 *       for the lifetime of the map entry.
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
 * @note This function does not free memory for keys or referenced values.
 *       The caller is responsible for managing those resources.
 */
void ar_map_destroy(map_t *map);


#endif /* AGERUN_MAP_H */

#ifndef AGERUN_MAP_H
#define AGERUN_MAP_H

#include <stdbool.h>
#include <stddef.h>

/* Constants */
#define MAP_SIZE 64

/**
 * A key-value mapping structure that associates string keys with pointer values.
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
const void* ar_map_get(map_t *map, const char *key);

/**
 * Set a reference in map
 * @param map Map
 * @param key Key to set (the pointer is stored directly, not copied)
 * @param ref Pointer to value to reference
 * @return true if successful, false otherwise
 * @note The caller is responsible for ensuring the key string remains valid
 *       for the lifetime of the map entry.
 */
bool ar_map_set(map_t *map, const char *key, const void *ref);

/**
 * Free all resources in a map
 * @param map Map to free
 * @note This function does not free memory for keys or referenced values.
 *       The caller is responsible for managing those resources.
 */
void ar_map_free(map_t *map);


#endif /* AGERUN_MAP_H */

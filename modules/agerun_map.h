#ifndef AGERUN_MAP_H
#define AGERUN_MAP_H

#include <stdbool.h>

/* Constants */
#define MAP_SIZE 64

/**
 * Map Entry for storing key-value pairs
 */
typedef struct entry_s {
    char *key;
    void *ref;
    bool is_used;
} entry_t;

/**
 * Map for storing agent state
 */
typedef struct map_s {
    entry_t entries[MAP_SIZE];
    int count;
    int ref_count; /* Reference counter for this map */
} map_t;

/**
 * Create a new heap-allocated empty map
 * @return Pointer to the new map, or NULL on failure
 * @note This function allocates memory for the map structure and then
 *       initializes it by calling ar_map_init(). Use this when you need
 *       a standalone map allocated on the heap.
 */
map_t* ar_map_create(void);

/**
 * Initialize a map structure
 * @param map Map to initialize
 * @return true if successful, false otherwise
 * @note Use this function to initialize a map that is already allocated,
 *       such as a map that is embedded in another structure or allocated
 *       on the stack. This is called by ar_map_create() internally.
 */
bool ar_map_init(map_t *map);

/**
 * Get a reference from map by key
 * @param map Map
 * @param key Key to lookup
 * @return Pointer to the referenced value, or NULL if not found
 */
void* ar_map_get(map_t *map, const char *key);

/**
 * Set a reference in map
 * @param map Map
 * @param key Key to set
 * @param ref Pointer to value to reference
 * @return true if successful, false otherwise
 */
bool ar_map_set(map_t *map, const char *key, void *ref);

/**
 * Free all resources in a map
 * @param map Map to free
 */
void ar_map_free(map_t *map);

/**
 * Get a reference to the map (increments reference count)
 * @param map Map to reference
 * @return Same map pointer for convenience
 */
map_t* ar_map_get_reference(map_t *map);

#endif /* AGERUN_MAP_H */

#ifndef AGERUN_MAP_H
#define AGERUN_MAP_H

#include <stdbool.h>
#include "agerun_data.h"

/* Constants */
#define MAP_SIZE 256

/**
 * Map Entry for storing key-value pairs
 */
typedef struct entry_s {
    char *key;
    data_t value;
    bool is_used;
} entry_t;

/**
 * Map for storing agent state
 */
typedef struct map_s {
    entry_t entries[MAP_SIZE];
    int count;
} map_t;

/**
 * Create a new empty map
 * @return Pointer to the new map, or NULL on failure
 */
map_t* ar_map_create(void);

/**
 * Initialize a map
 * @param map Map to initialize
 * @return true if successful, false otherwise
 */
bool ar_map_init(map_t *map);

/**
 * Get a value from map by key
 * @param map Map
 * @param key Key to lookup
 * @return Pointer to the value, or NULL if not found
 */
data_t* ar_map_get(map_t *map, const char *key);

/**
 * Set a value in map (reference assignment)
 * @param map Map
 * @param key Key to set
 * @param value_ptr Pointer to value to set (value is referenced, not copied)
 * @return true if successful, false otherwise
 */
bool ar_map_set(map_t *map, const char *key, data_t *value_ptr);

/**
 * Free all resources in a map
 * @param map Map to free
 */
void ar_map_free(map_t *map);

#endif /* AGERUN_MAP_H */

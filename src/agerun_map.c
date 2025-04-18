#include "agerun_map.h"
#include "agerun_data.h"
#include <stdlib.h>
#include <string.h>

/**
 * Create a new empty map
 * @return Pointer to the new map, or NULL on failure
 */
map_t* ar_map_create(void) {
    map_t *map = (map_t *)malloc(sizeof(map_t));
    if (!map) return NULL;
    
    if (!ar_map_init(map)) {
        free(map);
        return NULL;
    }
    
    return map;
}

/**
 * Initialize a map
 * @param map Map to initialize
 * @return true if successful, false otherwise
 */
bool ar_map_init(map_t *map) {
    if (!map) {
        return false;
    }
    
    for (int i = 0; i < MAP_SIZE; i++) {
        map->entries[i].is_used = false;
        map->entries[i].key = NULL;
    }
    
    map->count = 0;
    return true;
}

/**
 * Get a value from map by key
 * @param map Map
 * @param key Key to lookup
 * @return Pointer to the value, or NULL if not found
 */
data_t* ar_map_get(map_t *map, const char *key) {
    if (!map || !key) {
        return NULL;
    }
    
    for (int i = 0; i < MAP_SIZE; i++) {
        if (map->entries[i].is_used && map->entries[i].key && 
            strcmp(map->entries[i].key, key) == 0) {
            return &map->entries[i].value;
        }
    }
    return NULL;
}

/**
 * Set a value in map
 * @param map Map
 * @param key Key to set
 * @param value_ptr Pointer to value to set
 * @return true if successful, false otherwise
 */
bool ar_map_set(map_t *map, const char *key, data_t *value_ptr) {
    if (!map || !key || !value_ptr) {
        return false;
    }
    
    // First, check if key already exists using ar_map_get
    data_t *existing = ar_map_get(map, key);
    if (existing) {
        // Free old value
        ar_data_free(existing);
        
        // Directly assign the new value
        *existing = *value_ptr;
        
        return true;
    }
    
    // Find empty slot
    for (int i = 0; i < MAP_SIZE; i++) {
        if (!map->entries[i].is_used) {
            char *key_copy = strdup(key);
            if (!key_copy) {
                return false;
            }
            
            map->entries[i].is_used = true;
            map->entries[i].key = key_copy;
            map->entries[i].value = *value_ptr;
            
            map->count++;
            return true;
        }
    }
    
    return false; // No space left
}

/**
 * Free all resources in a map
 * @param map Map to free
 */
void ar_map_free(map_t *map) {
    if (!map) return;
    
    for (int i = 0; i < MAP_SIZE; i++) {
        if (map->entries[i].is_used && map->entries[i].key) {
            free(map->entries[i].key);
            ar_data_free(&map->entries[i].value);
        }
    }
    
    /* Free the map structure itself */
    free(map);
}

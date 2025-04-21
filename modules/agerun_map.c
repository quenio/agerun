#include "agerun_map.h"
#include <stdlib.h>
#include <string.h>

/* Constants */
#define MAP_SIZE 64

/**
 * Map Entry for storing key-value pairs
 */
typedef struct entry_s {
    const char *key;
    void *ref;
    bool is_used;
} entry_t;

/**
 * Map implementation structure
 */
struct map_s {
    entry_t entries[MAP_SIZE];
    int count;
};


/**
 * Create a new heap-allocated empty map
 * @return Pointer to the new map, or NULL on failure
 */
map_t* ar_map_create(void) {
    map_t *map = (map_t *)malloc(sizeof(map_t));
    if (!map) return NULL;
    
    for (int i = 0; i < MAP_SIZE; i++) {
        map->entries[i].is_used = false;
        map->entries[i].key = NULL;
        map->entries[i].ref = NULL;
    }
    
    map->count = 0;
    return map;
}

/**
 * Get a reference from map by key
 * @param map Map
 * @param key Key to lookup
 * @return Pointer to the referenced value, or NULL if not found
 */
void* ar_map_get(const map_t *map, const char *key) {
    if (!map || !key) {
        return NULL;
    }
    
    for (int i = 0; i < MAP_SIZE; i++) {
        if (map->entries[i].is_used && map->entries[i].key && 
            strcmp(map->entries[i].key, key) == 0) {
            return map->entries[i].ref;
        }
    }
    return NULL;
}

/**
 * Set a reference in map
 * @param map Map
 * @param key Key to set
 * @param ref Pointer to value to reference
 * @return true if successful, false otherwise
 */
bool ar_map_set(map_t *map, const char *key, void *ref) {
    if (!map || !key) {
        return false;
    }
    
    // First, check if key already exists
    for (int i = 0; i < MAP_SIZE; i++) {
        if (map->entries[i].is_used && map->entries[i].key && 
            strcmp(map->entries[i].key, key) == 0) {
            // Just update the reference pointer
            map->entries[i].ref = ref;
            return true;
        }
    }
    
    // Find empty slot
    for (int i = 0; i < MAP_SIZE; i++) {
        if (!map->entries[i].is_used) {
            map->entries[i].is_used = true;
            map->entries[i].key = key;  // Store the key pointer directly without copying
            map->entries[i].ref = ref;
            
            map->count++;
            return true;
        }
    }
    
    return false; // No space left
}

/**
 * Iterate over all entries in the map
 * @param map Map to iterate over
 * @param iterator Function to call for each entry
 * @param arg Additional argument to pass to the iterator function
 * @return true if iteration completed, false if stopped early by iterator
 */
bool ar_map_iterate(const map_t *map, map_iterator_t iterator, void *arg) {
    if (!map || !iterator) {
        return false;
    }
    
    for (int i = 0; i < MAP_SIZE; i++) {
        if (map->entries[i].is_used && map->entries[i].key) {
            if (!iterator(map->entries[i].key, map->entries[i].ref, arg)) {
                return false; // Iterator requested to stop
            }
        }
    }
    
    return true; // Completed all iterations
}

/**
 * Get the number of used entries in the map
 * @param map The map to count
 * @return The number of used entries
 */
size_t ar_map_count(const map_t *map) {
    if (!map) {
        return 0;
    }
    
    return (size_t)map->count;
}

/**
 * Get an array of all refs in the map
 * @param map The map to get refs from
 * @return Array of pointers to refs, or NULL on failure
 * @note The caller is responsible for freeing the returned array using free().
 *       The refs themselves are not copied and remain owned by the caller.
 *       The caller can use ar_map_count() to determine the size of the array.
 */
void** ar_map_refs(const map_t *map) {
    if (!map) {
        return NULL;
    }
    
    if (map->count == 0) {
        return NULL;
    }
    
    void **refs = (void**)malloc((size_t)map->count * sizeof(void*));
    if (!refs) {
        return NULL;
    }
    
    size_t index = 0;
    for (int i = 0; i < MAP_SIZE && index < (size_t)map->count; i++) {
        if (map->entries[i].is_used && map->entries[i].key) {
            refs[index++] = map->entries[i].ref;
        }
    }
    
    return refs;
}

/**
 * Free all resources in a map
 * @param map Map to free
 */
void ar_map_destroy(map_t *map) {
    if (!map) return;
    
    free(map);
}


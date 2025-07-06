#include "ar_map.h"
#include "ar_heap.h"
#include <stdlib.h>
#include <string.h>

/* Constants */
#define MAP_SIZE 64

/**
 * Map Entry for storing key-value pairs
 */
typedef struct entry_s {
    const char *ref_key;
    void *ref_value;
    bool is_used;
} entry_t;

/**
 * Map implementation structure
 */
struct ar_map_s {
    entry_t entries[MAP_SIZE];
    int count;
};


/**
 * Create a new heap-allocated empty map
 * @return Pointer to the new map, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
ar_map_t* ar_map__create(void) {
    ar_map_t *own_map = (ar_map_t *)AR__HEAP__MALLOC(sizeof(ar_map_t), "Map structure");
    if (!own_map) return NULL;
    
    for (int i = 0; i < MAP_SIZE; i++) {
        own_map->entries[i].is_used = false;
        own_map->entries[i].ref_key = NULL;
        own_map->entries[i].ref_value = NULL;
    }
    
    own_map->count = 0;
    return own_map; // Ownership transferred to caller
}

/**
 * Get a reference from map by key
 * @param ref_map The map to look up in (borrowed reference)
 * @param ref_key The key to lookup (borrowed reference)
 * @return Pointer to the referenced value, or NULL if not found
 * @note Ownership: Returns a borrowed reference. Caller does not own the returned value.
 */
void* ar_map__get(const ar_map_t *ref_map, const char *ref_key) {
    if (!ref_map || !ref_key) {
        return NULL;
    }
    
    for (int i = 0; i < MAP_SIZE; i++) {
        if (ref_map->entries[i].is_used && ref_map->entries[i].ref_key && 
            strcmp(ref_map->entries[i].ref_key, ref_key) == 0) {
            return ref_map->entries[i].ref_value;
        }
    }
    return NULL;
}

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
bool ar_map__set(ar_map_t *mut_map, const char *ref_key, void *ref_value) {
    if (!mut_map || !ref_key) {
        return false;
    }
    
    // First, check if key already exists
    for (int i = 0; i < MAP_SIZE; i++) {
        if (mut_map->entries[i].is_used && mut_map->entries[i].ref_key && 
            strcmp(mut_map->entries[i].ref_key, ref_key) == 0) {
            if (ref_value == NULL) {
                // Setting to NULL means removing the entry
                mut_map->entries[i].is_used = false;
                mut_map->entries[i].ref_key = NULL;
                mut_map->entries[i].ref_value = NULL;
                mut_map->count--;
            } else {
                // Just update the reference pointer
                mut_map->entries[i].ref_value = ref_value;
            }
            return true;
        }
    }
    
    // If value is NULL and key doesn't exist, nothing to remove
    if (ref_value == NULL) {
        return true;
    }
    
    // Find empty slot
    for (int i = 0; i < MAP_SIZE; i++) {
        if (!mut_map->entries[i].is_used) {
            mut_map->entries[i].is_used = true;
            mut_map->entries[i].ref_key = ref_key;  // Store the key pointer directly without copying
            mut_map->entries[i].ref_value = ref_value;
            
            mut_map->count++;
            return true;
        }
    }
    
    return false; // No space left
}

/**
 * Get the number of used entries in the map
 * @param ref_map The map to count (borrowed reference)
 * @return The number of used entries
 */
size_t ar_map__count(const ar_map_t *ref_map) {
    if (!ref_map) {
        return 0;
    }
    
    return (size_t)ref_map->count;
}

/**
 * Get an array of all refs in the map
 * @param ref_map The map to get refs from (borrowed reference)
 * @return Array of pointers to refs, or NULL on failure
 * @note Ownership: Returns an owned array that caller must free.
 *       The caller is responsible for freeing the returned array using AR__HEAP__FREE().
 *       The refs themselves are borrowed references and remain owned by their original owners.
 *       The caller can use ar_map_count() to determine the size of the array.
 */
void** ar_map__refs(const ar_map_t *ref_map) {
    if (!ref_map) {
        return NULL;
    }
    
    if (ref_map->count == 0) {
        return NULL;
    }
    
    void **own_refs = (void**)AR__HEAP__MALLOC((size_t)ref_map->count * sizeof(void*), "Map references array");
    if (!own_refs) {
        return NULL;
    }
    
    size_t index = 0;
    for (int i = 0; i < MAP_SIZE && index < (size_t)ref_map->count; i++) {
        if (ref_map->entries[i].is_used && ref_map->entries[i].ref_key) {
            own_refs[index++] = ref_map->entries[i].ref_value;
        }
    }
    
    return own_refs; // Ownership of the array transferred to caller
}

/**
 * Free all resources in a map
 * @param own_map Map to free (owned value)
 * @note Ownership: Destroys the map structure itself.
 *       It does not free memory for keys or referenced values.
 *       The caller remains responsible for freeing all keys and values that were added to the map.
 */
void ar_map__destroy(ar_map_t *own_map) {
    if (!own_map) return;
    
    AR__HEAP__FREE(own_map);
}


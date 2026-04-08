#include "ar_map.h"
#include "ar_heap.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Constants */
#define MAP_SIZE 128

typedef enum {
    ENTRY_STATE_EMPTY = 0,
    ENTRY_STATE_OCCUPIED,
    ENTRY_STATE_TOMBSTONE
} entry_state_t;

/**
 * Map Entry for storing key-value pairs
 */
typedef struct entry_s {
    const char *ref_key;
    void *ref_value;
    entry_state_t state;
} entry_t;

/**
 * Map implementation structure
 */
struct ar_map_s {
    entry_t entries[MAP_SIZE];
    int count;
};

static uint32_t _hash_key(const char *ref_key) {
    uint32_t hash = 2166136261u;
    const unsigned char *ref_bytes = (const unsigned char *)ref_key;

    while (*ref_bytes != '\0') {
        hash ^= (uint32_t)(*ref_bytes);
        hash *= 16777619u;
        ref_bytes++;
    }

    return hash;
}

static int _find_entry_index(const ar_map_t *ref_map, const char *ref_key) {
    uint32_t start_index;

    if (!ref_map || !ref_key) {
        return -1;
    }

    start_index = _hash_key(ref_key) % MAP_SIZE;
    for (int probe = 0; probe < MAP_SIZE; probe++) {
        int index = (int)((start_index + (uint32_t)probe) % MAP_SIZE);
        const entry_t *ref_entry = &ref_map->entries[index];

        if (ref_entry->state == ENTRY_STATE_EMPTY) {
            return -1;
        }

        if (ref_entry->state == ENTRY_STATE_OCCUPIED && ref_entry->ref_key != NULL &&
            strcmp(ref_entry->ref_key, ref_key) == 0) {
            return index;
        }
    }

    return -1;
}

static int _find_insert_index(const ar_map_t *ref_map, const char *ref_key) {
    uint32_t start_index;
    int first_tombstone = -1;

    if (!ref_map || !ref_key) {
        return -1;
    }

    start_index = _hash_key(ref_key) % MAP_SIZE;
    for (int probe = 0; probe < MAP_SIZE; probe++) {
        int index = (int)((start_index + (uint32_t)probe) % MAP_SIZE);
        const entry_t *ref_entry = &ref_map->entries[index];

        if (ref_entry->state == ENTRY_STATE_OCCUPIED && ref_entry->ref_key != NULL &&
            strcmp(ref_entry->ref_key, ref_key) == 0) {
            return index;
        }

        if (ref_entry->state == ENTRY_STATE_TOMBSTONE && first_tombstone == -1) {
            first_tombstone = index;
        }

        if (ref_entry->state == ENTRY_STATE_EMPTY) {
            if (first_tombstone != -1) {
                return first_tombstone;
            }
            return index;
        }
    }

    return first_tombstone;
}

/**
 * Create a new heap-allocated empty map
 * @return Pointer to the new map, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
ar_map_t* ar_map__create(void) {
    ar_map_t *own_map = (ar_map_t *)AR__HEAP__MALLOC(sizeof(ar_map_t), "Map structure");
    if (!own_map) {
        return NULL;
    }

    for (int i = 0; i < MAP_SIZE; i++) {
        own_map->entries[i].state = ENTRY_STATE_EMPTY;
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
    int entry_index;

    if (!ref_map || !ref_key) {
        return NULL;
    }

    entry_index = _find_entry_index(ref_map, ref_key);
    if (entry_index < 0) {
        return NULL;
    }

    return ref_map->entries[entry_index].ref_value;
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
    int entry_index;
    entry_t *mut_entry;

    if (!mut_map || !ref_key) {
        return false;
    }

    entry_index = _find_insert_index(mut_map, ref_key);
    if (entry_index < 0) {
        return ref_value == NULL;
    }

    mut_entry = &mut_map->entries[entry_index];
    if (mut_entry->state == ENTRY_STATE_OCCUPIED && mut_entry->ref_key != NULL &&
        strcmp(mut_entry->ref_key, ref_key) == 0) {
        if (ref_value == NULL) {
            mut_entry->state = ENTRY_STATE_TOMBSTONE;
            mut_entry->ref_key = NULL;
            mut_entry->ref_value = NULL;
            mut_map->count--;
        } else {
            mut_entry->ref_value = ref_value;
        }
        return true;
    }

    if (ref_value == NULL) {
        return true;
    }

    mut_entry->state = ENTRY_STATE_OCCUPIED;
    mut_entry->ref_key = ref_key;
    mut_entry->ref_value = ref_value;
    mut_map->count++;
    return true;
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
        if (ref_map->entries[i].state == ENTRY_STATE_OCCUPIED && ref_map->entries[i].ref_key != NULL) {
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
    if (!own_map) {
        return;
    }

    AR__HEAP__FREE(own_map);
}

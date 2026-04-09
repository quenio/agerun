#include "ar_map.h"
#include "ar_heap.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define AR_MAP__INITIAL_CAPACITY 16U
#define AR_MAP__MAX_LOAD_PERCENT 70U

typedef enum {
    ENTRY_STATE_EMPTY = 0,
    ENTRY_STATE_OCCUPIED,
    ENTRY_STATE_TOMBSTONE
} entry_state_t;

typedef struct entry_s {
    const char *ref_key;
    void *ref_value;
    entry_state_t state;
} entry_t;

struct ar_map_s {
    entry_t *own_entries;
    size_t capacity;
    size_t count;
    size_t tombstone_count;
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

static entry_t *_allocate_entries(size_t capacity) {
    entry_t *own_entries;

    if (capacity == 0U) {
        return NULL;
    }

    own_entries = (entry_t *)AR__HEAP__MALLOC(capacity * sizeof(entry_t), "Map entries");
    if (!own_entries) {
        return NULL;
    }

    for (size_t i = 0; i < capacity; i++) {
        own_entries[i].ref_key = NULL;
        own_entries[i].ref_value = NULL;
        own_entries[i].state = ENTRY_STATE_EMPTY;
    }

    return own_entries; // Ownership transferred to caller
}

static bool _find_slot(
    const entry_t *ref_entries,
    size_t capacity,
    const char *ref_key,
    size_t *out_index,
    bool *out_exists
) {
    uint32_t start_index;
    size_t first_tombstone;
    bool has_tombstone;

    if (!ref_entries || capacity == 0U || !ref_key || !out_index || !out_exists) {
        return false;
    }

    start_index = _hash_key(ref_key) % (uint32_t)capacity;
    first_tombstone = 0U;
    has_tombstone = false;

    for (size_t probe = 0; probe < capacity; probe++) {
        size_t index = (size_t)((start_index + (uint32_t)probe) % (uint32_t)capacity);
        const entry_t *ref_entry = &ref_entries[index];

        if (ref_entry->state == ENTRY_STATE_OCCUPIED && ref_entry->ref_key != NULL &&
            strcmp(ref_entry->ref_key, ref_key) == 0) {
            *out_index = index;
            *out_exists = true;
            return true;
        }

        if (ref_entry->state == ENTRY_STATE_TOMBSTONE && !has_tombstone) {
            first_tombstone = index;
            has_tombstone = true;
            continue;
        }

        if (ref_entry->state == ENTRY_STATE_EMPTY) {
            *out_index = has_tombstone ? first_tombstone : index;
            *out_exists = false;
            return true;
        }
    }

    if (has_tombstone) {
        *out_index = first_tombstone;
        *out_exists = false;
        return true;
    }

    return false;
}

static bool _insert_entry_into_table(
    entry_t *mut_entries,
    size_t capacity,
    const char *ref_key,
    void *ref_value
) {
    size_t entry_index;
    bool entry_exists;

    if (!mut_entries || capacity == 0U || !ref_key || !ref_value) {
        return false;
    }

    if (!_find_slot(mut_entries, capacity, ref_key, &entry_index, &entry_exists)) {
        return false;
    }

    mut_entries[entry_index].ref_key = ref_key;
    mut_entries[entry_index].ref_value = ref_value;
    mut_entries[entry_index].state = ENTRY_STATE_OCCUPIED;
    return true;
}

static bool _resize_map(ar_map_t *mut_map, size_t new_capacity) {
    entry_t *own_new_entries;

    if (!mut_map || new_capacity == 0U || new_capacity < mut_map->count) {
        return false;
    }

    own_new_entries = _allocate_entries(new_capacity);
    if (!own_new_entries) {
        return false;
    }

    for (size_t i = 0; i < mut_map->capacity; i++) {
        const entry_t *ref_entry = &mut_map->own_entries[i];

        if (ref_entry->state != ENTRY_STATE_OCCUPIED || ref_entry->ref_key == NULL) {
            continue;
        }

        if (!_insert_entry_into_table(
                own_new_entries,
                new_capacity,
                ref_entry->ref_key,
                ref_entry->ref_value
            )) {
            AR__HEAP__FREE(own_new_entries);
            return false;
        }
    }

    AR__HEAP__FREE(mut_map->own_entries);
    mut_map->own_entries = own_new_entries;
    mut_map->capacity = new_capacity;
    mut_map->tombstone_count = 0U;
    return true;
}

static bool _ensure_capacity_for_insert(ar_map_t *mut_map) {
    size_t used_slots;
    size_t required_slots;

    if (!mut_map || mut_map->capacity == 0U) {
        return false;
    }

    used_slots = mut_map->count + mut_map->tombstone_count;
    required_slots = used_slots + 1U;

    if (required_slots * 100U < mut_map->capacity * AR_MAP__MAX_LOAD_PERCENT) {
        return true;
    }

    return _resize_map(mut_map, mut_map->capacity * 2U);
}

ar_map_t *ar_map__create(void) {
    ar_map_t *own_map;

    own_map = (ar_map_t *)AR__HEAP__MALLOC(sizeof(ar_map_t), "Map structure");
    if (!own_map) {
        return NULL;
    }

    own_map->own_entries = _allocate_entries(AR_MAP__INITIAL_CAPACITY);
    if (!own_map->own_entries) {
        AR__HEAP__FREE(own_map);
        return NULL;
    }

    own_map->capacity = AR_MAP__INITIAL_CAPACITY;
    own_map->count = 0U;
    own_map->tombstone_count = 0U;
    return own_map; // Ownership transferred to caller
}

void *ar_map__get(const ar_map_t *ref_map, const char *ref_key) {
    size_t entry_index;
    bool entry_exists;

    if (!ref_map || !ref_map->own_entries || ref_map->capacity == 0U || !ref_key) {
        return NULL;
    }

    if (!_find_slot(ref_map->own_entries, ref_map->capacity, ref_key, &entry_index, &entry_exists) ||
        !entry_exists) {
        return NULL;
    }

    return ref_map->own_entries[entry_index].ref_value;
}

bool ar_map__set(ar_map_t *mut_map, const char *ref_key, void *ref_value) {
    size_t entry_index;
    bool entry_exists;
    entry_t *mut_entry;

    if (!mut_map || !mut_map->own_entries || mut_map->capacity == 0U || !ref_key) {
        return false;
    }

    if (!_find_slot(mut_map->own_entries, mut_map->capacity, ref_key, &entry_index, &entry_exists)) {
        return false;
    }

    mut_entry = &mut_map->own_entries[entry_index];
    if (entry_exists) {
        if (ref_value == NULL) {
            mut_entry->ref_key = NULL;
            mut_entry->ref_value = NULL;
            mut_entry->state = ENTRY_STATE_TOMBSTONE;
            mut_map->count--;
            mut_map->tombstone_count++;
        } else {
            mut_entry->ref_value = ref_value;
        }
        return true;
    }

    if (ref_value == NULL) {
        return true;
    }

    if (!_ensure_capacity_for_insert(mut_map)) {
        return false;
    }

    if (!_find_slot(mut_map->own_entries, mut_map->capacity, ref_key, &entry_index, &entry_exists) ||
        entry_exists) {
        return false;
    }

    mut_entry = &mut_map->own_entries[entry_index];
    if (mut_entry->state == ENTRY_STATE_TOMBSTONE) {
        mut_map->tombstone_count--;
    }

    mut_entry->ref_key = ref_key;
    mut_entry->ref_value = ref_value;
    mut_entry->state = ENTRY_STATE_OCCUPIED;
    mut_map->count++;
    return true;
}

size_t ar_map__count(const ar_map_t *ref_map) {
    if (!ref_map) {
        return 0U;
    }

    return ref_map->count;
}

void **ar_map__refs(const ar_map_t *ref_map) {
    void **own_refs;
    size_t ref_index;

    if (!ref_map || !ref_map->own_entries || ref_map->count == 0U) {
        return NULL;
    }

    own_refs = (void **)AR__HEAP__MALLOC(ref_map->count * sizeof(void *), "Map references array");
    if (!own_refs) {
        return NULL;
    }

    ref_index = 0U;
    for (size_t i = 0; i < ref_map->capacity && ref_index < ref_map->count; i++) {
        if (ref_map->own_entries[i].state == ENTRY_STATE_OCCUPIED &&
            ref_map->own_entries[i].ref_key != NULL) {
            own_refs[ref_index++] = ref_map->own_entries[i].ref_value;
        }
    }

    return own_refs; // Ownership of the array transferred to caller
}

void ar_map__destroy(ar_map_t *own_map) {
    if (!own_map) {
        return;
    }

    if (own_map->own_entries) {
        AR__HEAP__FREE(own_map->own_entries);
        own_map->own_entries = NULL;
    }

    AR__HEAP__FREE(own_map);
}

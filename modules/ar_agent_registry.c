/**
 * @file ar_agent_registry.c
 * @brief Implementation of the agent registry module
 */

#include "ar_agent_registry.h"
#include "ar_data.h"
#include "ar_heap.h"
#include "ar_list.h"
#include "ar_map.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Agent registry structure */
struct ar_agent_registry_s {
    ar_list_t *own_registered_ids;      /* List of registered agent ID strings */
    ar_map_t *own_agent_map;            /* Map of agent_id -> agent pointer */
    int64_t next_agent_id;              /* Next ID to allocate */
    int64_t first_registered_id;        /* First registered agent ID in insertion order */
    int64_t last_registered_id;         /* Last registered agent ID in insertion order */
    int64_t *own_next_ids;              /* Next registered ID by numeric agent ID */
    int64_t *own_prev_ids;              /* Previous registered ID by numeric agent ID */
    bool *own_is_registered;            /* Registration bitmap by numeric agent ID */
    const char **ref_key_by_id;         /* Stable borrowed key string for each registered ID */
    size_t id_capacity;                 /* Allocated size for indexed arrays */
};

static void _reset_index_state(ar_agent_registry_t *mut_registry) {
    if (!mut_registry) {
        return;
    }

    for (size_t i = 0; i < mut_registry->id_capacity; i++) {
        if (mut_registry->own_next_ids) {
            mut_registry->own_next_ids[i] = -1;
        }
        if (mut_registry->own_prev_ids) {
            mut_registry->own_prev_ids[i] = -1;
        }
        if (mut_registry->own_is_registered) {
            mut_registry->own_is_registered[i] = false;
        }
        if (mut_registry->ref_key_by_id) {
            mut_registry->ref_key_by_id[i] = NULL;
        }
    }

    mut_registry->first_registered_id = 0;
    mut_registry->last_registered_id = 0;
}

static bool _ensure_id_capacity(ar_agent_registry_t *mut_registry, int64_t agent_id) {
    int64_t *own_new_next_ids;
    int64_t *own_new_prev_ids;
    bool *own_new_is_registered;
    const char **ref_new_key_by_id;
    size_t new_capacity;

    if (!mut_registry || agent_id < 0) {
        return false;
    }

    if ((size_t)agent_id < mut_registry->id_capacity) {
        return true;
    }

    new_capacity = mut_registry->id_capacity == 0 ? 16 : mut_registry->id_capacity;
    while (new_capacity <= (size_t)agent_id) {
        new_capacity *= 2;
    }

    own_new_next_ids = AR__HEAP__MALLOC(new_capacity * sizeof(int64_t), "agent registry next ids");
    if (!own_new_next_ids) {
        return false;
    }

    own_new_prev_ids = AR__HEAP__MALLOC(new_capacity * sizeof(int64_t), "agent registry prev ids");
    if (!own_new_prev_ids) {
        AR__HEAP__FREE(own_new_next_ids);
        return false;
    }

    own_new_is_registered = AR__HEAP__CALLOC(new_capacity, sizeof(bool), "agent registry registered flags");
    if (!own_new_is_registered) {
        AR__HEAP__FREE(own_new_prev_ids);
        AR__HEAP__FREE(own_new_next_ids);
        return false;
    }

    ref_new_key_by_id = AR__HEAP__CALLOC(new_capacity, sizeof(const char *), "agent registry key refs");
    if (!ref_new_key_by_id) {
        AR__HEAP__FREE(own_new_is_registered);
        AR__HEAP__FREE(own_new_prev_ids);
        AR__HEAP__FREE(own_new_next_ids);
        return false;
    }

    for (size_t i = 0; i < new_capacity; i++) {
        own_new_next_ids[i] = -1;
        own_new_prev_ids[i] = -1;
    }

    for (size_t i = 0; i < mut_registry->id_capacity; i++) {
        if (mut_registry->own_next_ids) {
            own_new_next_ids[i] = mut_registry->own_next_ids[i];
        }
        if (mut_registry->own_prev_ids) {
            own_new_prev_ids[i] = mut_registry->own_prev_ids[i];
        }
        if (mut_registry->own_is_registered) {
            own_new_is_registered[i] = mut_registry->own_is_registered[i];
        }
        if (mut_registry->ref_key_by_id) {
            ref_new_key_by_id[i] = mut_registry->ref_key_by_id[i];
        }
    }

    if (mut_registry->own_next_ids) {
        AR__HEAP__FREE(mut_registry->own_next_ids);
    }
    if (mut_registry->own_prev_ids) {
        AR__HEAP__FREE(mut_registry->own_prev_ids);
    }
    if (mut_registry->own_is_registered) {
        AR__HEAP__FREE(mut_registry->own_is_registered);
    }
    if (mut_registry->ref_key_by_id) {
        AR__HEAP__FREE(mut_registry->ref_key_by_id);
    }

    mut_registry->own_next_ids = own_new_next_ids;
    mut_registry->own_prev_ids = own_new_prev_ids;
    mut_registry->own_is_registered = own_new_is_registered;
    mut_registry->ref_key_by_id = ref_new_key_by_id;
    mut_registry->id_capacity = new_capacity;
    return true;
}

/* Create a new agent registry */
ar_agent_registry_t* ar_agent_registry__create(void) {
    ar_agent_registry_t *own_registry = AR__HEAP__MALLOC(sizeof(ar_agent_registry_t), "agent registry");
    if (!own_registry) {
        return NULL;
    }

    own_registry->own_registered_ids = ar_list__create();
    if (!own_registry->own_registered_ids) {
        AR__HEAP__FREE(own_registry);
        return NULL;
    }

    own_registry->own_agent_map = ar_map__create();
    if (!own_registry->own_agent_map) {
        ar_list__destroy(own_registry->own_registered_ids);
        AR__HEAP__FREE(own_registry);
        return NULL;
    }

    own_registry->next_agent_id = 1;
    own_registry->first_registered_id = 0;
    own_registry->last_registered_id = 0;
    own_registry->own_next_ids = NULL;
    own_registry->own_prev_ids = NULL;
    own_registry->own_is_registered = NULL;
    own_registry->ref_key_by_id = NULL;
    own_registry->id_capacity = 0;

    return own_registry; /* Ownership transferred to caller */
}

/* Destroy an agent registry */
void ar_agent_registry__destroy(ar_agent_registry_t *own_registry) {
    if (!own_registry) {
        return;
    }

    if (own_registry->own_registered_ids) {
        while (ar_list__count(own_registry->own_registered_ids) > 0) {
            ar_data_t *own_data = ar_list__remove_first(own_registry->own_registered_ids);
            if (own_data) {
                ar_data__destroy(own_data);
            }
        }
        ar_list__destroy(own_registry->own_registered_ids);
    }

    if (own_registry->own_agent_map) {
        ar_map__destroy(own_registry->own_agent_map);
    }

    if (own_registry->own_next_ids) {
        AR__HEAP__FREE(own_registry->own_next_ids);
    }
    if (own_registry->own_prev_ids) {
        AR__HEAP__FREE(own_registry->own_prev_ids);
    }
    if (own_registry->own_is_registered) {
        AR__HEAP__FREE(own_registry->own_is_registered);
    }
    if (own_registry->ref_key_by_id) {
        AR__HEAP__FREE(own_registry->ref_key_by_id);
    }

    AR__HEAP__FREE(own_registry);
}

/* Get the number of registered agents */
int ar_agent_registry__count(const ar_agent_registry_t *ref_registry) {
    if (!ref_registry || !ref_registry->own_registered_ids) {
        return 0;
    }
    return (int)ar_list__count(ref_registry->own_registered_ids);
}

/* Get the first registered agent ID */
int64_t ar_agent_registry__get_first(const ar_agent_registry_t *ref_registry) {
    if (!ref_registry) {
        return 0;
    }
    return ref_registry->first_registered_id;
}

/* Get the next registered agent ID */
int64_t ar_agent_registry__get_next(const ar_agent_registry_t *ref_registry, int64_t current_id) {
    if (!ref_registry || current_id <= 0 || (size_t)current_id >= ref_registry->id_capacity ||
        !ref_registry->own_is_registered || !ref_registry->own_next_ids ||
        !ref_registry->own_is_registered[current_id]) {
        return 0;
    }

    return ref_registry->own_next_ids[current_id];
}

/* Clear all agents from the registry */
void ar_agent_registry__clear(ar_agent_registry_t *mut_registry) {
    if (!mut_registry || !mut_registry->own_registered_ids) {
        return;
    }

    while (ar_list__count(mut_registry->own_registered_ids) > 0) {
        ar_data_t *own_data = ar_list__remove_first(mut_registry->own_registered_ids);
        if (own_data) {
            ar_data__destroy(own_data);
        }
    }

    if (mut_registry->own_agent_map) {
        ar_map__destroy(mut_registry->own_agent_map);
        mut_registry->own_agent_map = ar_map__create();
    }

    _reset_index_state(mut_registry);
    mut_registry->next_agent_id = 1;
}

/* Get the next agent ID */
int64_t ar_agent_registry__get_next_id(const ar_agent_registry_t *ref_registry) {
    if (!ref_registry) {
        return 0;
    }
    return ref_registry->next_agent_id;
}

/* Set the next agent ID */
void ar_agent_registry__set_next_id(ar_agent_registry_t *mut_registry, int64_t id) {
    if (!mut_registry) {
        return;
    }
    mut_registry->next_agent_id = id;
}

/* Allocate a new agent ID */
int64_t ar_agent_registry__allocate_id(ar_agent_registry_t *mut_registry) {
    if (!mut_registry) {
        return 0;
    }
    return mut_registry->next_agent_id++;
}

/* Register an agent ID */
bool ar_agent_registry__register_id(ar_agent_registry_t *mut_registry, int64_t agent_id) {
    char id_str[32];
    ar_data_t *own_id_data;
    const char *ref_stable_key;

    if (!mut_registry || !mut_registry->own_registered_ids || agent_id <= 0) {
        return false;
    }

    if (!_ensure_id_capacity(mut_registry, agent_id)) {
        return false;
    }

    if (mut_registry->own_is_registered[agent_id]) {
        return false;
    }

    snprintf(id_str, sizeof(id_str), "%" PRId64, agent_id);

    own_id_data = ar_data__create_string(id_str);
    if (!own_id_data) {
        return false;
    }

    if (!ar_list__add_last(mut_registry->own_registered_ids, own_id_data)) {
        ar_data__destroy(own_id_data);
        return false;
    }

    ref_stable_key = ar_data__get_string(own_id_data);
    mut_registry->ref_key_by_id[agent_id] = ref_stable_key;
    mut_registry->own_is_registered[agent_id] = true;
    mut_registry->own_next_ids[agent_id] = 0;
    mut_registry->own_prev_ids[agent_id] = mut_registry->last_registered_id;

    if (mut_registry->last_registered_id != 0) {
        mut_registry->own_next_ids[mut_registry->last_registered_id] = agent_id;
    } else {
        mut_registry->first_registered_id = agent_id;
    }

    mut_registry->last_registered_id = agent_id;
    return true;
}

/* Unregister an agent ID */
bool ar_agent_registry__unregister_id(ar_agent_registry_t *mut_registry, int64_t agent_id) {
    void **own_items;
    size_t count;
    ar_data_t *ref_target = NULL;
    int64_t prev_id;
    int64_t next_id;

    if (!mut_registry || !mut_registry->own_registered_ids || agent_id <= 0 ||
        (size_t)agent_id >= mut_registry->id_capacity || !mut_registry->own_is_registered ||
        !mut_registry->own_is_registered[agent_id]) {
        return false;
    }

    ar_agent_registry__untrack_agent(mut_registry, agent_id);

    prev_id = mut_registry->own_prev_ids[agent_id];
    next_id = mut_registry->own_next_ids[agent_id];

    if (prev_id != 0) {
        mut_registry->own_next_ids[prev_id] = next_id;
    } else {
        mut_registry->first_registered_id = next_id;
    }

    if (next_id != 0) {
        mut_registry->own_prev_ids[next_id] = prev_id;
    } else {
        mut_registry->last_registered_id = prev_id;
    }

    own_items = ar_list__items(mut_registry->own_registered_ids);
    if (!own_items) {
        return false;
    }

    count = ar_list__count(mut_registry->own_registered_ids);
    for (size_t i = 0; i < count; i++) {
        ar_data_t *ref_data = (ar_data_t*)own_items[i];
        if (!ref_data) {
            continue;
        }

        if (ar_data__get_string(ref_data) == mut_registry->ref_key_by_id[agent_id]) {
            ref_target = ref_data;
            break;
        }
    }

    AR__HEAP__FREE(own_items);

    if (!ref_target) {
        return false;
    }

    {
        ar_data_t *own_removed = (ar_data_t*)ar_list__remove(mut_registry->own_registered_ids, ref_target);
        if (own_removed) {
            ar_data__destroy(own_removed);
        }
    }

    mut_registry->own_is_registered[agent_id] = false;
    mut_registry->own_next_ids[agent_id] = -1;
    mut_registry->own_prev_ids[agent_id] = -1;
    mut_registry->ref_key_by_id[agent_id] = NULL;
    return true;
}

/* Check if an agent ID is registered */
bool ar_agent_registry__is_registered(const ar_agent_registry_t *ref_registry, int64_t agent_id) {
    if (!ref_registry || agent_id <= 0 || (size_t)agent_id >= ref_registry->id_capacity ||
        !ref_registry->own_is_registered) {
        return false;
    }

    return ref_registry->own_is_registered[agent_id];
}

/* Track an agent object in the registry */
bool ar_agent_registry__track_agent(ar_agent_registry_t *mut_registry, int64_t agent_id, void *mut_agent) {
    const char *ref_key;

    if (!mut_registry || !mut_registry->own_agent_map || !mut_agent || agent_id <= 0 ||
        (size_t)agent_id >= mut_registry->id_capacity || !mut_registry->own_is_registered ||
        !mut_registry->own_is_registered[agent_id] || !mut_registry->ref_key_by_id) {
        return false;
    }

    ref_key = mut_registry->ref_key_by_id[agent_id];
    if (!ref_key) {
        return false;
    }

    return ar_map__set(mut_registry->own_agent_map, ref_key, mut_agent);
}

/* Untrack an agent object from the registry */
void* ar_agent_registry__untrack_agent(ar_agent_registry_t *mut_registry, int64_t agent_id) {
    const char *ref_key;
    void *ref_agent;

    if (!mut_registry || !mut_registry->own_agent_map || agent_id <= 0 ||
        (size_t)agent_id >= mut_registry->id_capacity || !mut_registry->ref_key_by_id) {
        return NULL;
    }

    ref_key = mut_registry->ref_key_by_id[agent_id];
    if (!ref_key) {
        return NULL;
    }

    ref_agent = ar_map__get(mut_registry->own_agent_map, ref_key);
    if (ref_agent) {
        ar_map__set(mut_registry->own_agent_map, ref_key, NULL);
    }

    return ref_agent;
}

/* Find a tracked agent by ID */
void* ar_agent_registry__find_agent(const ar_agent_registry_t *ref_registry, int64_t agent_id) {
    char key[32];

    if (!ref_registry || !ref_registry->own_agent_map || agent_id <= 0) {
        return NULL;
    }

    snprintf(key, sizeof(key), "%" PRId64, agent_id);
    return ar_map__get(ref_registry->own_agent_map, key);
}

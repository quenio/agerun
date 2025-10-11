/**
 * @file ar_proxy_registry.c
 * @brief Implementation of the proxy registry module
 */

#include "ar_proxy_registry.h"
#include "ar_proxy.h"
#include "ar_data.h"
#include "ar_heap.h"
#include "ar_list.h"
#include "ar_map.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

/* Proxy registry structure */
struct ar_proxy_registry_s {
    ar_list_t *own_registered_ids;  // List of registered proxy IDs (negative integers)
    ar_map_t *own_proxy_map;        // Map of proxy_id -> ar_proxy_t pointer
};

/* Create a new proxy registry */
ar_proxy_registry_t* ar_proxy_registry__create(void) {
    ar_proxy_registry_t *own_registry = AR__HEAP__MALLOC(sizeof(ar_proxy_registry_t), "proxy registry");
    if (!own_registry) {
        return NULL;
    }

    own_registry->own_registered_ids = ar_list__create();
    if (!own_registry->own_registered_ids) {
        AR__HEAP__FREE(own_registry);
        return NULL;
    }

    own_registry->own_proxy_map = ar_map__create();
    if (!own_registry->own_proxy_map) {
        ar_list__destroy(own_registry->own_registered_ids);
        AR__HEAP__FREE(own_registry);
        return NULL;
    }

    return own_registry;
}

/* Destroy a proxy registry */
void ar_proxy_registry__destroy(ar_proxy_registry_t *own_registry) {
    if (!own_registry) {
        return;
    }

    // Destroy all registered proxies
    if (own_registry->own_registered_ids && own_registry->own_proxy_map) {
        void **items = ar_list__items(own_registry->own_registered_ids);
        if (items) {
            size_t count = ar_list__count(own_registry->own_registered_ids);
            for (size_t i = 0; i < count; i++) {
                ar_data_t *ref_data = (ar_data_t*)items[i];
                if (ref_data) {
                    const char *id_str = ar_data__get_string(ref_data);
                    if (id_str) {
                        // Get and destroy the proxy
                        ar_proxy_t *own_proxy = (ar_proxy_t*)ar_map__get(own_registry->own_proxy_map, id_str);
                        if (own_proxy) {
                            ar_proxy__destroy(own_proxy);
                        }
                    }
                }
            }
            AR__HEAP__FREE(items);
        }
    }

    // Destroy ID data in the list
    if (own_registry->own_registered_ids) {
        while (ar_list__count(own_registry->own_registered_ids) > 0) {
            ar_data_t *own_data = ar_list__remove_first(own_registry->own_registered_ids);
            if (own_data) {
                ar_data__destroy(own_data);
            }
        }
        ar_list__destroy(own_registry->own_registered_ids);
    }

    if (own_registry->own_proxy_map) {
        ar_map__destroy(own_registry->own_proxy_map);
    }

    AR__HEAP__FREE(own_registry);
}

/* Get the number of registered proxies */
int ar_proxy_registry__count(const ar_proxy_registry_t *ref_registry) {
    if (!ref_registry || !ref_registry->own_registered_ids) {
        return 0;
    }
    return (int)ar_list__count(ref_registry->own_registered_ids);
}

/* Helper function to get string key for proxy_id from the registered list */
static const char* _get_proxy_key_from_list(const ar_proxy_registry_t *ref_registry, int64_t proxy_id) {
    if (!ref_registry || !ref_registry->own_registered_ids) {
        return NULL;
    }

    void **items = ar_list__items(ref_registry->own_registered_ids);
    if (!items) {
        return NULL;
    }

    size_t count = ar_list__count(ref_registry->own_registered_ids);
    const char *key = NULL;

    for (size_t i = 0; i < count; i++) {
        ar_data_t *ref_data = (ar_data_t*)items[i];
        if (!ref_data) {
            continue;
        }

        const char *id_str = ar_data__get_string(ref_data);
        if (!id_str) {
            continue;
        }

        int64_t id = strtoll(id_str, NULL, 10);
        if (id == proxy_id) {
            key = id_str;
            break;
        }
    }

    AR__HEAP__FREE(items);
    return key;
}

/* Register a proxy */
bool ar_proxy_registry__register(ar_proxy_registry_t *mut_registry, int64_t proxy_id, ar_proxy_t *own_proxy) {
    if (!mut_registry || !mut_registry->own_registered_ids || !mut_registry->own_proxy_map || !own_proxy) {
        return false;
    }

    // Check if already registered
    if (ar_proxy_registry__is_registered(mut_registry, proxy_id)) {
        return false;
    }

    // Create string representation of the ID
    char id_str[32];
    snprintf(id_str, sizeof(id_str), "%" PRId64, proxy_id);

    // Create string data for the ID (ar_data__create_string copies the string)
    ar_data_t *own_id_data = ar_data__create_string(id_str);
    if (!own_id_data) {
        return false;
    }

    // Add to the list
    if (!ar_list__add_last(mut_registry->own_registered_ids, own_id_data)) {
        ar_data__destroy(own_id_data);
        return false;
    }

    // Get the key from the list (which owns the string now)
    const char *ref_key = _get_proxy_key_from_list(mut_registry, proxy_id);
    if (!ref_key) {
        // Should not happen, but handle gracefully
        ar_list__remove(mut_registry->own_registered_ids, own_id_data);
        ar_data__destroy(own_id_data);
        return false;
    }

    // Store the proxy pointer in the map
    if (!ar_map__set(mut_registry->own_proxy_map, ref_key, own_proxy)) {
        ar_list__remove(mut_registry->own_registered_ids, own_id_data);
        ar_data__destroy(own_id_data);
        return false;
    }

    // Registry now owns the proxy
    return true;
}

/* Unregister a proxy */
bool ar_proxy_registry__unregister(ar_proxy_registry_t *mut_registry, int64_t proxy_id) {
    if (!mut_registry || !mut_registry->own_registered_ids || !mut_registry->own_proxy_map) {
        return false;
    }

    // Get the string key from the registered list
    const char *ref_key = _get_proxy_key_from_list(mut_registry, proxy_id);
    if (!ref_key) {
        return false;
    }

    // Get and destroy the proxy
    ar_proxy_t *own_proxy = (ar_proxy_t*)ar_map__get(mut_registry->own_proxy_map, ref_key);
    if (own_proxy) {
        ar_proxy__destroy(own_proxy);
        ar_map__set(mut_registry->own_proxy_map, ref_key, NULL);
    }

    // Find and remove the ID from the list
    void **items = ar_list__items(mut_registry->own_registered_ids);
    if (!items) {
        return false;
    }

    size_t count = ar_list__count(mut_registry->own_registered_ids);
    ar_data_t *target = NULL;

    for (size_t i = 0; i < count; i++) {
        ar_data_t *ref_data = (ar_data_t*)items[i];
        if (!ref_data) {
            continue;
        }

        const char *id_str = ar_data__get_string(ref_data);
        if (!id_str) {
            continue;
        }

        int64_t id = strtoll(id_str, NULL, 10);
        if (id == proxy_id) {
            target = ref_data;
            break;
        }
    }

    AR__HEAP__FREE(items);

    if (target) {
        ar_data_t *own_removed = (ar_data_t*)ar_list__remove(mut_registry->own_registered_ids, target);
        if (own_removed) {
            ar_data__destroy(own_removed);
        }
        return true;
    }

    return false;
}

/* Find a proxy */
ar_proxy_t* ar_proxy_registry__find(const ar_proxy_registry_t *ref_registry, int64_t proxy_id) {
    if (!ref_registry || !ref_registry->own_proxy_map) {
        return NULL;
    }

    // Get the string key from the registered list
    const char *ref_key = _get_proxy_key_from_list(ref_registry, proxy_id);
    if (!ref_key) {
        return NULL;
    }

    return (ar_proxy_t*)ar_map__get(ref_registry->own_proxy_map, ref_key);
}

/* Check if registered */
bool ar_proxy_registry__is_registered(const ar_proxy_registry_t *ref_registry, int64_t proxy_id) {
    if (!ref_registry || !ref_registry->own_registered_ids) {
        return false;
    }

    void **items = ar_list__items(ref_registry->own_registered_ids);
    if (!items) {
        return false;
    }

    size_t count = ar_list__count(ref_registry->own_registered_ids);
    bool found = false;

    for (size_t i = 0; i < count; i++) {
        ar_data_t *ref_data = (ar_data_t*)items[i];
        if (!ref_data) {
            continue;
        }

        const char *id_str = ar_data__get_string(ref_data);
        if (!id_str) {
            continue;
        }

        int64_t id = strtoll(id_str, NULL, 10);
        if (id == proxy_id) {
            found = true;
            break;
        }
    }

    AR__HEAP__FREE(items);
    return found;
}

/* Clear all proxies */
void ar_proxy_registry__clear(ar_proxy_registry_t *mut_registry) {
    if (!mut_registry || !mut_registry->own_registered_ids || !mut_registry->own_proxy_map) {
        return;
    }

    // Destroy all registered proxies first
    void **items = ar_list__items(mut_registry->own_registered_ids);
    if (items) {
        size_t count = ar_list__count(mut_registry->own_registered_ids);
        for (size_t i = 0; i < count; i++) {
            ar_data_t *ref_data = (ar_data_t*)items[i];
            if (ref_data) {
                const char *id_str = ar_data__get_string(ref_data);
                if (id_str) {
                    // Get and destroy the proxy
                    ar_proxy_t *own_proxy = (ar_proxy_t*)ar_map__get(mut_registry->own_proxy_map, id_str);
                    if (own_proxy) {
                        ar_proxy__destroy(own_proxy);
                    }
                }
            }
        }
        AR__HEAP__FREE(items);
    }

    // Destroy all ID data in the list
    while (ar_list__count(mut_registry->own_registered_ids) > 0) {
        ar_data_t *own_data = ar_list__remove_first(mut_registry->own_registered_ids);
        if (own_data) {
            ar_data__destroy(own_data);
        }
    }

    // Clear the proxy map by destroying and recreating it
    if (mut_registry->own_proxy_map) {
        ar_map__destroy(mut_registry->own_proxy_map);
        mut_registry->own_proxy_map = ar_map__create();
    }
}

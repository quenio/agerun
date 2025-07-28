#include "ar_method_resolver.h"
#include "ar_method_registry.h"
#include "ar_method.h"
#include "ar_list.h"
#include "ar_semver.h"
#include "ar_heap.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Method resolver structure */
struct ar_method_resolver_s {
    ar_method_registry_t *ref_registry;  /* Borrowed reference to registry */
};

ar_method_resolver_t* ar_method_resolver__create(ar_method_registry_t *ref_registry) {
    if (!ref_registry) {
        return NULL;
    }
    
    ar_method_resolver_t *own_resolver = AR__HEAP__MALLOC(sizeof(ar_method_resolver_t), "ar_method_resolver_t");
    if (!own_resolver) {
        return NULL;
    }
    
    own_resolver->ref_registry = ref_registry;
    
    return own_resolver;
}

void ar_method_resolver__destroy(ar_method_resolver_t *own_resolver) {
    if (own_resolver) {
        AR__HEAP__FREE(own_resolver);
    }
}

/**
 * Check if a version string is a full semver (has 2 dots)
 */
static bool _is_full_version(const char *ref_version) {
    if (!ref_version) {
        return false;
    }
    
    int dot_count = 0;
    for (const char *p = ref_version; *p; p++) {
        if (*p == '.') {
            dot_count++;
        }
    }
    
    return dot_count == 2;
}

/**
 * Resolve partial version pattern by finding the latest matching version
 */
static ar_method_t* _resolve_partial_version(ar_method_resolver_t *ref_resolver,
                                             const char *ref_name,
                                             const char *ref_pattern) {
    // Get all methods from registry
    ar_list_t *own_all_methods = ar_method_registry__get_all_methods(ref_resolver->ref_registry);
    if (!own_all_methods) {
        return NULL;
    }
    
    // Filter by method name and collect versions
    void **items = ar_list__items(own_all_methods);
    size_t count = ar_list__count(own_all_methods);
    
    // Temporary arrays to hold matching methods and their versions
    ar_method_t **matching_methods = AR__HEAP__MALLOC(sizeof(ar_method_t*) * count, "matching methods");
    const char **matching_versions = AR__HEAP__MALLOC(sizeof(const char*) * count, "matching versions");
    
    if (!matching_methods || !matching_versions) {
        AR__HEAP__FREE(items);
        ar_list__destroy(own_all_methods);
        AR__HEAP__FREE(matching_methods);
        AR__HEAP__FREE(matching_versions);
        return NULL;
    }
    
    int matching_count = 0;
    for (size_t i = 0; i < count; i++) {
        ar_method_t *method = (ar_method_t*)items[i];
        if (strcmp(ar_method__get_name(method), ref_name) == 0) {
            matching_methods[matching_count] = method;
            matching_versions[matching_count] = ar_method__get_version(method);
            matching_count++;
        }
    }
    
    ar_method_t *result = NULL;
    
    // Use semver to find the latest matching version
    if (matching_count > 0) {
        int latest_idx = ar_semver__find_latest_matching(matching_versions, matching_count, ref_pattern);
        if (latest_idx >= 0) {
            result = matching_methods[latest_idx];
        }
    }
    
    // Clean up
    AR__HEAP__FREE(items);
    ar_list__destroy(own_all_methods);
    AR__HEAP__FREE(matching_methods);
    AR__HEAP__FREE(matching_versions);
    
    return result;
}

ar_method_t* ar_method_resolver__resolve_method(ar_method_resolver_t *ref_resolver,
                                                const char *ref_name,
                                                const char *ref_version) {
    if (!ref_resolver || !ref_name) {
        return NULL;
    }
    
    // If version is NULL or empty, return the latest version
    if (!ref_version || ref_version[0] == '\0') {
        return ar_method_registry__get_latest_version(ref_resolver->ref_registry, ref_name);
    }
    
    // If version is a full semver (e.g., "1.2.3"), return exact match
    if (_is_full_version(ref_version)) {
        return ar_method_registry__get_method_by_exact_match(ref_resolver->ref_registry, ref_name, ref_version);
    }
    
    // Otherwise, it's a partial version pattern
    return _resolve_partial_version(ref_resolver, ref_name, ref_version);
}
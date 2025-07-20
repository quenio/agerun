/**
 * @file ar_path.c
 * @brief Implementation of the path manipulation module
 */

#include "ar_path.h"
#include "ar_heap.h"
#include <string.h>
#include <stdbool.h>

/* Internal structure for path objects */
struct ar_path_s {
    char *own_path_string;  /* Owned copy of the path string */
    char separator;         /* Separator character */
    char **own_segments;    /* Owned array of segment strings */
    size_t segment_count;   /* Number of segments */
};

/* Helper function to allocate and copy a segment */
static char* _copy_segment(const char *start, size_t len) {
    char *segment = AR__HEAP__MALLOC(len + 1, "segment");
    if (segment) {
        memcpy(segment, start, len);
        segment[len] = '\0';
    }
    return segment;
}

/* Helper function to build a path string from segments */
static char* _build_path_string(char **segments, size_t count, char separator) {
    if (!segments || count == 0) {
        return AR__HEAP__STRDUP("", "empty path");
    }
    
    // Calculate total length
    size_t total_len = 0;
    for (size_t i = 0; i < count; i++) {
        if (i > 0) {
            total_len++;  // For separator
        }
        total_len += strlen(segments[i]);
    }
    
    // Build path string
    char *own_path = AR__HEAP__MALLOC(total_len + 1, "path string");
    if (!own_path) {
        return NULL;
    }
    
    char *p = own_path;
    for (size_t i = 0; i < count; i++) {
        if (i > 0) {
            *p++ = separator;
        }
        size_t len = strlen(segments[i]);
        memcpy(p, segments[i], len);
        p += len;
    }
    *p = '\0';
    
    return own_path;
}

/* Helper function to parse path into segments */
static void _parse_segments(ar_path_t *mut_path) {
    if (!mut_path || !mut_path->own_path_string || *mut_path->own_path_string == '\0') {
        mut_path->segment_count = 0;
        mut_path->own_segments = NULL;
        return;
    }
    
    // Count segments
    size_t count = 1;
    const char *p = mut_path->own_path_string;
    while (*p) {
        if (*p == mut_path->separator) {
            count++;
        }
        p++;
    }
    
    // Allocate segment array
    mut_path->own_segments = AR__HEAP__MALLOC(count * sizeof(char*), "segment array");
    if (!mut_path->own_segments) {
        mut_path->segment_count = 0;
        return;
    }
    
    // Parse segments
    size_t index = 0;
    const char *start = mut_path->own_path_string;
    p = start;
    
    while (*p) {
        if (*p == mut_path->separator) {
            size_t len = (size_t)(p - start);
            mut_path->own_segments[index] = _copy_segment(start, len);
            index++;
            start = p + 1;
        }
        p++;
    }
    
    // Last segment
    size_t len = (size_t)(p - start);
    mut_path->own_segments[index] = _copy_segment(start, len);
    
    mut_path->segment_count = count;
}

ar_path_t* ar_path__create(const char *ref_path, char separator) {
    if (!ref_path) {
        return NULL;
    }
    
    ar_path_t *own_path = AR__HEAP__MALLOC(sizeof(ar_path_t), "ar_path_t");
    if (!own_path) {
        return NULL;
    }
    
    own_path->own_path_string = AR__HEAP__STRDUP(ref_path, "path string");
    if (!own_path->own_path_string) {
        AR__HEAP__FREE(own_path);
        return NULL;
    }
    
    own_path->separator = separator;
    
    // Parse segments
    _parse_segments(own_path);
    
    // Ownership transferred to caller
    return own_path;
}

ar_path_t* ar_path__create_variable(const char *ref_path) {
    return ar_path__create(ref_path, '.');
}

ar_path_t* ar_path__create_file(const char *ref_path) {
    return ar_path__create(ref_path, '/');
}

void ar_path__destroy(ar_path_t *own_path) {
    if (!own_path) {
        return;
    }
    
    // Free segments
    if (own_path->own_segments) {
        for (size_t i = 0; i < own_path->segment_count; i++) {
            AR__HEAP__FREE(own_path->own_segments[i]);
        }
        AR__HEAP__FREE(own_path->own_segments);
    }
    
    AR__HEAP__FREE(own_path->own_path_string);
    AR__HEAP__FREE(own_path);
}

const char* ar_path__get_string(const ar_path_t *ref_path) {
    if (!ref_path) {
        return NULL;
    }
    
    return ref_path->own_path_string;
}

char ar_path__get_separator(const ar_path_t *ref_path) {
    if (!ref_path) {
        return '\0';
    }
    
    return ref_path->separator;
}

size_t ar_path__get_segment_count(const ar_path_t *ref_path) {
    if (!ref_path) {
        return 0;
    }
    
    return ref_path->segment_count;
}

const char* ar_path__get_segment(const ar_path_t *ref_path, size_t index) {
    if (!ref_path || index >= ref_path->segment_count) {
        return NULL;
    }
    
    return ref_path->own_segments[index];
}


ar_path_t* ar_path__get_parent(const ar_path_t *ref_path) {
    if (!ref_path || ref_path->segment_count <= 1) {
        return NULL;
    }
    
    // Build parent path string using all segments except the last
    char *own_parent_string = _build_path_string(
        ref_path->own_segments, 
        ref_path->segment_count - 1, 
        ref_path->separator
    );
    if (!own_parent_string) {
        return NULL;
    }
    
    // Create parent path
    ar_path_t *own_parent = ar_path__create(own_parent_string, ref_path->separator);
    AR__HEAP__FREE(own_parent_string);
    
    // Ownership transferred to caller
    return own_parent;
}

bool ar_path__starts_with(const ar_path_t *ref_path, const char *ref_prefix) {
    if (!ref_path || !ref_prefix) {
        return false;
    }
    
    // Create a temporary path with the prefix to compare segments
    ar_path_t *own_prefix_path = ar_path__create(ref_prefix, ref_path->separator);
    if (!own_prefix_path) {
        return false;
    }
    
    // Check if we have enough segments
    if (own_prefix_path->segment_count > ref_path->segment_count) {
        ar_path__destroy(own_prefix_path);
        return false;
    }
    
    // Compare each segment
    bool matches = true;
    for (size_t i = 0; i < own_prefix_path->segment_count; i++) {
        if (strcmp(ref_path->own_segments[i], own_prefix_path->own_segments[i]) != 0) {
            matches = false;
            break;
        }
    }
    
    ar_path__destroy(own_prefix_path);
    return matches;
}

const char* ar_path__get_variable_root(const ar_path_t *ref_path) {
    if (!ref_path || ref_path->segment_count == 0) {
        return NULL;
    }
    
    return ref_path->own_segments[0];
}

bool ar_path__is_memory_path(const ar_path_t *ref_path) {
    return ar_path__starts_with(ref_path, "memory");
}

bool ar_path__is_context_path(const ar_path_t *ref_path) {
    return ar_path__starts_with(ref_path, "context");
}

bool ar_path__is_message_path(const ar_path_t *ref_path) {
    return ar_path__starts_with(ref_path, "message");
}

ar_path_t* ar_path__join(const ar_path_t *ref_base, const char *ref_suffix) {
    if (!ref_base || !ref_suffix) {
        return NULL;
    }
    
    // If suffix is empty, return a copy of the base
    if (*ref_suffix == '\0') {
        return ar_path__create(ref_base->own_path_string, ref_base->separator);
    }
    
    // If base is empty, return path with just the suffix
    if (*ref_base->own_path_string == '\0') {
        return ar_path__create(ref_suffix, ref_base->separator);
    }
    
    // Calculate joined path length
    size_t base_len = strlen(ref_base->own_path_string);
    size_t suffix_len = strlen(ref_suffix);
    size_t total_len = base_len + 1 + suffix_len;  // +1 for separator
    
    // Build joined path string
    char *own_joined = AR__HEAP__MALLOC(total_len + 1, "joined path");
    if (!own_joined) {
        return NULL;
    }
    
    memcpy(own_joined, ref_base->own_path_string, base_len);
    own_joined[base_len] = ref_base->separator;
    memcpy(own_joined + base_len + 1, ref_suffix, suffix_len);
    own_joined[total_len] = '\0';
    
    // Create new path
    ar_path_t *own_path = ar_path__create(own_joined, ref_base->separator);
    AR__HEAP__FREE(own_joined);
    
    // Ownership transferred to caller
    return own_path;
}

ar_path_t* ar_path__normalize(const ar_path_t *ref_path) {
    if (!ref_path) {
        return NULL;
    }
    
    // Count non-empty segments
    size_t non_empty_count = 0;
    bool starts_with_separator = false;
    
    for (size_t i = 0; i < ref_path->segment_count; i++) {
        if (i == 0 && *ref_path->own_segments[i] == '\0') {
            // First segment is empty (path starts with separator)
            starts_with_separator = true;
            non_empty_count++;  // Keep the first empty segment for paths like "/usr/local"
        } else if (*ref_path->own_segments[i] != '\0') {
            non_empty_count++;
        }
    }
    
    // If no non-empty segments, return empty path
    if (non_empty_count == 0) {
        return ar_path__create("", ref_path->separator);
    }
    
    // Build array of non-empty segments
    char **segments = AR__HEAP__MALLOC(non_empty_count * sizeof(char*), "segment array");
    if (!segments) {
        return NULL;
    }
    
    size_t j = 0;
    for (size_t i = 0; i < ref_path->segment_count; i++) {
        if ((i == 0 && starts_with_separator) || *ref_path->own_segments[i] != '\0') {
            segments[j++] = ref_path->own_segments[i];
        }
    }
    
    // Build normalized path string
    char *own_normalized = _build_path_string(segments, non_empty_count, ref_path->separator);
    AR__HEAP__FREE(segments);
    
    if (!own_normalized) {
        return NULL;
    }
    
    // Create normalized path
    ar_path_t *own_path = ar_path__create(own_normalized, ref_path->separator);
    AR__HEAP__FREE(own_normalized);
    
    // Ownership transferred to caller
    return own_path;
}

const char* ar_path__get_suffix_after_root(const ar_path_t *ref_path) {
    if (!ref_path) {
        return NULL;
    }
    
    // Need at least 2 segments (root.suffix)
    if (ref_path->segment_count < 2) {
        return NULL;
    }
    
    // Find the position after the first separator
    const char *first_separator = strchr(ref_path->own_path_string, ref_path->separator);
    if (!first_separator) {
        return NULL;
    }
    
    // Get the suffix after separator
    const char *suffix = first_separator + 1;
    
    // Return NULL if suffix is empty (e.g., "memory." has no valid key)
    if (*suffix == '\0') {
        return NULL;
    }
    
    return suffix;
}
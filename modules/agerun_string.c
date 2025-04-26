#include "agerun_string.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

/**
 * Helper function to trim whitespace from a string
 */
char* ar_string_trim(char *mut_str) {
    if (!mut_str) return NULL;
    
    // Trim leading space
    char *start = mut_str;
    while (ar_string_isspace(*start)) start++;
    
    if(*start == 0) // All spaces
        return start;
    
    // Trim trailing space
    char *end = start + strlen(start) - 1;
    while (end > start && ar_string_isspace(*end)) end--;
    
    // Write new null terminator
    *(end + 1) = 0;
    
    return start;
}

/**
 * Counts the number of segments in a path separated by the given separator.
 *
 * @param ref_str The string to analyze (e.g., "key.sub_key.sub_sub_key")
 * @param separator The character used as separator (e.g., '.')
 * @return Number of segments in the string (0 if ref_str is NULL)
 */
size_t ar_string_path_count(const char *ref_str, char separator) {
    if (!ref_str || !*ref_str) {
        return 0;
    }
    
    size_t count = 1;  // Start with 1 for the first segment
    const char *ptr = ref_str;
    
    while ((ptr = strchr(ptr, separator)) != NULL) {
        count++;
        ptr++;  // Move past the separator
    }
    
    return count;
}

/**
 * Extracts a segment from a separated string.
 *
 * @param ref_str The string to extract from (e.g., "key.sub_key.sub_sub_key")
 * @param separator The character used as separator (e.g., '.')
 * @param index The zero-based index of the segment to extract
 * @return OWNER: Heap-allocated string containing the extracted segment, or NULL on error
 *         Caller is responsible for freeing the returned string.
 */
char* ar_string_path_segment(const char *ref_str, char separator, size_t index) {
    if (!ref_str || !*ref_str) {
        return NULL;
    }
    
    const char *start = ref_str;
    size_t current_index = 0;
    
    // Find the start of the segment at the specified index
    while (current_index < index) {
        start = strchr(start, separator);
        if (!start) {
            // Index is out of bounds
            return NULL;
        }
        start++;  // Move past the separator
        current_index++;
    }
    
    // Find the end of the segment (either next separator or end of string)
    const char *end = strchr(start, separator);
    if (!end) {
        // This is the last segment
        end = start + strlen(start);
    }
    
    // Calculate segment length
    size_t length = (size_t)(end - start);
    if (length == 0) {
        // Empty segment
        return strdup(""); // Ownership transferred to caller
    }
    
    // Allocate and copy the segment
    char *own_segment = (char *)malloc(length + 1);
    if (!own_segment) {
        return NULL;
    }
    
    memcpy(own_segment, start, length);
    own_segment[length] = '\0';
    
    return own_segment; // Ownership transferred to caller
}

/**
 * Extracts the parent path from a path string.
 *
 * @param ref_str The path string to extract from (e.g., "key.sub_key.sub_sub_key")
 * @param separator The character used as separator (e.g., '.')
 * @return OWNER: Heap-allocated string containing the parent path, or NULL if no parent exists
 *         (i.e., for root paths or errors). Caller is responsible for freeing the returned string.
 */
char* ar_string_path_parent(const char *ref_str, char separator) {
    if (!ref_str || !*ref_str) {
        return NULL;
    }
    
    // Count segments to determine if parent exists
    size_t segments = ar_string_path_count(ref_str, separator);
    if (segments <= 1) {
        // No parent for root paths or single segments
        return NULL;
    }
    
    // Find the last separator
    const char *last_sep = strrchr(ref_str, separator);
    if (!last_sep) {
        // This should not happen if segments > 1, but handle it anyway
        return NULL;
    }
    
    // Calculate the length of the parent path (excluding the last separator)
    size_t parent_len = (size_t)(last_sep - ref_str);
    if (parent_len == 0) {
        // Edge case: path starts with a separator (e.g., ".key")
        return strdup(""); // Ownership transferred to caller
    }
    
    // Allocate and copy the parent path
    char *own_parent = (char *)malloc(parent_len + 1);
    if (!own_parent) {
        return NULL;
    }
    
    memcpy(own_parent, ref_str, parent_len);
    own_parent[parent_len] = '\0';
    
    return own_parent; // Ownership transferred to caller
}

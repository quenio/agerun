#include "agerun_string.h"
#include "agerun_heap.h"
#include "agerun_assert.h" /* Include the assertion utilities */

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

/**
 * Helper function to trim whitespace from a string
 */
char* ar_string_trim(char *mut_str) {
    if (!mut_str) return NULL;
    
    // Trim leading space
    char *ref_start = mut_str;
    while (ar_string_isspace(*ref_start)) ref_start++;
    
    if(*ref_start == 0) // All spaces
        return ref_start; // Borrowed reference, not owned by caller
    
    // Trim trailing space
    char *mut_end = ref_start + strlen(ref_start) - 1;
    while (mut_end > ref_start && ar_string_isspace(*mut_end)) mut_end--;
    
    // Write new null terminator
    *(mut_end + 1) = 0;
    
    return ref_start; // Borrowed reference, not owned by caller
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
    const char *ref_ptr = ref_str;
    
    while ((ref_ptr = strchr(ref_ptr, separator)) != NULL) {
        count++;
        ref_ptr++;  // Move past the separator
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
 *         Caller is responsible for freeing the returned string using AR_HEAP_FREE().
 */
char* ar_string_path_segment(const char *ref_str, char separator, size_t index) {
    if (!ref_str || !*ref_str) {
        return NULL;
    }
    
    const char *ref_start = ref_str;
    size_t current_index = 0;
    
    // Find the start of the segment at the specified index
    while (current_index < index) {
        ref_start = strchr(ref_start, separator);
        if (!ref_start) {
            // Index is out of bounds
            return NULL;
        }
        ref_start++;  // Move past the separator
        current_index++;
    }
    
    // Find the end of the segment (either next separator or end of string)
    const char *ref_end = strchr(ref_start, separator);
    if (!ref_end) {
        // This is the last segment
        ref_end = ref_start + strlen(ref_start);
    }
    
    // Calculate segment length
    size_t length = (size_t)(ref_end - ref_start);
    if (length == 0) {
        // Empty segment
        char *own_result = AR_HEAP_STRDUP("", "Empty string path segment"); 
        AR_ASSERT_OWNERSHIP(own_result);
        return own_result; // Ownership transferred to caller
    }
    
    // Allocate and copy the segment
    char *own_segment = (char *)AR_HEAP_MALLOC(length + 1, "String path segment");
    if (!own_segment) {
        return NULL;
    }
    
    memcpy(own_segment, ref_start, length);
    own_segment[length] = '\0';
    
    return own_segment; // Ownership transferred to caller
}

/**
 * Extracts the parent path from a path string.
 *
 * @param ref_str The path string to extract from (e.g., "key.sub_key.sub_sub_key")
 * @param separator The character used as separator (e.g., '.')
 * @return OWNER: Heap-allocated string containing the parent path, or NULL if no parent exists
 *         (i.e., for root paths or errors). Caller is responsible for freeing the returned string using AR_HEAP_FREE().
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
    const char *ref_last_sep = strrchr(ref_str, separator);
    if (!ref_last_sep) {
        // This should not happen if segments > 1, but handle it anyway
        return NULL;
    }
    
    // Calculate the length of the parent path (excluding the last separator)
    size_t parent_len = (size_t)(ref_last_sep - ref_str);
    if (parent_len == 0) {
        // Edge case: path starts with a separator (e.g., ".key")
        char *own_result = AR_HEAP_STRDUP("", "Empty string path segment");
        AR_ASSERT_OWNERSHIP(own_result);
        return own_result; // Ownership transferred to caller
    }
    
    // Allocate and copy the parent path
    char *own_parent = (char *)AR_HEAP_MALLOC(parent_len + 1, "String path parent");
    if (!own_parent) {
        return NULL;
    }
    
    memcpy(own_parent, ref_str, parent_len);
    own_parent[parent_len] = '\0';
    
    return own_parent; // Ownership transferred to caller
}

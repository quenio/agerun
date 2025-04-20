#include "agerun_string.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

/**
 * Helper function to trim whitespace from a string
 */
char* ar_string_trim(char *str) {
    if (!str) return NULL;
    
    // Trim leading space
    char *start = str;
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
 * @param str The string to analyze (e.g., "key.sub_key.sub_sub_key")
 * @param separator The character used as separator (e.g., '.')
 * @return Number of segments in the string (0 if str is NULL)
 */
size_t ar_string_path_count(const char *str, char separator) {
    if (!str || !*str) {
        return 0;
    }
    
    size_t count = 1;  // Start with 1 for the first segment
    const char *ptr = str;
    
    while ((ptr = strchr(ptr, separator)) != NULL) {
        count++;
        ptr++;  // Move past the separator
    }
    
    return count;
}

/**
 * Extracts a segment from a separated string.
 *
 * @param str The string to extract from (e.g., "key.sub_key.sub_sub_key")
 * @param separator The character used as separator (e.g., '.')
 * @param index The zero-based index of the segment to extract
 * @return Heap-allocated string containing the extracted segment, or NULL on error
 *         Caller is responsible for freeing the returned string.
 */
char* ar_string_path_segment(const char *str, char separator, size_t index) {
    if (!str || !*str) {
        return NULL;
    }
    
    const char *start = str;
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
        return strdup("");
    }
    
    // Allocate and copy the segment
    char *segment = (char *)malloc(length + 1);
    if (!segment) {
        return NULL;
    }
    
    memcpy(segment, start, length);
    segment[length] = '\0';
    
    return segment;
}

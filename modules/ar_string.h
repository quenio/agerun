#ifndef AGERUN_STRING_H
#define AGERUN_STRING_H
#include <stddef.h>

/**
 * Returns non-zero if c is a whitespace character.
 * This wrapper safely handles signed char values by casting to unsigned char.
 */
int ar_string__isspace(int c);

/**
 * Trims leading and trailing whitespace from a string.
 *
 * @param mut_str The string to trim (modified in-place)
 * @return BORROW: Pointer to the trimmed string within original buffer
 */
char* ar_string__trim(char *mut_str);

/**
 * Counts the number of segments in a path separated by the given separator.
 *
 * @param ref_str The string to analyze (e.g., "key.sub_key.sub_sub_key")
 * @param separator The character used as separator (e.g., '.')
 * @return Number of segments in the string (0 if ref_str is NULL)
 */
size_t ar_string__path_count(const char *ref_str, char separator);

/**
 * Extracts a segment from a separated string.
 *
 * @param ref_str The string to extract from (e.g., "key.sub_key.sub_sub_key")
 * @param separator The character used as separator (e.g., '.')
 * @param index The zero-based index of the segment to extract
 * @return OWNER: Heap-allocated string containing the extracted segment, or NULL on error
 *         Caller is responsible for freeing the returned string.
 */
char* ar_string__path_segment(const char *ref_str, char separator, size_t index);

/**
 * Extracts the parent path from a path string.
 *
 * @param ref_str The path string to extract from (e.g., "key.sub_key.sub_sub_key")
 * @param separator The character used as separator (e.g., '.')
 * @return OWNER: Heap-allocated string containing the parent path, or NULL if no parent exists
 *         (i.e., for root paths or errors). Caller is responsible for freeing the returned string.
 */
char* ar_string__path_parent(const char *ref_str, char separator);

#endif /* AGERUN_STRING_H */

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

#endif /* AGERUN_STRING_H */

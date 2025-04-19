#ifndef AGERUN_STRING_H
#define AGERUN_STRING_H
#include <ctype.h>
/**
 * Returns non-zero if c is a whitespace character.
 * This wrapper safely handles signed char values by casting to unsigned char.
 */
static inline int ar_string_isspace(int c) {
    return isspace((unsigned char)c);
}

/**
 * Trims leading and trailing whitespace from a string.
 *
 * @param str The string to trim
 * @return Pointer to the trimmed string
 */
char* ar_string_trim(char *str);

#endif /* AGERUN_STRING_H */

#include "../include/agerun_string.h"

#include <ctype.h>
#include <string.h>

/**
 * Helper function to trim whitespace from a string
 */
char* ar_trim(char *str) {
    if (!str) return NULL;
    
    // Trim leading space
    char *start = str;
    while(isspace((unsigned char)*start)) start++;
    
    if(*start == 0) // All spaces
        return start;
    
    // Trim trailing space
    char *end = start + strlen(start) - 1;
    while(end > start && isspace((unsigned char)*end)) end--;
    
    // Write new null terminator
    *(end + 1) = 0;
    
    return start;
}
